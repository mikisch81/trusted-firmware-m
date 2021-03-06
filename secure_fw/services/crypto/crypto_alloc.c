/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <limits.h>

#include "tfm_crypto_defs.h"

/* Pre include Mbed TLS headers */
#define LIB_PREFIX_NAME __tfm_crypto__
#include "mbedtls_global_symbols.h"

/* Include the Mbed TLS configuration file, the way Mbed TLS does it
 * in each of its header files.
 */
#if !defined(MBEDTLS_CONFIG_FILE)
#include "platform/ext/common/tfm_mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "psa_crypto.h"
#include "tfm_crypto_api.h"

/* The file "psa_crypto_struct.h" contains definitions for
 * implementation-specific structs that are declared in "psa_crypto.h".
 */
#include "psa_crypto_struct.h"

/**
 * \brief This value defines the maximum number of simultaneous operations
 *        supported by this implementation.
 */
#define TFM_CRYPTO_CONC_OPER_NUM (8)

/**
 * \brief This value is used to mark an handle as invalid.
 *
 */
#define INVALID_HANDLE (0xFFFFFFFF)

struct tfm_crypto_operation_s {
    uint32_t in_use;                /*!< Indicates if the operation is in use */
    enum tfm_crypto_operation_type type; /*!< Type of the operation */
    union {
        struct psa_cipher_operation_s cipher;   /*!< Cipher operation context */
        struct psa_mac_operation_s mac;         /*!< MAC operation context */
        struct psa_hash_operation_s hash;       /*!< Hash operation context */
    } operation;
};

static struct tfm_crypto_operation_s operation[TFM_CRYPTO_CONC_OPER_NUM] ={{0}};

static void memset_operation_context(uint32_t index)
{
    uint32_t i, mem_size;

    volatile uint8_t *mem_ptr = (uint8_t *) &(operation[index].operation);

    switch(operation[index].type) {
    case TFM_CRYPTO_CIPHER_OPERATION:
        mem_size = sizeof(struct psa_cipher_operation_s);
        break;
    case TFM_CRYPTO_MAC_OPERATION:
        mem_size = sizeof(struct psa_mac_operation_s);
        break;
    case TFM_CRYPTO_HASH_OPERATION:
        mem_size = sizeof(struct psa_hash_operation_s);
        break;
    case TFM_CRYPTO_OPERATION_NONE:
    default:
        mem_size = 0;
        break;
    }

    for (i=0; i<mem_size; i++) {
        mem_ptr[i] = 0;
    }
}

/*!
 * \defgroup public Public functions
 *
 */

/*!@{*/
enum tfm_crypto_err_t tfm_crypto_operation_alloc(
                                        enum tfm_crypto_operation_type type,
                                        uint32_t *handle)
{
    uint32_t i = 0;

    for (i=0; i<TFM_CRYPTO_CONC_OPER_NUM; i++) {
        if (operation[i].in_use == TFM_CRYPTO_NOT_IN_USE) {
            operation[i].in_use = TFM_CRYPTO_IN_USE;
            operation[i].type = type;
            *handle = i;
            return TFM_CRYPTO_ERR_PSA_SUCCESS;
        }
    }


    return TFM_CRYPTO_ERR_PSA_ERROR_NOT_PERMITTED;
}

enum tfm_crypto_err_t tfm_crypto_operation_release(uint32_t *handle)
{
    uint32_t i = *handle;

    if ( (i<TFM_CRYPTO_CONC_OPER_NUM) &&
         (operation[i].in_use == TFM_CRYPTO_IN_USE) ) {
        memset_operation_context(i);
        operation[i].in_use = TFM_CRYPTO_NOT_IN_USE;
        operation[i].type = TFM_CRYPTO_OPERATION_NONE;
        *handle = INVALID_HANDLE;
        return TFM_CRYPTO_ERR_PSA_SUCCESS;
    }

    return TFM_CRYPTO_ERR_PSA_ERROR_INVALID_ARGUMENT;
}

enum tfm_crypto_err_t tfm_crypto_operation_lookup(
                                        enum tfm_crypto_operation_type type,
                                        uint32_t *handle,
                                        void **oper)
{
    uint32_t i = *handle;

    if ( (i<TFM_CRYPTO_CONC_OPER_NUM) &&
         (operation[i].in_use == TFM_CRYPTO_IN_USE) &&
         (operation[i].type == type) ) {
        *oper = (void *) &(operation[i].operation);
        return TFM_CRYPTO_ERR_PSA_SUCCESS;
    }

    return TFM_CRYPTO_ERR_PSA_ERROR_INVALID_ARGUMENT;
}
/*!@}*/
