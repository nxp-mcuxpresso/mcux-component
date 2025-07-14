/**
 *   @file    hse_host_boot.c
 *
 *   @brief   This file implements wrappers for secure boot configuration and HSE FW blue image conversion.
 *
 *   @addtogroup [HOST_FRAMEWORK]
 *   @{
 */
/*==================================================================================================
*
*   Copyright 2022 NXP.
*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
==================================================================================================*/
#include "hse_demo_app_services.h"
#include "hse_host.h"
#include "string.h"
#include "hse_host_flashSrv.h"
#include "hse_host_boot.h"
/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

/*******************************************************************************
 * Function:    HSE_SignBootImage
 * Description: Signs IVT/DCD/SelfTest images or Application image for AppBSB
 ******************************************************************************/
hseSrvResponse_t HSE_SignBootImage(
    const uint8_t *pInImage,
    const uint32_t inTagLength,
    uint8_t *pOutTagAddr)
{
    uint8_t u8MuChannel = 1U;
    hseSrvDescriptor_t *pHseSrvDesc;
    hseBootDataImageSignSrv_t *pBootDataImgSignSrv;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /* Clear the service descriptor placed in shared memory */
    pHseSrvDesc = &gHseSrvDesc[MU0][u8MuChannel];
    pBootDataImgSignSrv = &(pHseSrvDesc->hseSrv.bootDataImageSignReq);
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    /* Fill the service descriptor */
    pHseSrvDesc->srvId = HSE_SRV_ID_BOOT_DATA_IMAGE_SIGN;
    pBootDataImgSignSrv->pInImage = PTR_TO_HOST_ADDR(pInImage);
    pBootDataImgSignSrv->inTagLength = inTagLength;
    pBootDataImgSignSrv->pOutTagAddr = PTR_TO_HOST_ADDR(pOutTagAddr);

    /* Send the request synchronously */
    srvResponse = HSE_Send(MU0, u8MuChannel, gSyncTxOption, pHseSrvDesc);
    return srvResponse;
}

/*******************************************************************************
 * Function:    HSE_VerifyBootImage
 * Description: Verifies the MAC, generated over IVT/DCD/SelfTest/AppBSB images
 *              via hseBootDataImageSign Service
 ******************************************************************************/
hseSrvResponse_t HSE_VerifyBootImage(
    const uint8_t *pInImage)
{
    uint8_t u8MuChannel = 1U;
    hseSrvDescriptor_t *pHseSrvDesc;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /* Clear the service descriptor placed in shared memory */
    pHseSrvDesc = &gHseSrvDesc[MU0][u8MuChannel];
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    /* Fill the service descriptor */
    pHseSrvDesc->srvId = HSE_SRV_ID_BOOT_DATA_IMAGE_VERIFY;
    pHseSrvDesc->hseSrv.bootDataImageSigVerifyReq.pInImage =
        PTR_TO_HOST_ADDR(pInImage);

    /* Send the request synchronously */
    srvResponse = HSE_Send(MU0, u8MuChannel, gSyncTxOption, pHseSrvDesc);
    return srvResponse;
}

#if defined(HSE_SPT_SMR_CR)

/*******************************************************************************
 * Function:    HSE_InstallSmrEntry
 * Description: Installs a SMR entry used for Advanced Secure Boot.
 ******************************************************************************/
hseSrvResponse_t HSE_InstallSmrEntry(
    const uint8_t entryIndex,
    const hseSmrEntry_t *pSmrEntry,
    const uint8_t *pData,
    const uint32_t dataLen,
    const uint8_t *pSign0,
    const uint8_t *pSign1,
    const uint32_t SignLen0,
    const uint32_t SignLen1)
{
    uint8_t u8MuChannel = 1U;
    hseSrvDescriptor_t *pHseSrvDesc;
    hseSmrEntryInstallSrv_t *pSmrEntryInstall;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /* Clear the service descriptor placed in shared memory */
    pHseSrvDesc = &gHseSrvDesc[MU0][u8MuChannel];
    pSmrEntryInstall = &(pHseSrvDesc->hseSrv.smrEntryInstallReq);
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    /* Fill the service descriptor */
    pHseSrvDesc->srvId = HSE_SRV_ID_SMR_ENTRY_INSTALL;
    pSmrEntryInstall->accessMode = HSE_ACCESS_MODE_ONE_PASS;
    pSmrEntryInstall->entryIndex = entryIndex;
    pSmrEntryInstall->pSmrEntry = PTR_TO_HOST_ADDR(pSmrEntry);

    pSmrEntryInstall->pSmrData = PTR_TO_HOST_ADDR(pData);
    pSmrEntryInstall->smrDataLength = dataLen;
    pSmrEntryInstall->pAuthTag[0] = PTR_TO_HOST_ADDR(pSign0);
    pSmrEntryInstall->pAuthTag[1] = PTR_TO_HOST_ADDR(pSign1);
    pSmrEntryInstall->authTagLength[0] = SignLen0;
    pSmrEntryInstall->authTagLength[1] = SignLen1;

    /* Send the request synchronously */
    srvResponse = HSE_Send(MU0, u8MuChannel, gSyncTxOption, pHseSrvDesc);
    return srvResponse;
}

/*******************************************************************************
 * Function:    smrVerifyTest
 * Description: Verifies the SMR entry used for Advanced Secure Boot.
 ******************************************************************************/
hseSrvResponse_t smrVerifyTest(uint32_t smrentry)
{
    uint8_t u8MuChannel = 1U;
    hseSmrVerifySrv_t *pSmrVerifySrv;
    hseSrvResponse_t hseStatus = HSE_SRV_RSP_GENERAL_ERROR;

    hseSrvDescriptor_t *pHseSrvDesc = &gHseSrvDesc[MU0][u8MuChannel];
    pSmrVerifySrv = &(pHseSrvDesc->hseSrv.smrVerifyReq);

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_SMR_VERIFY;

    pSmrVerifySrv->entryIndex = smrentry;

    hseStatus = HSE_Send(MU0, u8MuChannel, gSyncTxOption, pHseSrvDesc);
    goto exit;
exit:
    return hseStatus;
}

/*******************************************************************************
 * Function:    HSE_InstallCoreResetEntry
 * Description: Installs a core reset entry used for Advanced Secure Boot.
 ******************************************************************************/
hseSrvResponse_t HSE_InstallCoreResetEntry(
    const uint8_t entryIndex,
    const hseCrEntry_t *pCrEntry)
{
    uint8_t u8MuChannel = 1U;
    hseSrvDescriptor_t *pHseSrvDesc;
    hseCrEntryInstallSrv_t *pCrEntryInstallSrv;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /* Clear the service descriptor placed in shared memory */
    pHseSrvDesc = &gHseSrvDesc[MU0][u8MuChannel];
    pCrEntryInstallSrv = &(pHseSrvDesc->hseSrv.crEntryInstallReq);
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    /* Fill the service descriptor */
    pHseSrvDesc->srvId = HSE_SRV_ID_CORE_RESET_ENTRY_INSTALL;
    pCrEntryInstallSrv->crEntryIndex = entryIndex;
    pCrEntryInstallSrv->pCrEntry = PTR_TO_HOST_ADDR(pCrEntry);

    /* Send the request synchronously */
    srvResponse = HSE_Send(MU0, u8MuChannel, gSyncTxOption, pHseSrvDesc);
    return srvResponse;
}

#endif /* defined(HSE_SPT_SMR_CR) */
/** @} */
