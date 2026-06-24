/**
 *   @file    hse_fuses.c
 *
 *   @brief   Examples of HSE fuses programming.
 *
 *   @addtogroup [hse_fuses]
 *   @{
 */
/*==================================================================================================
*
*   Copyright 2022 NXP.
*
*   License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025
==================================================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/

#include "hse_global_variables.h"
#include "hse_host_attrs.h"
#include "hse_demo_app_services.h"

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/
extern volatile hseSrvResponse_t gsrvResponse;
hseAttrMUConfig_t Read_hseMuConfigValue;
extern volatile bool_t write_attr;
extern volatile hseMUConfig_t WriteMu1Config;
extern volatile hseMUConfig_t ReadMu1Config;
/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

/*************************************************************************************************
Function: HSE_ConfigNvmAttributes
    @brief    Example of configuring HSE NVM attributes
    @detailed Configure MU instances that will be active and allocate SHARED_RAM / MU (split done via XRDC)
                (Optional) Extended Security Polices can be configured (StartAsUser)
                These attributes will be part of the SYS_IMG
*************************************************************************************************/
hseSrvResponse_t HSE_ConfigNvmAttributes(hseMUConfig_t gMuConfig)
{
    /* MU configuration to enable all MU instances without XRDC configured - default configuration */
    hseAttrMUConfig_t hseMuXrdcConfig = {
        .muInstances = {
            {.muConfig = HSE_MU_ACTIVATED,
                .xrdcDomainId = 0U,
                .sharedMemChunkSize = 0U,
                .reserved = {0}},
            {.muConfig = gMuConfig,
                .xrdcDomainId = 0U,
                .sharedMemChunkSize = 0U,
                .reserved = {0}}}};
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    
    /* Define which MU instances are activated and configure XRDC over SHARED_RAM */
    srvResponse = SetAttr(HSE_MU_CONFIG_ATTR_ID, sizeof(hseAttrMUConfig_t), (void *)&hseMuXrdcConfig);
    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    return srvResponse;
}

hseSrvResponse_t HSE_GetMUAttribute(hseAttrMUConfig_t *pMuConfigValue)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /* Define which MU instances are activated and configure XRDC over SHARED_RAM */
    srvResponse = Get_Attr(HSE_MU_CONFIG_ATTR_ID, sizeof(hseAttrMUConfig_t), (void *)pMuConfigValue);
    ASSERT(HSE_SRV_RSP_OK == srvResponse);
    return srvResponse;
}

void MU_EnablementService()
{
    gsrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    if (HSE_SRV_RSP_OK == HSE_GetMUAttribute(&Read_hseMuConfigValue) && (TRUE == write_attr) && (Read_hseMuConfigValue.muInstances[MU1].muConfig != WriteMu1Config))
    {
        /*read back the MU configuration*/
        gsrvResponse = HSE_ConfigNvmAttributes(WriteMu1Config);
    }
    if (HSE_SRV_RSP_OK == gsrvResponse)
    {
        gsrvResponse = HSE_GetMUAttribute(&Read_hseMuConfigValue);
    }
    ReadMu1Config = Read_hseMuConfigValue.muInstances[MU1].muConfig;
}

#ifdef __cplusplus
}
#endif

/** @} */
