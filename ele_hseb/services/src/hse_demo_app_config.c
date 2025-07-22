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

#include "hse_global_variables.h"
#include "hse_demo_app_services.h"
#include "hse_mu.h"
#include "hse_she_api.h"
#include "hse_host.h"
#include "string.h"
#include "hse_b_catalog_formatting.h"
#include "fsl_debug_console.h"

extern volatile hseSrvResponse_t gsrvResponse;
extern volatile eHSEFWAttributes gProgramAttributes;
extern volatile eHSEFWAttributes gEnableIVTAuthBit;
volatile fwteststatus_t gInstallHSEFwTest = FW_NOT_INSTALLED;
hseSrvResponse_t formatkey_srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
volatile uint8_t ghseCapabilites[8] = {0U};


void WaitForHSEFWInitToFinish()
{
    /* Wait for HSE to initialize(read status bits) after installation */
    while ((HSE_STATUS_INIT_OK & HSE_MU_GetHseStatus(0)) == 0)
    {
        gInstallHSEFwTest = FW_NOT_INSTALLED;
        PRINTF("HSEB FW is NOT installed, please follow README for installation steps.\r\n");
    }
    /* firmware installed, read hse */
    gInstallHSEFwTest = FW_INSTALLED;
}

/* Function to Check if NVM and RAM keys already formatted format NVM and RAM key catalog
   import keys for cryptographic operation and secure boot*/
void HSE_DemoAppConfigKeys()
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    
    PRINTF("\r\nCheck if NVM and RAM key catalogs are formatted...");
    if(true!= IsKeyCatalogFormatted())
    {
        PRINTF("Key catalogs are not formatted\r\n");
        PRINTF("Formatting key catalogs...");
        formatkey_srvResponse = FormatKeyCatalog();
        if (HSE_SRV_RSP_OK == formatkey_srvResponse)
        {
            testStatus |= KEY_CATALOGS_FORMATTED;
            PRINTF("Success\r\n");
        }
    }
    else
    {
        PRINTF("Key catalogs are formatted\r\n");
    }
    
    /* import keys for cryptographic operation */
    PRINTF("Import symmetric keys for cryptographic operation...");
    srvResponse = ImportSymmetricKeys();
    if (srvResponse != HSE_SRV_RSP_OK)
    {
        gCryptoServicesExecuted = CRYPTO_SERVICE_SUCCESS_NONE;
    }
    else
    {
        PRINTF("Success\r\n");
    }
}

void CopyFlashToSRAMHseHost(void)
{
    volatile uint32_t *FlashRamAddr = 0U;
    volatile uint32_t *FlashSrcAddr = 0U;
    volatile uint32_t *FlashSrcEndAddr = 0U;
    uint32_t i = 0U;

    FlashRamAddr = HSE_HOST_RAM_DST_START_ADDR;
    FlashSrcAddr = HSE_HOST_FLASH_SRC_START_ADDR;
    FlashSrcEndAddr = HSE_HOST_FLASH_SRC_END_ADDR;

    /*Copying the Flash Driver from flash to RAM to avoid read while write
     *  violation */
    for (i = 0U; i < (((uint32_t)FlashSrcEndAddr - (uint32_t)FlashSrcAddr) / 4U); i++)
    {
        FlashRamAddr[i] = FlashSrcAddr[i];
    }
}

/*******************************************************************************
 * Function:    Get_Attr
 * Description: Gets the specified attribute
 * Returns:     Service response code of the operation.
 ******************************************************************************/
hseSrvResponse_t Get_Attr(hseAttrId_t attrId, uint32_t attrLen, void *pAttr)
{
    hseSrvResponse_t hseSrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    hseSrvDescriptor_t *pHseSrvDesc;
    uint8_t u8MuChannel;
    /* Get a free channel on MU0 */
    u8MuChannel = HSE_GetFreeChannel(MU0);
    if (HSE_INVALID_CHANNEL == u8MuChannel)
    {
        goto failed;
    }
    /* Clear the service descriptor placed in shared memory */
    pHseSrvDesc = &gHseSrvDesc[MU0][u8MuChannel];
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_GET_ATTR;
    pHseSrvDesc->hseSrv.getAttrReq.attrId = attrId;
    pHseSrvDesc->hseSrv.getAttrReq.attrLen = attrLen;
    pHseSrvDesc->hseSrv.getAttrReq.pAttr = (HOST_ADDR)pAttr;
    hseSrvResponse = HSE_Send(MU0, u8MuChannel, gSyncTxOption, pHseSrvDesc);
    if (HSE_SRV_RSP_OK != hseSrvResponse)
    {
        goto failed;
    }

failed:
    return hseSrvResponse;
}


void attributeProgrammingService(eHSEFWAttributes gProgramAttribute)
{
   /* read hse fw version */
        if (READ_HSE_VERSION == gProgramAttribute)
        {
          getFwVersionService();
        }
        /* read hse fw capabilities */
        if (READ_HSE_CAPABILITIES == gProgramAttribute)
        {
          HSE_GetCapabilities_Example((uint8_t *)&ghseCapabilites);
        }
        /* program adkp key */
        if (PROGRAM_APPLICATION_DEBUG_AUTH_KEY == gProgramAttribute)
        {
          ProgramADKPService();
        }
        /*user requested debug authorization or debug auth mode change*/
        if ((PROGRAM_DEBUG_AUTH == gProgramAttribute))
        {
          Debug_Auth_Service();
        }
        /*
         *  User has request ADKP master bit to be set i.e.
         *  password generated for debug authorization will always be unique.
         *  Note: ADKP Key shall be programmed prior to this operation
         *  This is one-time programmable only and cannot be revered
         */
        if (PROGRAM_EXTEND_CUST_SECURITY_POLICY == gProgramAttribute)
        {
          ExtendCustomerSecurityPolicyService();
        }
        /*user has requested life cycle advancement*/
        if (ADVANCE_LC == gProgramAttribute)
        {
          Advance_LifeCycle_Service();
        }
        /*
         * user can requested IVT auth bit to be set by two ways
         * either by attribute programming option
         * or by enabling boot authentication.
         * Note: ADKP shall be programmed prior to this operation
         * and this is one-time programmable and operation cannot be reversed
         * back i.e. before HSE or app boot, IVT will always be authenticated
         */
        if ((ENABLE_AUTH_MODE == gProgramAttribute) || (ENABLE_AUTH_MODE == gEnableIVTAuthBit))
        {
          IVT_Auth_Service();
        }
        /*user requested second MU to be enabled*/
        if (CONFIG_MU == gProgramAttribute)
        {
          MU_EnablementService();
        }
}

