/*
 * Copyright (c) 2017-2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include "tfm_ss_core_test_2.h"
#include "tfm_api.h"
#include "tfm_secure_api.h"
#include "spm_partition_defs.h"
#include "test/test_services/tfm_core_test/core_test_defs.h"

#define INVALID_NS_CLIENT_ID  0x49abcdef

/* Don't initialise caller_partition_id_zi and expect it to be linked in the
 * zero-initialised data area
 */
static int32_t caller_client_id_zi;

/* Initialise caller_partition_id_rw and expect it to be linked in the
 * read-write data area
 */
static int32_t caller_client_id_rw = INVALID_NS_CLIENT_ID;

static int32_t* invalid_addresses [] = {(int32_t*)0x0, (int32_t*)0xFFF12000};

/* FIXME: Add a testcase to test that a failed init makes the secure partition
 * closed, and none of its functions can be called.
 * A new test service for this purpose is to be added.
 */
int32_t core_test_2_init(void)
{
    return TFM_SUCCESS;
}

int32_t spm_core_test_2_slave_service(void)
{
    return TFM_SUCCESS;
}

int32_t spm_core_test_2_check_caller_client_id(void)
{
    size_t i;
    int32_t caller_client_id_stack = INVALID_NS_CLIENT_ID;
    int32_t ret;

    caller_client_id_zi = INVALID_NS_CLIENT_ID;

    /* test with invalid output pointers */
    for (i = 0; i < sizeof(invalid_addresses)/sizeof(invalid_addresses[0]); ++i)
    {
        ret = tfm_core_get_caller_client_id(invalid_addresses[i]);
        if (ret != TFM_ERROR_INVALID_PARAMETER) {
            return CORE_TEST_ERRNO_TEST_FAULT;
        }
    }

    /* test with valid output pointers */
    ret = tfm_core_get_caller_client_id(&caller_client_id_zi);
    if (ret != TFM_SUCCESS || caller_client_id_zi != TFM_SP_CORE_TEST_ID) {
        return CORE_TEST_ERRNO_TEST_FAULT;
    }

    ret = tfm_core_get_caller_client_id(&caller_client_id_rw);
    if (ret != TFM_SUCCESS || caller_client_id_rw != TFM_SP_CORE_TEST_ID) {
        return CORE_TEST_ERRNO_TEST_FAULT;
    }

    ret = tfm_core_get_caller_client_id(&caller_client_id_stack);
    if (ret != TFM_SUCCESS ||
            caller_client_id_stack != TFM_SP_CORE_TEST_ID) {
        return CORE_TEST_ERRNO_TEST_FAULT;
    }

    return TFM_SUCCESS;
}

/* Invert function */
#define SFN_INVERT_MAX_LEN 128

int32_t spm_core_test_2_sfn_invert(int32_t *res_ptr, uint32_t *in_ptr,
                                   uint32_t *out_ptr, int32_t len)
{
    int32_t i;
    static uint32_t invert_buffer[SFN_INVERT_MAX_LEN];

    if (tfm_core_memory_permission_check(res_ptr, sizeof(int32_t),
        TFM_MEMORY_ACCESS_RW) != TFM_SUCCESS) {
        return CORE_TEST_ERRNO_INVALID_BUFFER;;
    }

    *res_ptr = -1;

    if (len > SFN_INVERT_MAX_LEN) {
        return CORE_TEST_ERRNO_INVALID_BUFFER;;
    }

    /* Check requires byte-based size */
    if ((tfm_core_memory_permission_check(in_ptr, len << 2,
        TFM_MEMORY_ACCESS_RW) != TFM_SUCCESS) ||
        (tfm_core_memory_permission_check(out_ptr, len << 2,
        TFM_MEMORY_ACCESS_RW) != TFM_SUCCESS)) {
        return CORE_TEST_ERRNO_INVALID_BUFFER;;
    }

    for (i = 0; i < len; i++) {
        invert_buffer[i] = in_ptr[i];
    }
    for (i = 0; i < len; i++) {
        invert_buffer[i] = ~invert_buffer[i];
    }
    for (i = 0; i < len; i++) {
        out_ptr[i] = invert_buffer[i];
    }

    *res_ptr = 0;
    return TFM_SUCCESS;
}
