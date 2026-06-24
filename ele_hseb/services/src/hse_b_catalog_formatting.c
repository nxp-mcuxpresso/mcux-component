/*--------------------------------------------------------------------------*/
/* Copyright 2022 NXP                                                       */
/*                                                                          */
/* License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025 */
/*--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

#include "hse_host_format_key_catalogs.h"
#include "hse_b_catalog_formatting.h"
#include "global_defs.h"

/*==================================================================================================
 *                                        INCLUDE FILES
 ==================================================================================================*/
#ifdef HSE_SPT_FORMAT_KEY_CATALOGS
const hseKeyGroupCfgEntry_t nvmKeyCatalog[] = {HSE_DEMO_NVM_KEY_CATALOG_CFG};
const hseKeyGroupCfgEntry_t ramKeyCatalog[] = {HSE_DEMO_RAM_KEY_CATALOG_CFG};
#endif

/**
 * @brief         Wrapper for FormatKeyCatalogs.
 * @details
 */
hseSrvResponse_t FormatKeyCatalogs_(void)
{
    #ifdef HSE_SPT_FORMAT_KEY_CATALOGS
    hseSrvResponse_t status;
    status = FormatKeyCatalogs(nvmKeyCatalog, ramKeyCatalog);
    status = ParseKeyCatalogs();
    ASSERT(HSE_SRV_RSP_OK == status);
    return status;
    #else
    return HSE_SRV_RSP_NOT_SUPPORTED;
    #endif
}

/**
 * @brief         Wrapper for KeyCatalogParsing
 * @details
 */

hseSrvResponse_t ParseKeyCatalogs(void)
{
    #ifdef HSE_SPT_FORMAT_KEY_CATALOGS
    hseSrvResponse_t status;
    status = HKF_Init(nvmKeyCatalog, ramKeyCatalog);
    return status;
    #else
    return HSE_SRV_RSP_NOT_SUPPORTED;
    #endif
}

#ifdef __cplusplus
}
#endif

/** @} */
