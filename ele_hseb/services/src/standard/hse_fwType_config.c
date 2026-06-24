/*--------------------------------------------------------------------------*/
/* Copyright 2025 NXP                                                       */
/*                                                                          */
/* License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025 */
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


