/*
 * Copyright (c) 2017-2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "sst_tests.h"

#include <stdio.h>
#include <string.h>

#include "test/framework/test_framework_helpers.h"
#include "secure_fw/services/secure_storage/assets/sst_asset_defs.h"
#include "secure_fw/services/secure_storage/sst_object_system.h"
#include "psa_sst_api.h"
#include "s_test_helpers.h"

/* Test suite defines */
#define INVALID_ASSET_ID           0xFFFF
#define READ_BUF_SIZE                14UL
#define WRITE_BUF_SIZE                5UL

/* Memory bounds to check */
#define ROM_ADDR_LOCATION        0x10000000
#define DEV_ADDR_LOCATION        0x50000000
#define NON_EXIST_ADDR_LOCATION  0xFFFFFFFF

/* Test data sized to fill the SHA224 asset */
#define READ_DATA_SHA224    "XXXXXXXXXXXXXXXXXXXXXXXXXXXX"
#define WRITE_DATA_SHA224_1 "TEST_DATA_ONE_TWO_THREE_FOUR"
#define WRITE_DATA_SHA224_2 "(ABCDEFGHIJKLMNOPQRSTUVWXYZ)"
#define BUF_SIZE_SHA224     (SST_ASSET_MAX_SIZE_SHA224_HASH + 1)

/* Define used for bounds checking type tests */
#define BUFFER_SIZE_PLUS_ONE (BUFFER_SIZE + 1)

/* Define default asset's token */
#define ASSET_TOKEN      NULL
#define ASSET_TOKEN_SIZE 0

/* Define test suite for asset manager tests */
/* List of tests */
static void tfm_sst_test_2001(struct test_result_t *ret);
static void tfm_sst_test_2002(struct test_result_t *ret);
static void tfm_sst_test_2003(struct test_result_t *ret);
static void tfm_sst_test_2004(struct test_result_t *ret);
static void tfm_sst_test_2005(struct test_result_t *ret);
static void tfm_sst_test_2006(struct test_result_t *ret);
static void tfm_sst_test_2007(struct test_result_t *ret);
static void tfm_sst_test_2008(struct test_result_t *ret);
static void tfm_sst_test_2009(struct test_result_t *ret);
static void tfm_sst_test_2010(struct test_result_t *ret);
static void tfm_sst_test_2011(struct test_result_t *ret);
static void tfm_sst_test_2012(struct test_result_t *ret);
static void tfm_sst_test_2013(struct test_result_t *ret);
#ifdef SST_ENABLE_PARTIAL_ASSET_RW
static void tfm_sst_test_2014(struct test_result_t *ret);
static void tfm_sst_test_2015(struct test_result_t *ret);
static void tfm_sst_test_2016(struct test_result_t *ret);
#endif
static void tfm_sst_test_2017(struct test_result_t *ret);
static void tfm_sst_test_2018(struct test_result_t *ret);
static void tfm_sst_test_2019(struct test_result_t *ret);
#ifdef SST_ENABLE_PARTIAL_ASSET_RW
static void tfm_sst_test_2020(struct test_result_t *ret);
#endif

static struct test_t write_tests[] = {
    {&tfm_sst_test_2001, "TFM_SST_TEST_2001",
     "Create interface", {0} },
    {&tfm_sst_test_2002, "TFM_SST_TEST_2002",
     "Get information interface", {0} },
    {&tfm_sst_test_2003, "TFM_SST_TEST_2003",
     "Get information with null attributes struct pointer", {0} },
    {&tfm_sst_test_2004, "TFM_SST_TEST_2004",
     "Write interface", {0} },
    {&tfm_sst_test_2005, "TFM_SST_TEST_2005",
     "Write with null buffer pointers", {0} },
    {&tfm_sst_test_2006, "TFM_SST_TEST_2006",
     "Write beyond end of asset", {0} },
    {&tfm_sst_test_2007, "TFM_SST_TEST_2007",
     "Read interface", {0} },
    {&tfm_sst_test_2008, "TFM_SST_TEST_2008",
     "Read with null buffer pointers", {0} },
    {&tfm_sst_test_2009, "TFM_SST_TEST_2009",
     "Read beyond current size of asset", {0} },
    {&tfm_sst_test_2010, "TFM_SST_TEST_2010",
     "Delete interface", {0} },
    {&tfm_sst_test_2011, "TFM_SST_TEST_2011",
     "Write and partial reads", {0} },
    {&tfm_sst_test_2012, "TFM_SST_TEST_2012",
     "Write partial data in an asset and reload secure storage area", {0} },
    {&tfm_sst_test_2013, "TFM_SST_TEST_2013",
     "Write more data than asset max size", {0} },
#ifdef SST_ENABLE_PARTIAL_ASSET_RW
    {&tfm_sst_test_2014, "TFM_SST_TEST_2014",
     "Append data to an asset", {0} },
    {&tfm_sst_test_2015, "TFM_SST_TEST_2015",
     "Append data to an asset until EOF", {0} },
    {&tfm_sst_test_2016, "TFM_SST_TEST_2016",
     "Write data to two assets alternately", {0} },
#endif
    {&tfm_sst_test_2017, "TFM_SST_TEST_2017",
     "Access an illegal location: ROM", {0} },
    {&tfm_sst_test_2018, "TFM_SST_TEST_2018",
     "Access an illegal location: device memory", {0} },
    {&tfm_sst_test_2019, "TFM_SST_TEST_2019",
     "Access an illegal location: non-existant memory", {0} },
#ifdef SST_ENABLE_PARTIAL_ASSET_RW
    {&tfm_sst_test_2020, "TFM_SST_TEST_2020",
     "Write data to the middle of an existing asset", {0} },
#endif
};

void register_testsuite_s_sst_sec_interface(struct test_suite_t *p_test_suite)
{
    uint32_t list_size = (sizeof(write_tests) / sizeof(write_tests[0]));

    set_testsuite("SST secure interface tests (TFM_SST_TEST_2XXX)",
                  write_tests, list_size, p_test_suite);

#ifdef SST_SHOW_FLASH_WARNING
    TEST_LOG("\r\n**WARNING** The SST regression tests reduce the life of the "
             "flash memory as they write/erase multiple times the memory. \r\n"
             "Please, set the SST_RAM_FS flag to use RAM instead of flash."
             "\r\n\r\n");
#endif
}

/**
 * \brief Tests create function against:
 * - Valid client ID and asset ID
 * - Invalid asset ID
 * - Invalid client ID
 */
static void tfm_sst_test_2001(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Checks write permissions in create function */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Attempts to create the asset a second time */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Should not fail to create an already-created asset");
        return;
    }

    /* Calls create with invalid asset ID */
    err = psa_sst_create(INVALID_ASSET_ID, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Create should fail for invalid ASSET ID");
        return;
    }

#ifdef INVALID_CLIENT_ID_TEST
    /* Calls create with invalid client ID */
    /* FIXME: Add test help call to change the client ID to an invalid one */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Create should fail for invalid client ID");
        return;
    }
#endif

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests get attributes function against:
 * - Valid client ID and attributes struct pointer
 * - Invalid client ID
 * - Invalid asset ID
 * - Invalid client ID
 */
static void tfm_sst_test_2002(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    struct psa_sst_asset_info_t asset_info;
    enum psa_sst_err_t err;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Calls get_attributes with valid client ID and
     * attributes struct pointer
     */
    err = psa_sst_get_info(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           &asset_info);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Client S_CLIENT_ID should be able to read the "
                  "information of this asset");
        return;
    }

    /* Checks attributes */
    if (asset_info.size_current != 0) {
        TEST_FAIL("Asset current size should be 0 as it is only created");
        return;
    }

    if (asset_info.size_max != SST_ASSET_MAX_SIZE_AES_KEY_192) {
        TEST_FAIL("Max size of the asset is incorrect");
        return;
    }

    /* Calls get information with invalid asset ID */
    err = psa_sst_get_info(INVALID_ASSET_ID, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           &asset_info);
    if (err == PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Get attributes function should fail for an invalid "
                  "asset ID");
        return;
    }

#ifdef INVALID_CLIENT_ID_TEST
    /* Calls get_attributes with invalid client ID */
    /* FIXME: Add test help call to change the client ID to an invalid one */
    err = psa_sst_get_info(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           &asset_info);
    if (err == PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Get information function should fail for an invalid "
                  "client ID");
        return;
    }
#endif

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests get attributes function with an invalid attributes struct
 *        pointer.
 */
static void tfm_sst_test_2003(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Calls get information with invalid struct attributes pointer */
    err = psa_sst_get_info(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE, NULL);
    if (err != PSA_SST_ERR_PARAM_ERROR) {
        TEST_FAIL("Get information function should fail for an invalid "
                  "struct info pointer");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests write function against:
 * - Valid client ID and data pointer
 * - Invalid asset ID
 * - Invalid client ID
 */
static void tfm_sst_test_2004(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    struct psa_sst_asset_info_t asset_info;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    uint8_t wrt_data[WRITE_BUF_SIZE] = "DATA";

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 0;

    /* Writes data in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write should works correctly");
        return;
    }

    /* Calls get information with valid client ID and
     * attributes struct pointer
     */
    err = psa_sst_get_info(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           &asset_info);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Client S_CLIENT_ID should be able to read the "
                  "information of this asset");
        return;
    }

    /* Checks attributes */
    if (asset_info.size_current != WRITE_BUF_SIZE) {
        TEST_FAIL("Asset current size should be size of the write data");
        return;
    }

    /* Calls write function with invalid asset ID */
    err = psa_sst_write(INVALID_ASSET_ID, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Invalid asset ID should not write in the file");
        return;
    }

#ifdef INVALID_CLIENT_ID_TEST
    /* Calls write function with invalid client ID */
    /* FIXME: Add test help call to change the client ID to an invalid one */
    err = tfm_sst_veneer_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                               io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Invalid client ID should not write in the file");
        return;
    }
#endif

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests write function with:
 * - Null write buffer pointer
 */
static void tfm_sst_test_2005(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = NULL;
    io_data.size = 1;
    io_data.offset = 0;

    /* Calls write function with data pointer set to NULL */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Write should fail with data pointer set to NULL");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests write function with offset + write data size larger than max
 *        asset size.
 */
static void tfm_sst_test_2006(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    uint8_t wrt_data[BUFFER_PLUS_PADDING_SIZE] = {0};

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Attempts to write beyond end of asset starting from a valid offset */
    io_data.data = wrt_data;
    io_data.size = BUFFER_SIZE_PLUS_ONE;
    io_data.offset = 0;

    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err == PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Writing beyond end of asset should not succeed");
        return;
    }

    /* Attempts to write to an offset beyond the end of the asset */
    io_data.size = 1;
    io_data.offset = SST_ASSET_MAX_SIZE_AES_KEY_192;

    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err == PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write to an offset beyond end of asset should not succeed");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests read function against:
 * - Valid client ID and data pointer
 * - Invalid asset ID
 * - Invalid client ID
 */
static void tfm_sst_test_2007(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    uint8_t wrt_data[WRITE_BUF_SIZE] = "DATA";
    uint8_t read_data[READ_BUF_SIZE] = "XXXXXXXXXXXXX";

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 0;

    /* Writes data in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write should works correctly");
        return;
    }

    /* Sets data structure for read*/
    io_data.data = read_data + HALF_PADDING_SIZE;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 0;

    /* Read data from the asset */
    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Read should works correctly");
        return;
    }

    if (memcmp(read_data, "XXXX", HALF_PADDING_SIZE) != 0) {
        TEST_FAIL("Read buffer contains illegal pre-data");
        return;
    }

    if (memcmp((read_data+HALF_PADDING_SIZE), wrt_data, WRITE_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer has read incorrect data");
        return;
    }

    if (memcmp((read_data+HALF_PADDING_SIZE+WRITE_BUF_SIZE), "XXXX",
                HALF_PADDING_SIZE) != 0) {
        TEST_FAIL("Read buffer contains illegal post-data");
        return;
    }

    /* Calls read with invalid asset ID */
    err = psa_sst_read(INVALID_ASSET_ID, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read should fail when read is called with an invalid "
                  "asset ID");
        return;
    }

#ifdef INVALID_CLIENT_ID_TEST
    /* Calls read with invalid client ID */
    /* FIXME: Add test help call to change the client ID to an invalid one */
    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read should fail when read is called with an invalid "
                  "client ID");
        return;
    }
#endif

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests read function with:
 * - Null read buffer pointer
 */
static void tfm_sst_test_2008(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    io_data.data = NULL;
    io_data.size = 1;
    io_data.offset = 0;

    /* Calls read with invalid data pointer */
    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read with read data pointer set to NULL should fail");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests read function with offset + read data size larger than current
 *        asset size.
 */
static void tfm_sst_test_2009(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    struct psa_sst_asset_info_t asset_info;
    uint8_t data[BUFFER_SIZE_PLUS_ONE] = {0};

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = data;
    io_data.size = SST_ASSET_MAX_SIZE_AES_KEY_192;
    io_data.offset = 0;

    /* Writes data in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write should works correctly");
        return;
    }

    /* Gets current asset information */
    err = psa_sst_get_info(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           &asset_info);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Client S_CLIENT_ID should be able to read the "
                  "information of this asset");
        return;
    }

    /* Checks attributes */
    if (asset_info.size_current == 0) {
        TEST_FAIL("Asset current size should be bigger than 0");
        return;
    }

    /* Attempts to read beyond the current size starting from a valid offset */
    io_data.data = data;
    io_data.size = BUFFER_SIZE_PLUS_ONE;
    io_data.offset = 0;

    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err == PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Read beyond current size should not succeed");
        return;
    }

    /* Attempts to read from an offset beyond the current size of the asset */
    io_data.size = 1;
    io_data.offset = asset_info.size_current;

    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE, io_data.size,
                       io_data.offset, io_data.data);
    if (err == PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Read from an offset beyond current size should not succeed");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests delete function against:
 * - Valid client ID
 * - Invalid client ID
 * - Invalid asset ID
 * - Remove first asset in the data block and check if
 *   next asset's data is compacted correctly.
 */
static void tfm_sst_test_2010(struct test_result_t *ret)
{
    const uint32_t asset_uuid_1 =  SST_ASSET_ID_SHA224_HASH;
    const uint32_t asset_uuid_2 = SST_ASSET_ID_SHA384_HASH;
    struct sst_test_buf_t io_data;
    enum psa_sst_err_t err;
    uint8_t read_data[BUF_SIZE_SHA224] = READ_DATA_SHA224;
    uint8_t wrt_data[BUF_SIZE_SHA224] = WRITE_DATA_SHA224_1;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates assset */
    err = psa_sst_create(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

#ifdef INVALID_CLIENT_ID_TEST
    /* Calls delete asset with invalid client ID */
    /* FIXME: Add test help call to change the client ID to an invalid one */
    err = psa_sst_delete(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("The delete action should fail if an invalid client "
                  "ID is provided");
        return;
    }
#endif

    /* Calls delete asset */
    err = psa_sst_delete(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("The delete action should work correctly");
        return;
    }

    /* Calls delete with a deleted asset ID */
    err = psa_sst_delete(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("The delete action should fail as ID is not valid");
        return;
    }

    /* Calls delete asset with invalid asset ID */
    err = psa_sst_delete(INVALID_ASSET_ID, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("The delete action should fail if an invalid asset ID "
                  "is provided");
        return;
    }

    /***** Test data block compact feature *****/
    /* Create asset 2 to locate it at the beginning of the block. Then,
     * create asset 1 to be located after asset 2. Write data on asset
     * 1 and remove asset 2. If delete works correctly, when the code
     * reads back the asset 1 data, the data must be correct.
     */

    /* Creates assset 2 first to locate it at the beginning of the
     * data block
     */
    err = psa_sst_create(asset_uuid_2, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Creates asset 1 to locate it after the asset 2 in the data block */
    err = psa_sst_create(SST_ASSET_ID_SHA224_HASH,
                         ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data;
    io_data.size = SST_ASSET_MAX_SIZE_SHA224_HASH;
    io_data.offset = 0;

    /* Writes data in asset 1 */
    err = psa_sst_write(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data should work for client S_CLIENT_ID");
        return;
    }

    /* Deletes asset 2. It means that after the delete call, asset 1 should be
     * at the beginning of the block.
     */
    err = psa_sst_delete(asset_uuid_2, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("The delete action should work correctly");
        return;
    }

    /* If compact works as expected, the code should be able to read back
     * correctly the data from asset 1
     */

    /* Sets data structure */
    io_data.data = read_data;
    io_data.size = SST_ASSET_MAX_SIZE_SHA224_HASH;
    io_data.offset = 0;

    /* Read back the asset 1 */
    err = psa_sst_read(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Incorrect number of bytes read back");
        return;
    }

    if (memcmp(read_data, wrt_data, BUF_SIZE_SHA224) != 0) {
        TEST_FAIL("Read buffer has incorrect data");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests write and partial reads.
 */
static void tfm_sst_test_2011(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    uint32_t i;
    uint8_t read_data[READ_BUF_SIZE] = "XXXXXXXXXXXXX";
    uint8_t wrt_data[WRITE_BUF_SIZE] = "DATA";

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 0;

    /* Writes data in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write should works correctly");
        return;
    }

    /* Sets data structure for read */
    io_data.data = read_data + HALF_PADDING_SIZE;
    io_data.size = 1;
    io_data.offset = 0;

    for (i = 0; i < WRITE_BUF_SIZE ; i++) {
        /* Read data from the asset */
        err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           io_data.size, io_data.offset, io_data.data);
#ifdef SST_ENABLE_PARTIAL_ASSET_RW
        if (err != PSA_SST_ERR_SUCCESS) {
#else
        if (io_data.offset != 0 && err != PSA_SST_ERR_PARAM_ERROR) {
#endif
            TEST_FAIL("Read did not behave correctly");
            return;
        }

        /* Increases data pointer and offset */
        io_data.data++;
        io_data.offset++;
    }

    if (memcmp(read_data, "XXXX", HALF_PADDING_SIZE) != 0) {
        TEST_FAIL("Read buffer contains illegal pre-data");
        return;
    }

#ifdef SST_ENABLE_PARTIAL_ASSET_RW
    if (memcmp((read_data+HALF_PADDING_SIZE), wrt_data, WRITE_BUF_SIZE) != 0) {
#else
    /* Only the first read ("D") is from a valid offset (0) */
    if (memcmp((read_data+HALF_PADDING_SIZE), "DXXXX", WRITE_BUF_SIZE) != 0) {
#endif
        TEST_FAIL("Read buffer has read incorrect data");
        return;
    }

    if (memcmp((read_data+HALF_PADDING_SIZE+WRITE_BUF_SIZE), "XXXX",
                HALF_PADDING_SIZE) != 0) {
        TEST_FAIL("Read buffer contains illegal post-data");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests correct behaviour when data is written in the secure storage
 *        area and the secure_fs_perpare is called after it.
 *        The expected behaviour is to read back the data wrote
 *        before the seconds perpare call.
 */
static void tfm_sst_test_2012(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    uint8_t read_data[READ_BUF_SIZE] = "XXXXXXXXXXXXX";
    uint8_t wrt_data[WRITE_BUF_SIZE] = "DATA";

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 0;

    /* Writes data in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write should works correctly");
        return;
    }

    /* Calls prepare again to simulate reinitialization */
    err = sst_am_prepare();
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Saved system should have been preparable");
        return;
    }

    /* Sets data structure */
    io_data.data = read_data + HALF_PADDING_SIZE;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 0;

    /* Reads back the data after the prepare */
    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Incorrect number of bytes read back");
        return;
    }

    if (memcmp(read_data, "XXXX", HALF_PADDING_SIZE)) {
        TEST_FAIL("Read buffer contains illegal pre-data");
        return;
    }

    if (memcmp((read_data+HALF_PADDING_SIZE), wrt_data, WRITE_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer has read incorrect data");
        return;
    }

    if (memcmp((read_data+HALF_PADDING_SIZE+WRITE_BUF_SIZE), "XXXX",
                HALF_PADDING_SIZE) != 0) {
        TEST_FAIL("Read buffer contains illegal post-data");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests write function against a write call where data size is
 *        bigger than the maximum assert size.
 */
static void tfm_sst_test_2013(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    uint8_t wrt_data[BUF_SIZE_SHA224] = {0};

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data;
    io_data.size = SST_ASSET_MAX_SIZE_SHA224_HASH + 1;
    io_data.offset = 0;

    /* Writes data in the asset when data size is bigger than asset size */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err == PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Should have failed asset write of too large");
        return;
    }

    ret->val = TEST_PASSED;
}

#ifdef SST_ENABLE_PARTIAL_ASSET_RW
/**
 * \brief Tests write function against multiple writes.
 */
static void tfm_sst_test_2014(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_SHA224_HASH;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    uint8_t read_data[READ_BUF_SIZE]  = "XXXXXXXXXXXXX";
    uint8_t wrt_data[WRITE_BUF_SIZE+1]  = "Hello";
    uint8_t wrt_data2[WRITE_BUF_SIZE+1] = "World";

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 0;

    /* Writes data in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data 1 failed");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data2;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = WRITE_BUF_SIZE;

    /* Writes data 2 in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data 2 failed");
        return;
    }

    /* Sets data structure */
    io_data.data = read_data;
    io_data.size = WRITE_BUF_SIZE * 2;
    io_data.offset = 0;

    /* Read back the data */
    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Incorrect number of bytes read back");
        return;
    }

    /* The X is used to check that the number of bytes read was exactly the
     * number requested
     */
    if (memcmp(read_data, "HelloWorldXXX", READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer has read incorrect data");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests write function against multiple writes until the end of asset.
 */
static void tfm_sst_test_2015(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_SHA224_HASH;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    uint8_t read_data[BUF_SIZE_SHA224] = READ_DATA_SHA224;
    uint8_t wrt_data[BUF_SIZE_SHA224] = WRITE_DATA_SHA224_1;
    uint8_t wrt_data2[BUF_SIZE_SHA224] = WRITE_DATA_SHA224_2;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 0;

    /* Writes data in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data 1 failed");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data2;
    io_data.size = (SST_ASSET_MAX_SIZE_SHA224_HASH - WRITE_BUF_SIZE) + 1;
    io_data.offset = WRITE_BUF_SIZE;

    /* Writes data in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err == PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data 2 should have failed as this write tries to "
                  "write more bytes than the max size");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data + WRITE_BUF_SIZE;
    io_data.size = SST_ASSET_MAX_SIZE_SHA224_HASH - WRITE_BUF_SIZE;
    io_data.offset = WRITE_BUF_SIZE;

    /* Writes data in the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data 3 failed");
        return;
    }

    /* Sets data structure */
    io_data.data = read_data;
    io_data.size = SST_ASSET_MAX_SIZE_SHA224_HASH;
    io_data.offset = 0;

    /* Read back the data */
    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Incorrect number of bytes read back");
        return;
    }

    if (memcmp(read_data, wrt_data, BUF_SIZE_SHA224) != 0) {
        TEST_FAIL("Read buffer has incorrect data");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests writing data to two assets alternately before read-back.
 */
static void tfm_sst_test_2016(struct test_result_t *ret)
{
    const uint32_t asset_uuid_1 = SST_ASSET_ID_AES_KEY_192;
    const uint32_t asset_uuid_2 = SST_ASSET_ID_SHA224_HASH;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;
    uint8_t read_data[READ_BUF_SIZE] = "XXXXXXXXXXXXX";
    uint8_t wrt_data[WRITE_BUF_SIZE+1] = "Hello";
    uint8_t wrt_data2[3] = "Hi";
    uint8_t wrt_data3[WRITE_BUF_SIZE+1] = "World";
    uint8_t wrt_data4[WRITE_BUF_SIZE+1] = "12345";

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset 1 */
    err = psa_sst_create(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Creates asset 2 */
    err = psa_sst_create(asset_uuid_2, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 0;

    /* Writes data in asset 1 */
    err = psa_sst_write(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data should work for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data2;
    io_data.size = 2;
    io_data.offset = 0;

    /* Writes data 2 in asset 2 */
    err = psa_sst_write(asset_uuid_2, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data should work for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data3;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = WRITE_BUF_SIZE;

    /* Writes data 3 in asset 1 */
    err = psa_sst_write(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data should work for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = wrt_data4;
    io_data.size = WRITE_BUF_SIZE;
    io_data.offset = 2;

    /* Writes data 4 in asset 2 */
    err = psa_sst_write(asset_uuid_2, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Write data should work for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = read_data;
    io_data.size = WRITE_BUF_SIZE * 2; /* size of wrt_data + wrt_data3 */
    io_data.offset = 0;

    /* Read back the asset 1 */
    err = psa_sst_read(asset_uuid_1, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Incorrect number of bytes read back");
        return;
    }

    if (memcmp(read_data, "HelloWorldXXX", READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer has incorrect data");
        return;
    }

    /* Resets read buffer content to a known data */
    memset(read_data, 'X', READ_BUF_SIZE - 1);

    /* Sets data structure */
    io_data.data = read_data;
    io_data.size = 2 + WRITE_BUF_SIZE; /* size of wrt_data2 + wrt_data4 */
    io_data.offset = 0;

    /* Read back the asset 2 */
    err = psa_sst_read(asset_uuid_2, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Incorrect number of bytes read back");
        return;
    }

    if (memcmp(read_data, "Hi12345XXXXXX", READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer has incorrect data");
        return;
    }

    ret->val = TEST_PASSED;
}
#endif /* SST_ENABLE_PARTIAL_ASSET_RW */

/**
 * \brief Tests read from and write to an illegal location: ROM.
 */
static void tfm_sst_test_2017(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_SHA224_HASH;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = (uint8_t *)ROM_ADDR_LOCATION;
    io_data.size = 1;
    io_data.offset = 0;

    /* Calls write with a ROM address location */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Write should fail for an illegal location");
        return;
    }

    /* Calls read with a ROM address location */
    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read should fail for an illegal location");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests read from and write to an illegal location: device memory.
 */
static void tfm_sst_test_2018(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_SHA224_HASH;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = (uint8_t *)DEV_ADDR_LOCATION;
    io_data.size = 1;
    io_data.offset = 0;

    /* Calls write with a device address location */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Write should fail for an illegal location");
        return;
    }

    /* Calls read with a device address location */
    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read should fail for an illegal location");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests read from and write to an illegal location: non-existant memory.
 */
static void tfm_sst_test_2019(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_SHA224_HASH;
    enum psa_sst_err_t err;
    struct sst_test_buf_t io_data;

    /* Prepares test context */
    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    /* Creates asset */
    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for client S_CLIENT_ID");
        return;
    }

    /* Sets data structure */
    io_data.data = (uint8_t *)NON_EXIST_ADDR_LOCATION;
    io_data.size = 1;
    io_data.offset = 0;

    /* Calls write with a non-existing address location */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Write should fail for an illegal location");
        return;
    }

    /* Calls read with a non-existing address location */
    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read should fail for an illegal location");
        return;
    }

    ret->val = TEST_PASSED;
}

#ifdef SST_ENABLE_PARTIAL_ASSET_RW
/**
 * \brief Writes data to the middle of an existing asset.
 */
static void tfm_sst_test_2020(struct test_result_t *ret)
{
    const uint32_t asset_uuid = SST_ASSET_ID_AES_KEY_192;
    struct psa_sst_asset_info_t asset_info;
    struct sst_test_buf_t io_data;
    enum psa_sst_err_t err;
    uint8_t read_data[READ_BUF_SIZE] = "XXXXXXXXXXXXX";
    uint8_t write_data_1[WRITE_BUF_SIZE] = "AAAA";
    uint8_t write_data_2[2] = "B";

    if (prepare_test_ctx(ret) != 0) {
        TEST_FAIL("Prepare test context should not fail");
        return;
    }

    err = psa_sst_create(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail");
        return;
    }

    io_data.data = write_data_1;
    io_data.size = (WRITE_BUF_SIZE - 1);
    io_data.offset = 0;

    /* Writes write_data_1 to the asset */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("First write should not fail");
        return;
    }

    err = psa_sst_get_info(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           &asset_info);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Get information should not fail");
        return;
    }

    /* Checks that the asset's current size is equal to the size of the write
     * data.
     */
    if (asset_info.size_current != WRITE_BUF_SIZE - 1) {
        TEST_FAIL("Current size should be equal to write size");
        return;
    }

    io_data.data = write_data_2;
    io_data.size = 1;
    io_data.offset = 1;

    /* Overwrites the second character in the asset with write_data_2 */
    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Second write should not fail");
        return;
    }

    err = psa_sst_get_info(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                           &asset_info);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Get information should not fail");
        return;
    }

    /* Checks that the asset's current size has not changed */
    if (asset_info.size_current != (WRITE_BUF_SIZE - 1)) {
        TEST_FAIL("Current size should not have changed");
        return;
    }

    io_data.data = (read_data + HALF_PADDING_SIZE);
    io_data.size = (WRITE_BUF_SIZE - 1);
    io_data.offset = 0;

    err = psa_sst_read(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                       io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_SUCCESS) {
        TEST_FAIL("Read should not fail");
        return;
    }

    /* Checks that the asset contains write_data_1 with the second character
     * overwritten with write_data_2.
     */
    if (memcmp(read_data, "XXXXABAAXXXXX", READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer is incorrect");
        return;
    }

    /* Checks that offset can not be bigger than current asset's size */
    io_data.data = write_data_2;
    io_data.size = 1;
    io_data.offset = (asset_info.size_current + 1);

    err = psa_sst_write(asset_uuid, ASSET_TOKEN, ASSET_TOKEN_SIZE,
                        io_data.size, io_data.offset, io_data.data);
    if (err != PSA_SST_ERR_PARAM_ERROR) {
        TEST_FAIL("Write must fail if the offset is bigger than the current"
                  " asset's size");
        return;
    }

    ret->val = TEST_PASSED;
}
#endif /* SST_ENABLE_PARTIAL_ASSET_RW */
