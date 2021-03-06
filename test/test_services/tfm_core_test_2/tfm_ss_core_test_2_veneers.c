/*
 * Copyright (c) 2017-2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_ss_core_test_2_veneers.h"
#include "tfm_secure_api.h"
#include "tfm_ss_core_test_2.h"
#include "secure_fw/spm/spm_api.h"

/* Add functions to the service*/
__tfm_secure_gateway_attributes__
int32_t tfm_core_test_2_veneer_slave_service(void)
{
   TFM_CORE_SFN_REQUEST(TFM_SP_CORE_TEST_2_ID,
                            spm_core_test_2_slave_service,
                            0, 0, 0, 0);
}

__tfm_secure_gateway_attributes__
int32_t tfm_core_test_2_sfn_invert(int32_t *res_ptr, uint32_t *in_ptr,
                                   uint32_t *out_ptr, int32_t len)
{
    TFM_CORE_SFN_REQUEST(TFM_SP_CORE_TEST_2_ID,
                             spm_core_test_2_sfn_invert,
                             res_ptr, in_ptr, out_ptr, len);
}

__tfm_secure_gateway_attributes__
int32_t tfm_core_test_2_check_caller_client_id(void)
{
   TFM_CORE_SFN_REQUEST(TFM_SP_CORE_TEST_2_ID,
                            spm_core_test_2_check_caller_client_id,
                            0, 0, 0, 0);
}
