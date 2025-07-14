/*--------------------------------------------------------------------------*/
/* Copyright 2025 NXP                                                       */
/*                                                                          */
/* NXP Proprietary. This software is owned or controlled by NXP and may     */
/* only be used strictly in accordance with the applicable license terms.   */
/* By expressly accepting such terms or by downloading, installing,         */
/* activating and/or otherwise using the software, you are agreeing that    */
/* you have read, and that you agree to comply with and are bound by, such  */
/* license terms. If you do not agree to be bound by the applicable license */
/* terms, then you may not retain, install, activate or otherwise use the   */
/* software.                                                                */
/*--------------------------------------------------------------------------*/

#include "hse_b_catalog_formatting.h"
#include "hse_mu.h"
#include "hse_global_variables.h"
#include "hse_demo_app_services.h"

extern volatile hseSrvResponse_t gsrvResponse;
extern volatile eHSEFWAttributes gProgramAttributes;

bool_t IsKeyCatalogFormatted(void)
{
    return CHECK_HSE_STATUS(HSE_STATUS_INSTALL_OK);
}

/*check if HSE FW usage flag is already enabled,
 * if not enabled then do not proceed */
void WaitandSetFWEnablefeatureflag()
{
    while (FALSE == checkHseFwFeatureFlagEnabled())
    {
        /*user has requested to program HSE FW feature flag*/
        if (UTEST_PROGRAM == gProgramAttributes)
        {
            gsrvResponse = EnableHSEFWUsage();
        }
    }
    /*hse fw flag is enabled either already programmed or requested by user*/
    testStatus |= HSE_FW_USAGE_ENABLED;
}


