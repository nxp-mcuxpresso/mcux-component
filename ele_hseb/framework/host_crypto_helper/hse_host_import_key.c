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

#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
 ==================================================================================================*/

#include "hse_host.h"
#include "hse_host_import_key.h"
#include "hse_keys_allocator.h"
#include "string.h"
#include "hse_default_config.h"
#include "hse_common_test.h"
#include "hse_host_mac.h"
#include "hse_host_cipher.h"

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
static uint8_t          muIf = 0U;
static uint8_t          muChannelIdx = 1U;

#ifdef HSE_SPT_EXPORT_KEY
/* Variables used for symmetric key export (enc&auth -> plain) */
static hseCipherScheme_t exportSymCipherScheme = {
    .symCipher.cipherAlgo = HSE_CIPHER_ALGO_AES,
    .symCipher.cipherBlockMode = HSE_CIPHER_BLOCK_MODE_ECB
};
static hseAuthScheme_t exportSymAuthScheme = {
    .macScheme.macAlgo = HSE_MAC_ALGO_CMAC,
    .macScheme.sch.cmac.cipherAlgo = HSE_CIPHER_ALGO_AES
};


static uint8_t cmacAes256ProvisionKey[] =
{ 0x95, 0xd8, 0xaf, 0xb8, 0xa4, 0xb7, 0x24, 0x5c, 0xe7, 0x9f, 0x9f, 0x9c, 0x5d, 0xdd, 0x40, 0xde, 0x61, 0xb3, 0x59, 0x05, 0xdc, 0xb6, 0x38, 0xf2, 0xb8, 0x75, 0x40, 0x4a, 0x98, 0x5b, 0x3f, 0x7a };

static hseKeyInfo_t provKeyAes256_Info =
{
    .keyType = HSE_KEY_TYPE_AES,
    .keyBitLen = 256UL,
    .keyFlags = (HSE_KF_USAGE_SIGN|HSE_KF_USAGE_ENCRYPT|HSE_KF_USAGE_KEY_PROVISION | HSE_KF_ACCESS_EXPORTABLE)
};
static hseKey_t provKeyAes256 = {
    .keyHandle = HSE_INVALID_KEY_HANDLE,
    .pKeyInfo = &provKeyAes256_Info,
    .keyValue = {
        .pKey2 = cmacAes256ProvisionKey,
        .keyLen2 = ARRAY_SIZE(cmacAes256ProvisionKey),
    },
};

static hseKeyInfo_t copyProvKeyAes256_Info =
{
    .keyType = HSE_KEY_TYPE_AES,
    .keyBitLen = 256UL,
    .keyFlags = (HSE_KF_USAGE_VERIFY|HSE_KF_USAGE_DECRYPT | HSE_KF_ACCESS_EXPORTABLE)
};
static hseKey_t copyProvKeyAes256 = {
    .keyHandle = HSE_INVALID_KEY_HANDLE,
    .pKeyInfo = &copyProvKeyAes256_Info,
    .keyValue = {
        .pKey2 = cmacAes256ProvisionKey,
        .keyLen2 = ARRAY_SIZE(cmacAes256ProvisionKey),
    },
};
#endif
/*==================================================================================================
 *                                      GLOBAL CONSTANTS
 ==================================================================================================*/

/*==================================================================================================
 *                                      GLOBAL VARIABLES
 ==================================================================================================*/

bool_t bSetup = FALSE;
uint8_t keyContainer[1500] = {0};
uint8_t *pKey0 = NULL;
uint8_t *pKey1 = NULL;
uint8_t *pKey2 = NULL;
hseKeyInfo_t *gpKeyInfo = NULL;

hseKeyInfo_t keyInfo = {0U};
uint32_t     gKeySecCount = 0UL;

uint8_t outSign0[512] = {0};
uint8_t outSign1[512] = {0};
uint8_t gExportKeyContainer[MAX_KEY_CONTAINER_LENGTH] = {0};
uint8_t gDecryptedExportKeyContainer[MAX_KEY_CONTAINER_LENGTH] = {0};

/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
 ==================================================================================================*/
#ifdef HSE_SPT_IMPORT_KEY
static void SetupAuthRawKey(hseKeyImportParams_t *pImportKeyParams);
static void CleanUpAuthRawKey(hseKeyImportParams_t *pImportKeyParams);
#endif
#ifdef HSE_SPT_GET_KEY_INFO
static bool_t MustCopyKeyInfoInAuthContainer(hseKeyInfo_t *pKeyInfo, hseKeyHandle_t keyHandle);
#endif
#ifdef HSE_SPT_KEY_GEN
static hseSrvResponse_t GenerateSymKey(uint8_t keyType, uint16_t keyBitLen, uint32_t *pTargetKeyHandle,
                                       uint8_t isNvm, hseKeyFlags_t keyFlags, hseAesBlockModeMask_t cipherBlockMode);
#endif
#if defined(HSE_SPT_CLASSIC_DH) && defined(HSE_SPT_KEY_GEN)
static hseSrvResponse_t GenerateClassicDhKeyReq(uint32_t *pTargetKeyHandle,
                                                uint16_t keyBitLen,
                                                const uint8_t* pPrimeModulus,
                                                uint16_t baseGByteLen,
                                                const uint8_t* pBaseG,
                                                uint8_t* pPubKey);
#endif
/*==================================================================================================
 *                                       LOCAL FUNCTIONS
 ==================================================================================================*/
#ifdef HSE_SPT_IMPORT_KEY
static hseSrvResponse_t SetDefaultProvisionKey(uint8_t muInstance, uint8_t muChannel)
{
    uint16_t hseStatus = 0U;
    hseKeyImportParams_t importKeyParams = {0};
    hseSrvResponse_t hseSrvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /* Set the provision key depending on the authenticated User */
    hseStatus = HSE_MU_GetHseStatus(muInstance);
    if(HSE_STATUS_CUST_SUPER_USER != (hseStatus & HSE_STATUS_CUST_SUPER_USER) &&
       HSE_STATUS_OEM_SUPER_USER != (hseStatus & HSE_STATUS_OEM_SUPER_USER))
    {
        hseSrvResponse = HSE_SRV_RSP_NOT_ALLOWED;
        goto exit;
    }

    if(HSE_INVALID_KEY_HANDLE == provKeyAes256.keyHandle)
    {
        hseSrvResponse = HKF_AllocKeySlot(NVM_KEY, HSE_KEY_TYPE_AES, 256U, &(provKeyAes256.keyHandle));
        if(HSE_SRV_RSP_OK != hseSrvResponse)
        {
            goto exit;
        }
    }

    /* Import Provision key(s) */
    importKeyParams.cipherParams.pKeyHandle = NULL;
    importKeyParams.authParams.pKeyHandle = NULL;
    importKeyParams.pKey = &provKeyAes256;
    hseSrvResponse = ImportKeyReqMuChannel(muInstance, muChannel, &importKeyParams);
    if (HSE_SRV_RSP_OK != hseSrvResponse)
    {
        HKF_FreeKeySlot(&provKeyAes256.keyHandle);
        goto exit;
    }

    /* Import Provision key copy (used for decryption and verification) */
    if(HSE_INVALID_KEY_HANDLE == copyProvKeyAes256.keyHandle)
    {
        hseSrvResponse = HKF_AllocKeySlot(RAM_KEY, HSE_KEY_TYPE_AES, 256U, &copyProvKeyAes256.keyHandle);
        if(HSE_SRV_RSP_OK != hseSrvResponse)
        {
            goto exit;
        }
    }
    importKeyParams.pKey = &copyProvKeyAes256;
    hseSrvResponse = ImportKeyReqMuChannel(muInstance, muChannel, &importKeyParams);
    if (HSE_SRV_RSP_OK != hseSrvResponse)
    {
        HKF_FreeKeySlot(&provKeyAes256.keyHandle);
        HKF_FreeKeySlot(&copyProvKeyAes256.keyHandle);
        goto exit;
    }

exit:
    return hseSrvResponse;
}
#endif
#if defined(HSE_SPT_GET_KEY_INFO)
static bool_t MustCopyKeyInfoInAuthContainer(hseKeyInfo_t *pKeyInfo, hseKeyHandle_t keyHandle)
{
    return (
        (NULL != pKeyInfo) &&
        (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(keyHandle)) &&
        (HSE_KEY_TYPE_ECC_PUB != pKeyInfo->keyType) &&
        (HSE_KEY_TYPE_ECC_PUB_EXT != pKeyInfo->keyType) &&
        (HSE_KEY_TYPE_RSA_PUB != pKeyInfo->keyType) &&
        (HSE_KEY_TYPE_RSA_PUB_EXT != pKeyInfo->keyType)
        );
}
#endif
#if defined(HSE_SPT_IMPORT_KEY)

static void SetupAuthRawKey(hseKeyImportParams_t *pImportKeyParams)
{
    if ((HSE_IMPORT_USE_AUTH_KEY_CONTAINER == (pImportKeyParams->options & HSE_IMPORT_USE_AUTH_KEY_CONTAINER))
        && (NULL == pImportKeyParams->authParams.pKeyContainer))
    {
        uint32_t offset = 0UL;

        bSetup = TRUE;

        /* Key info must be in the container for NVM keys (not PUB types) */
        if (MustCopyKeyInfoInAuthContainer(pImportKeyParams->pKey->pKeyInfo, pImportKeyParams->pKey->keyHandle))
        {
            memcpy(&keyContainer[offset], pImportKeyParams->pKey->pKeyInfo, sizeof(hseKeyInfo_t));
            gpKeyInfo = pImportKeyParams->pKey->pKeyInfo;
            pImportKeyParams->pKey->pKeyInfo = (hseKeyInfo_t *)&keyContainer[offset];
            offset += sizeof(hseKeyInfo_t);
        }

        if (NULL != pImportKeyParams->pKey->keyValue.pKey0)
        {
            memcpy(&keyContainer[offset], pImportKeyParams->pKey->keyValue.pKey0, pImportKeyParams->pKey->keyValue.keyLen0);
            pKey0 = pImportKeyParams->pKey->keyValue.pKey0;
            pImportKeyParams->pKey->keyValue.pKey0 = &keyContainer[offset];
            offset += pImportKeyParams->pKey->keyValue.keyLen0;
        }
        if (NULL != pImportKeyParams->pKey->keyValue.pKey1)
        {
            memcpy(&keyContainer[offset], pImportKeyParams->pKey->keyValue.pKey1, pImportKeyParams->pKey->keyValue.keyLen1);
            pKey1 = pImportKeyParams->pKey->keyValue.pKey1;
            pImportKeyParams->pKey->keyValue.pKey1 = &keyContainer[offset];
            offset += pImportKeyParams->pKey->keyValue.keyLen1;
        }
        if (NULL != pImportKeyParams->pKey->keyValue.pKey2)
        {
            memcpy(&keyContainer[offset], pImportKeyParams->pKey->keyValue.pKey2, pImportKeyParams->pKey->keyValue.keyLen2);
            pKey2 = pImportKeyParams->pKey->keyValue.pKey2;
            pImportKeyParams->pKey->keyValue.pKey2 = &keyContainer[offset];
            offset += pImportKeyParams->pKey->keyValue.keyLen2;
        }
        pImportKeyParams->authParams.pKeyContainer = keyContainer;
        pImportKeyParams->authParams.keyContainerLength = offset;
    }
}

static void CleanUpAuthRawKey(hseKeyImportParams_t *pImportKeyParams)
{
    if (bSetup)
    {
        bSetup = FALSE;
        memset(keyContainer, 0U, 1500UL);
        pImportKeyParams->pKey->keyValue.pKey0 = pKey0;
        pImportKeyParams->pKey->keyValue.pKey1 = pKey1;
        pImportKeyParams->pKey->keyValue.pKey2 = pKey2;
        if (MustCopyKeyInfoInAuthContainer(gpKeyInfo, pImportKeyParams->pKey->keyHandle))
        {
            pImportKeyParams->pKey->pKeyInfo = gpKeyInfo;
        }
        pKey0 = NULL;
        pKey1 = NULL;
        pKey2 = NULL;
        gpKeyInfo = NULL;
        pImportKeyParams->authParams.pKeyContainer = NULL;
    }
}
#endif

#ifdef HSE_SPT_KEY_GEN
static hseSrvResponse_t GenerateSymKey(uint8_t keyType, uint16_t keyBitLen, uint32_t *pTargetKeyHandle,
                                       uint8_t isNvm, hseKeyFlags_t keyFlags, hseAesBlockModeMask_t cipherBlockMode)
{
    hseSrvResponse_t hseSrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseKeyGenerateSrv_t* pKeyGenSrv;
    hseKeyInfo_t lKeyInfo = {0};
    bool_t bEraseKeyBeforeGen = FALSE;

    hseSrvResponse = HKF_AllocKeySlot(isNvm, keyType, keyBitLen, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseSrvResponse)
    {
        goto exit;
    }

    bEraseKeyBeforeGen = ((NVM_KEY == isNvm) && (HSE_SRV_RSP_KEY_EMPTY != GetKeyInfo(*pTargetKeyHandle, &lKeyInfo)));
    if(bEraseKeyBeforeGen)
    {
        /* Erase NVM key before import (ensure the key slot is empty) */
        if(HSE_SRV_RSP_OK != (hseSrvResponse = EraseKeyReq(*pTargetKeyHandle, HSE_ERASE_NOT_USED)))
        {
            goto exit;
        }
    }

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    pHseSrvDesc->srvId = HSE_SRV_ID_KEY_GENERATE;

    pKeyGenSrv = &(pHseSrvDesc->hseSrv.keyGenReq);
    pKeyGenSrv->targetKeyHandle = *pTargetKeyHandle; /*TODO :  Need to verify how can pass key handle*/
    pKeyGenSrv->keyInfo.keyType = keyType;
    pKeyGenSrv->keyInfo.keyBitLen = keyBitLen; /*TODO :  Need to verify for otherkey size and define the design*/
    pKeyGenSrv->keyInfo.keyFlags = keyFlags;
    if(HSE_KEY_TYPE_AES == pKeyGenSrv->keyInfo.keyType)
    {
        pKeyGenSrv->keyInfo.specific.aesBlockModeMask = cipherBlockMode;
    }
    pKeyGenSrv->keyGenScheme = HSE_KEY_GEN_SYM_RANDOM_KEY;

    hseSrvResponse = HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    if(HSE_SRV_RSP_OK != hseSrvResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
    }
exit:
    return hseSrvResponse;
}
#endif

#ifdef HSE_SPT_CLASSIC_DH_KEY_PAIR_GEN
static hseSrvResponse_t GenerateClassicDhKeyReq(uint32_t *pTargetKeyHandle,
                                                uint16_t keyBitLen,
                                                const uint8_t* pPrimeModulus,
                                                uint16_t baseGByteLen,
                                                const uint8_t* pBaseG,
                                                uint8_t* pPubKey)
{
    hseSrvResponse_t hseSrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseKeyGenerateSrv_t* pKeyGenSrv;

    hseSrvResponse = HKF_AllocKeySlot(FALSE, HSE_KEY_TYPE_DH_PAIR, keyBitLen, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseSrvResponse)
    {
        goto exit;
    }


    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    pHseSrvDesc->srvId = HSE_SRV_ID_KEY_GENERATE;

    pKeyGenSrv = &(pHseSrvDesc->hseSrv.keyGenReq);
    pKeyGenSrv->targetKeyHandle = *pTargetKeyHandle;
    pKeyGenSrv->keyInfo.keyType = HSE_KEY_TYPE_DH_PAIR;
    pKeyGenSrv->keyInfo.keyBitLen = keyBitLen;
    pKeyGenSrv->keyInfo.keyFlags = HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE;
    pKeyGenSrv->keyGenScheme = HSE_KEY_GEN_CLASSIC_DH_KEY_PAIR;
    pKeyGenSrv->sch.classicDhKey.modulusLength = BITS_TO_BYTES(keyBitLen);
    pKeyGenSrv->sch.classicDhKey.pModulus = (HOST_ADDR)pPrimeModulus;
    pKeyGenSrv->sch.classicDhKey.baseGLength = baseGByteLen;
    pKeyGenSrv->sch.classicDhKey.pBaseG = (HOST_ADDR)pBaseG;
    pKeyGenSrv->sch.classicDhKey.pPubKey = (HOST_ADDR)pPubKey;

    hseSrvResponse = HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    if(HSE_SRV_RSP_OK != hseSrvResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
    }
exit:
    return hseSrvResponse;
}
#endif
/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
 ==================================================================================================*/

/*****************************************************************************************
 *                                      Generic
 ****************************************************************************************/

hseSrvResponse_t ImportKeyReq(hseKeyImportParams_t *pImportKeyParams)
{
    return ImportKeyReqMuChannel(muIf, muChannelIdx, pImportKeyParams);
}

hseSrvResponse_t ImportKeyReqMuChannel(uint8_t muInstance, uint8_t muChannel, hseKeyImportParams_t *pImportKeyParams)
{
    hseKeyInfo_t lKeyInfo = {0};
    bool_t bEraseKeyBeforeImport = ((HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(pImportKeyParams->pKey->keyHandle)) &&
        (HSE_SRV_RSP_KEY_EMPTY != GetKeyInfo(pImportKeyParams->pKey->keyHandle, &lKeyInfo)));
    return ImportKeyReqMuChannelWithoutErase(muInstance, muChannel, bEraseKeyBeforeImport, pImportKeyParams);
}

hseSrvResponse_t ImportKeyReqMuChannelWithoutErase(uint8_t muInstance, uint8_t muChannel, bool_t bEraseKeyBeforeImport, hseKeyImportParams_t *pImportKeyParams)
{
    #if defined(HSE_SPT_IMPORT_KEY)
    hseSrvResponse_t hseSrvResponse;
    hseSrvDescriptor_t *pHseSrvDesc = &gHseSrvDesc[muInstance][muChannel];
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    if(bEraseKeyBeforeImport)
    {
        /* Erase NVM key before import (ensure the key slot is empty) */
        if(HSE_SRV_RSP_OK != (hseSrvResponse = EraseKeyReq(pImportKeyParams->pKey->keyHandle, HSE_ERASE_NOT_USED)))
        {
            goto exit;
        }
    }

    /* Setup for Raw keys - Authenticated keys can be imported only from a key container */
    SetupAuthRawKey(pImportKeyParams);

    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;

    /* Increase the keyCounter only key is imported without authentication */
    pImportKeyParams->pKey->pKeyInfo->keyCounter = (TRUE == MustCopyKeyInfoInAuthContainer(pImportKeyParams->pKey->pKeyInfo, pImportKeyParams->pKey->keyHandle)) ? gKeySecCount :
        (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(pImportKeyParams->pKey->keyHandle)) ? (++gKeySecCount) : 0UL;

    pImportKeyReq->targetKeyHandle = pImportKeyParams->pKey->keyHandle;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)pImportKeyParams->pKey->pKeyInfo;

    pImportKeyReq->pKey[0] = (HOST_ADDR)pImportKeyParams->pKey->keyValue.pKey0;
    pImportKeyReq->keyLen[0] = pImportKeyParams->pKey->keyValue.keyLen0;
    pImportKeyReq->pKey[1] = (HOST_ADDR)pImportKeyParams->pKey->keyValue.pKey1;
    pImportKeyReq->keyLen[1] = pImportKeyParams->pKey->keyValue.keyLen1;
    pImportKeyReq->pKey[2] = (HOST_ADDR)pImportKeyParams->pKey->keyValue.pKey2;
    pImportKeyReq->keyLen[2] = pImportKeyParams->pKey->keyValue.keyLen2;

    pImportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    if(NULL != pImportKeyParams->cipherParams.pKeyHandle && HSE_INVALID_KEY_HANDLE != *pImportKeyParams->cipherParams.pKeyHandle)
    {
        pImportKeyReq->cipher.cipherKeyHandle = *pImportKeyParams->cipherParams.pKeyHandle;
        memcpy(&pImportKeyReq->cipher.cipherScheme, pImportKeyParams->cipherParams.pCipherScheme, sizeof(hseCipherScheme_t));
    }

    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;
    if(NULL != pImportKeyParams->authParams.pKeyHandle && HSE_INVALID_KEY_HANDLE != *pImportKeyParams->authParams.pKeyHandle)
    {
        pImportKeyReq->keyContainer.authKeyHandle = *pImportKeyParams->authParams.pKeyHandle;
        memcpy(&pImportKeyReq->keyContainer.authScheme, pImportKeyParams->authParams.pAuthScheme, sizeof(hseAuthScheme_t));
        pImportKeyReq->keyContainer.pAuth[0] = (HOST_ADDR)pImportKeyParams->authParams.pSign0;
        pImportKeyReq->keyContainer.authLen[0] = pImportKeyParams->authParams.signLen0;
        pImportKeyReq->keyContainer.pAuth[1] = (HOST_ADDR)pImportKeyParams->authParams.pSign1;
        pImportKeyReq->keyContainer.authLen[1] = pImportKeyParams->authParams.signLen1;

    }
    pImportKeyReq->keyContainer.pKeyContainer = (HOST_ADDR)pImportKeyParams->authParams.pKeyContainer;
    pImportKeyReq->keyContainer.keyContainerLen = (uint16_t)pImportKeyParams->authParams.keyContainerLength;

    hseSrvResponse = HSE_Send(muInstance, muChannel, gSyncTxOption, pHseSrvDesc);

    /* CleanUp for Raw keys - Erase the set container */
    CleanUpAuthRawKey(pImportKeyParams);
exit:
    return hseSrvResponse;
#else
    (void)muInstance;
    (void)muChannel;
    (void)bEraseKeyBeforeImport;
    (void)pImportKeyParams;
    return HSE_SRV_RSP_NOT_SUPPORTED;
#endif
}

hseSrvResponse_t ExportKeyReq(hseKeyExportParams_t *pExportKeyParams)
{
    return ExportKeyReqMuChannel(muIf, muChannelIdx, pExportKeyParams);
}

hseSrvResponse_t ExportKeyReqMuChannel(uint8_t muInstance, uint8_t muChannel, hseKeyExportParams_t *pExportKeyParams)
{
    #if defined(HSE_SPT_EXPORT_KEY)
    hseSrvDescriptor_t hseSrvDesc = {0};

    hseExportKeySrv_t *pExportKeyReq = &hseSrvDesc.hseSrv.exportKeyReq;
    hseSrvResponse_t hseSrvResponse;
    bool_t bIsSymNotEncrypted = FALSE, bIsSymNotAuthenticated = FALSE;
    hseKeyInfo_t lKeyInfo = {0};
    uint16_t outSignLen0 = 512UL, outSignLen1 = 512UL;

    /* Check if key is Empty */
    hseSrvResponse = GetKeyInfoMuChannel(muInstance, muChannel, pExportKeyParams->pKey->keyHandle, &lKeyInfo);
    if(HSE_SRV_RSP_OK != hseSrvResponse)
    {
        goto exit;
    }

    hseSrvDesc.srvId = HSE_SRV_ID_EXPORT_KEY;

    pExportKeyReq->targetKeyHandle = pExportKeyParams->pKey->keyHandle;
    pExportKeyReq->pKeyInfo = (HOST_ADDR)pExportKeyParams->pKey->pKeyInfo;

    pExportKeyReq->pKey[0] = (HOST_ADDR)pExportKeyParams->pKey->keyValue.pKey0;
    pExportKeyReq->pKeyLen[0] = (HOST_ADDR)&pExportKeyParams->pKey->keyValue.keyLen0;
    pExportKeyReq->pKey[1] = (HOST_ADDR)pExportKeyParams->pKey->keyValue.pKey1;
    pExportKeyReq->pKeyLen[1] = (HOST_ADDR)&pExportKeyParams->pKey->keyValue.keyLen1;
    pExportKeyReq->pKey[2] = (HOST_ADDR)pExportKeyParams->pKey->keyValue.pKey2;
    pExportKeyReq->pKeyLen[2] = (HOST_ADDR)&pExportKeyParams->pKey->keyValue.keyLen2;

    /* Complete cipher parameters */
    pExportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    if(NULL != pExportKeyParams->cipherParams.pKeyHandle && HSE_INVALID_KEY_HANDLE != *pExportKeyParams->cipherParams.pKeyHandle)
    {
        pExportKeyReq->cipher.cipherKeyHandle = *pExportKeyParams->cipherParams.pKeyHandle;
        if( (pExportKeyParams->cipherParams.pCipherScheme != NULL)
        &&((HSE_AUTH_CIPHER_MODE_CCM == pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.authCipherMode)
        ||(HSE_AUTH_CIPHER_MODE_GCM == pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.authCipherMode)))
        {
            pExportKeyReq->pKeyInfo = (HOST_ADDR) &((uint8_t *)((uint32_t)pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.pAAD))[pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.aadLength];
            pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.aadLength += sizeof(hseKeyInfo_t);
        }

        memcpy(&pExportKeyReq->cipher.cipherScheme, pExportKeyParams->cipherParams.pCipherScheme, sizeof(hseCipherScheme_t));
    }
    else
    {
        /* Symmetric keys MUST be exported encrypted */
        if( (HSE_KEY_TYPE_AES == lKeyInfo.keyType) ||
            (HSE_KEY_TYPE_HMAC == lKeyInfo.keyType) ||
            (HSE_KEY_TYPE_SHARED_SECRET == lKeyInfo.keyType) )
        {
            /* Add default cipher scheme */
            bIsSymNotEncrypted = TRUE;
            /* Set the provision key depending on the authenticated User */
            hseSrvResponse = SetDefaultProvisionKey(muInstance, muChannel);
            if(HSE_SRV_RSP_OK != hseSrvResponse)
            {
                goto exit;
            }
            /* Complete cipher parameters */
            pExportKeyReq->cipher.cipherKeyHandle = provKeyAes256.keyHandle;
            pExportKeyReq->cipher.cipherScheme = exportSymCipherScheme;
            /* verify the key length is aligned, not the container */
            if((pExportKeyParams->authParams.keyContainerLength - sizeof(hseKeyInfo_t)) % 16UL)
            {
                /* align the key to 16 byte */
                pExportKeyParams->authParams.keyContainerLength = 16UL * (((pExportKeyParams->authParams.keyContainerLength- sizeof(hseKeyInfo_t)) / 16UL) + 1UL) ;
                /* update container length */
                pExportKeyParams->authParams.keyContainerLength += sizeof(hseKeyInfo_t);
            }
        }
    }

    /* Complete authentication parameters */
    pExportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;
    if(NULL != pExportKeyParams->authParams.pKeyHandle && HSE_INVALID_KEY_HANDLE != *pExportKeyParams->authParams.pKeyHandle)
    {
        pExportKeyReq->keyContainer.authKeyHandle = *pExportKeyParams->authParams.pKeyHandle;
        pExportKeyReq->keyContainer.pKeyContainer = (HOST_ADDR)pExportKeyParams->authParams.pKeyContainer;
        pExportKeyReq->keyContainer.keyContainerLen = pExportKeyParams->authParams.keyContainerLength;
        memcpy(&pExportKeyReq->keyContainer.authScheme, pExportKeyParams->authParams.pAuthScheme, sizeof(hseAuthScheme_t));
        pExportKeyReq->keyContainer.pAuth[0] = (HOST_ADDR)pExportKeyParams->authParams.pSign0;
        pExportKeyReq->keyContainer.pAuthLen[0] = (HOST_ADDR)&pExportKeyParams->authParams.signLen0;
        pExportKeyReq->keyContainer.pAuth[1] = (HOST_ADDR)pExportKeyParams->authParams.pSign1;
        pExportKeyReq->keyContainer.pAuthLen[1] = (HOST_ADDR)&pExportKeyParams->authParams.signLen1;
    }
    else
    {
        /* Symmetric keys MUST be exported authenticated */
        if( ((HSE_KEY_TYPE_AES == lKeyInfo.keyType) ||
            (HSE_KEY_TYPE_HMAC == lKeyInfo.keyType) ||
            (HSE_KEY_TYPE_SHARED_SECRET == lKeyInfo.keyType)) )
            {
                if(
                (pExportKeyParams->cipherParams.pCipherScheme != NULL)
                &&((HSE_AUTH_CIPHER_MODE_CCM == pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.authCipherMode)
                ||(HSE_AUTH_CIPHER_MODE_GCM == pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.authCipherMode))
                )
                {

                }
                else
                {
                    /* Add default authentication scheme */
                    bIsSymNotAuthenticated = TRUE;
                    if(!bIsSymNotEncrypted)
                    {
                        /* Set the provision key depending on the authenticated User */
                        hseSrvResponse = SetDefaultProvisionKey(muInstance, muChannel);
                        if(HSE_SRV_RSP_OK != hseSrvResponse)
                        {
                            goto exit;
                        }
                    }
                    /* Complete authentication parameters */
                    pExportKeyReq->pKeyInfo = (HOST_ADDR)&gExportKeyContainer[0];
                    pExportKeyReq->pKey[2] = (HOST_ADDR)&gExportKeyContainer[sizeof(hseKeyInfo_t)];
                    // pExportKeyReq->pKeyInfo = (HOST_ADDR)&gExportKeyContainer[BITS_TO_BYTES(lKeyInfo.keyBitLen)];
                    pExportKeyReq->keyContainer.pKeyContainer = (HOST_ADDR)&gExportKeyContainer[0];
                    pExportKeyReq->keyContainer.keyContainerLen = MAX_KEY_CONTAINER_LENGTH;
                    pExportKeyReq->keyContainer.authKeyHandle = provKeyAes256.keyHandle;
                    pExportKeyReq->keyContainer.authScheme = exportSymAuthScheme;
                    pExportKeyReq->keyContainer.pAuth[0] = (HOST_ADDR)outSign0;
                    pExportKeyReq->keyContainer.pAuthLen[0] = (HOST_ADDR)&outSignLen0;
                    pExportKeyReq->keyContainer.pAuth[1] = (HOST_ADDR)outSign1;
                    pExportKeyReq->keyContainer.pAuthLen[1] = (HOST_ADDR)&outSignLen1;
                }
            }
       }


    memcpy(&gHseSrvDesc[muInstance][muChannel], &hseSrvDesc, sizeof(hseSrvDescriptor_t));

    hseSrvResponse = HSE_Send(muInstance, muChannel, gSyncTxOption, &gHseSrvDesc[muInstance][muChannel]);
    if(HSE_SRV_RSP_OK != hseSrvResponse)
    {
        goto exit;
    }

    if(bIsSymNotAuthenticated)
    {
        /* Verify signature generated with default auth scheme */
        uint32_t macLength = outSignLen0;
        hseSrvResponse = AesCmacVerify(copyProvKeyAes256.keyHandle,
                                        MAX_KEY_CONTAINER_LENGTH, gExportKeyContainer,
                                        &macLength, outSign0, 0U);
        if(hseSrvResponse != HSE_SRV_RSP_OK)
        {
            goto exit;
        }
        /* Copy the symmetric key to original buffer */
        if(!bIsSymNotEncrypted)
        {
            (void)memcpy(pExportKeyParams->pKey->pKeyInfo, &gExportKeyContainer[0], sizeof(hseKeyInfo_t));
            (void)memcpy(pExportKeyParams->pKey->keyValue.pKey2, &gExportKeyContainer[sizeof(hseKeyInfo_t)], pExportKeyParams->pKey->keyValue.keyLen2);
        }
    }

    if(bIsSymNotEncrypted)
    {
        /* Decrypt payload with default cipher scheme */
        hseSrvResponse = AesDecrypt(copyProvKeyAes256.keyHandle, HSE_CIPHER_BLOCK_MODE_ECB,
                                    NULL, pExportKeyReq->keyContainer.keyContainerLen - sizeof(hseKeyInfo_t),
                                    &((uint8_t *)((uint32_t)pExportKeyReq->keyContainer.pKeyContainer))[sizeof(hseKeyInfo_t)],
                                    gDecryptedExportKeyContainer, 0U);
        if(hseSrvResponse != HSE_SRV_RSP_OK)
        {
            goto exit;
        }

        pExportKeyParams->pKey->keyValue.keyLen2 = BITS_TO_BYTES(((hseKeyInfo_t *)(uint32_t)pExportKeyReq->pKeyInfo)->keyBitLen);
        (void)memcpy(pExportKeyParams->pKey->keyValue.pKey2, gDecryptedExportKeyContainer, pExportKeyParams->pKey->keyValue.keyLen2);
        (void)memcpy(pExportKeyParams->pKey->pKeyInfo, (uint8_t *)(uint32_t)pExportKeyReq->pKeyInfo, sizeof(hseKeyInfo_t));
    }
    else
    {
        if( (pExportKeyParams->cipherParams.pCipherScheme != NULL)
        &&((HSE_AUTH_CIPHER_MODE_CCM == pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.authCipherMode)
        ||(HSE_AUTH_CIPHER_MODE_GCM == pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.authCipherMode)))
        {
            (void)memcpy(pExportKeyParams->pKey->pKeyInfo, (uint8_t *)(uint32_t)pExportKeyReq->pKeyInfo, sizeof(hseKeyInfo_t));
            pExportKeyParams->cipherParams.pCipherScheme->aeadCipher.aadLength -= sizeof(hseKeyInfo_t);
        }
    }


exit:
    if((TRUE == bIsSymNotAuthenticated) || (TRUE == bIsSymNotEncrypted))
    {
        /* Clean up - erase provision & copy keys */
        (void)EraseKeyReq(provKeyAes256.keyHandle, HSE_ERASE_NOT_USED);
        HKF_FreeKeySlot(&provKeyAes256.keyHandle);
        (void)EraseKeyReq(copyProvKeyAes256.keyHandle, HSE_ERASE_NOT_USED);
        HKF_FreeKeySlot(&copyProvKeyAes256.keyHandle);
    }
    return hseSrvResponse;
#else
    (void)muInstance;
    (void)muChannel;
    (void)pExportKeyParams;
    return HSE_SRV_RSP_NOT_SUPPORTED;
#endif
}

/*****************************************************************************************
 *                                      Plain
 ****************************************************************************************/

hseSrvResponse_t ImportPlainSymKeyReq(hseKeyHandle_t targetKeyHandle, hseKeyType_t keyType,
                                      hseKeyFlags_t keyFlags, uint16_t keyByteLength,
                                      const uint8_t* pKey, hseAesBlockModeMask_t cipherModeMask)
{
    hseKeyInfo_t lKeyInfo = {0};
    bool_t bEraseKeyBeforeImport = ((HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) &&
        (HSE_SRV_RSP_KEY_EMPTY != GetKeyInfo(targetKeyHandle, &lKeyInfo)));
    return ImportPlainSymKeyReqMuChannel(muIf, muChannelIdx, targetKeyHandle, keyType, keyFlags, 0UL, keyByteLength, pKey, bEraseKeyBeforeImport, cipherModeMask);
}

hseSrvResponse_t ImportPlainSymKeyReqMuChannel(uint8_t u8MuInstance, uint8_t u8MuChannel, hseKeyHandle_t targetKeyHandle,
                                      hseKeyType_t keyType, hseKeyFlags_t keyFlags, hseSmrFlags_t smrFlags,
                                      uint16_t keyByteLength, const uint8_t* pKey, bool_t bEraseKeyBeforeImport,
                                      hseAesBlockModeMask_t cipherModeMask)
{
     #if defined(HSE_SPT_IMPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[u8MuInstance][u8MuChannel];
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    hseSrvResponse_t hseResponse;

    if(bEraseKeyBeforeImport)
    {
        /* Erase NVM key before import (ensure the key slot is empty) */
        if(HSE_SRV_RSP_OK != (hseResponse = EraseKeyReq(targetKeyHandle, HSE_ERASE_NOT_USED)))
        {
            goto exit;
        }
    }

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    memset(&keyInfo, 0, sizeof(hseKeyInfo_t));
    keyInfo.keyType = keyType;
    keyInfo.keyFlags = keyFlags;
    keyInfo.smrFlags = smrFlags;
    keyInfo.keyBitLen = (keyByteLength * 8);
    keyInfo.keyCounter  = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;
    if(HSE_KEY_TYPE_AES == keyType)
    {
        keyInfo.specific.aesBlockModeMask = cipherModeMask;
    }
    
    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;

    pImportKeyReq->targetKeyHandle = targetKeyHandle;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[2] = (HOST_ADDR)pKey;
    pImportKeyReq->keyLen[2] = keyByteLength;

    pImportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;

    hseResponse = HSE_Send(u8MuInstance, u8MuChannel, gSyncTxOption, pHseSrvDesc);
exit:
    return hseResponse;
#else
    (void)u8MuInstance;
    (void)u8MuChannel;
    (void)targetKeyHandle;
    (void)keyType;
    (void)keyFlags;
    (void)smrFlags;
    (void)keyByteLength;
    (void)pKey;
    (void)bEraseKeyBeforeImport;
    (void)cipherModeMask;
    return HSE_SRV_RSP_NOT_SUPPORTED;
#endif
}

hseSrvResponse_t ImportPlainRsaKeyReq(hseKeyHandle_t targetKeyHandle, hseKeyType_t keyType,
                                      hseKeyFlags_t keyFlags, HOST_ADDR pModulus,
                                      uint16_t keyBitLen, HOST_ADDR pPublicExponent,
                                      uint16_t publicExponentLength, HOST_ADDR pPrivateExponent)
{
    #if defined(HSE_SPT_IMPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    hseSrvResponse_t hseResponse;

    if(HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle))
    {
        /* Erase NVM key before import (ensure the key slot is empty) */
        if(HSE_SRV_RSP_OK != (hseResponse = EraseKeyReq(targetKeyHandle, HSE_ERASE_NOT_USED)))
        {
            goto exit;
        }
    }

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    memset(&keyInfo, 0, sizeof(hseKeyInfo_t));
    keyInfo.keyType = keyType;
    keyInfo.keyFlags = keyFlags;
    keyInfo.keyBitLen = keyBitLen;
    keyInfo.keyCounter  = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;

    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;
    pImportKeyReq->targetKeyHandle = targetKeyHandle;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[0] = pModulus;
    pImportKeyReq->keyLen[0] = BITS_TO_BYTES(keyBitLen);
    pImportKeyReq->pKey[1] = pPublicExponent;
    pImportKeyReq->keyLen[1] = publicExponentLength;
    pImportKeyReq->pKey[2] = pPrivateExponent;
    pImportKeyReq->keyLen[2] = BITS_TO_BYTES(keyBitLen);

    pImportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;

    hseResponse = HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
exit:
    return hseResponse;
#else
    (void)keyBitLen;
    (void)pPublicExponent;
    (void)targetKeyHandle;
    (void)keyType;
    (void)keyFlags;
    (void)publicExponentLength;
    (void)pPrivateExponent;
    (void)pModulus;
    return HSE_SRV_RSP_NOT_SUPPORTED;

#endif
}

hseSrvResponse_t ImportCertRsaKeyReq(hseKeyHandle_t targetKeyHandle, hseKeyInfo_t* pKeyInfo,
                                     uint16_t modulusLength, const uint8_t *pModulus,
                                     uint16_t publicExpLength, const uint8_t *pPublicExponent,
                                     uint16_t containerLength, const uint8_t *pContainer)
{
    #if defined(HSE_SPT_IMPORT_KEY)
    hseKeyInfo_t lKeyInfo = {0};
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseSrvResponse_t hseResponse;
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    bool_t bEraseKeyBeforeImport = ((HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) &&
        (HSE_SRV_RSP_KEY_EMPTY != GetKeyInfo(targetKeyHandle, &lKeyInfo)));

    if(bEraseKeyBeforeImport)
    {
        /* Erase NVM key before import (ensure the key slot is empty) */
        if(HSE_SRV_RSP_OK != (hseResponse = EraseKeyReq(targetKeyHandle, HSE_ERASE_NOT_USED)))
        {
            goto exit;
        }
    }

    /* Import the key */
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;
    pImportKeyReq->targetKeyHandle = targetKeyHandle;
    pKeyInfo->keyCounter  = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)pKeyInfo;
    pImportKeyReq->pKey[0] = (HOST_ADDR)pModulus;
    pImportKeyReq->keyLen[0] = modulusLength;
    pImportKeyReq->pKey[1] = (HOST_ADDR)pPublicExponent;
    pImportKeyReq->keyLen[1] = publicExpLength;

    pImportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE; //Signature not verified

    pImportKeyReq->keyContainer.pKeyContainer = (HOST_ADDR)pContainer;
    pImportKeyReq->keyContainer.keyContainerLen = containerLength;
    hseResponse = HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);

exit:
    return hseResponse;
    #else
    (void)modulusLength;
    (void)pPublicExponent;
    (void)targetKeyHandle;
    (void)pModulus;
    (void)publicExpLength;
    (void)pPublicExponent;
    (void)containerLength;
    (void)pContainer;
    (void)pKeyInfo;
    return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}

hseSrvResponse_t ExportPlainRsaPubKeyReq(hseKeyHandle_t targetKeyHandle, hseKeyInfo_t* pKeyInfo,
                                         uint16_t* pModulusLength, const uint8_t* pModulus,
                                         uint16_t* pPublicExponentLength, const uint8_t* pPublicExponent,
                                         uint16_t* pPrivateExponentLength, const uint8_t* pPrivateExponent)

{
    #if defined(HSE_SPT_EXPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];

    hseExportKeySrv_t*  pExportKeyReq = &pHseSrvDesc->hseSrv.exportKeyReq;
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_EXPORT_KEY;
    pExportKeyReq->targetKeyHandle = targetKeyHandle;
    pExportKeyReq->pKeyInfo = (HOST_ADDR)pKeyInfo;
    pExportKeyReq->pKey[0] = (HOST_ADDR)pModulus;
    pExportKeyReq->pKeyLen[0] = (HOST_ADDR)pModulusLength;
    pExportKeyReq->pKey[1] = (HOST_ADDR)pPublicExponent;
    pExportKeyReq->pKeyLen[1] = (HOST_ADDR)pPublicExponentLength;
    pExportKeyReq->pKey[2] = (HOST_ADDR)pPrivateExponent;
    pExportKeyReq->pKeyLen[2] = (HOST_ADDR)pPrivateExponentLength;

    pExportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    pExportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)pKeyInfo;
    (void)pPublicExponent;
    (void)targetKeyHandle;
    (void)pModulus;
    (void)pModulusLength;
    (void)pPublicExponent;
    (void)pPublicExponentLength;
    (void)pPrivateExponentLength;
    (void)pPrivateExponent;
    (void)pKeyInfo;
    return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}

hseSrvResponse_t ExportPlainClassicDhPubKeyReq(hseKeyHandle_t targetKeyHandle,
                                               hseKeyInfo_t* pKeyInfo,
                                               uint16_t* pPrimeModulusLength,
                                               uint8_t* pPrimeModulus,
                                               uint16_t* pPubKeyLength,
                                               uint8_t* pPubKey)
{
    #if defined(HSE_SPT_EXPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];

    hseExportKeySrv_t*  pExportKeyReq = &pHseSrvDesc->hseSrv.exportKeyReq;
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_EXPORT_KEY;
    pExportKeyReq->targetKeyHandle = targetKeyHandle;
    pExportKeyReq->pKeyInfo = (HOST_ADDR)pKeyInfo;
    pExportKeyReq->pKey[0] = (HOST_ADDR)pPrimeModulus;
    pExportKeyReq->pKeyLen[0] = (HOST_ADDR)pPrimeModulusLength;
    pExportKeyReq->pKey[1] = (HOST_ADDR)pPubKey;
    pExportKeyReq->pKeyLen[1] = (HOST_ADDR)pPubKeyLength;

    pExportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    pExportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)targetKeyHandle;
    (void)pKeyInfo;
    (void)pPrimeModulusLength;
    (void)pPrimeModulus;
    (void)pPubKeyLength;
    (void)pPubKey;
    return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}

hseSrvResponse_t ExportPlainFormattedEccPubKeyReq(hseKeyHandle_t targetKeyHandle, hseKeyInfo_t* pKeyInfo,
                                         uint16_t* pPubKeyLength, uint8_t* pPubKey, hseEccKeyFormat_t eccKeyFormat)

{
    #if defined(HSE_SPT_EXPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];

    hseExportKeySrv_t*  pExportKeyReq = &pHseSrvDesc->hseSrv.exportKeyReq;
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_EXPORT_KEY;
    pExportKeyReq->targetKeyHandle = targetKeyHandle;
    pExportKeyReq->pKeyInfo = (HOST_ADDR)pKeyInfo;
    pExportKeyReq->pKey[0] = (HOST_ADDR)pPubKey;
    pExportKeyReq->pKeyLen[0] = (HOST_ADDR)pPubKeyLength;

    pExportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    pExportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;
    pExportKeyReq->keyFormat.eccKeyFormat = eccKeyFormat;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)targetKeyHandle;
    (void)pKeyInfo;
    (void)pPubKeyLength;
    (void)pPubKey;
    return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}

hseSrvResponse_t ExportPlainSymKeyReq(hseKeyHandle_t targetKeyHandle, hseKeyInfo_t *pKeyInfo,
                                      uint8_t *pOutBuffer, uint16_t *pOutBufferLength)
{
    return ExportPlainSymKeyReqMuChannel(muIf, muChannelIdx, targetKeyHandle, pKeyInfo, pOutBuffer, pOutBufferLength);
}

hseSrvResponse_t ExportPlainSymKeyReqMuChannel(uint8_t muInstance, uint8_t muChannel,
    hseKeyHandle_t targetKeyHandle, hseKeyInfo_t *pKeyInfo, uint8_t *pOutBuffer, uint16_t *pOutBufferLength)
{
    return ExportSymKeyReqMuChannel(muInstance, muChannel, targetKeyHandle, pKeyInfo, pOutBuffer, pOutBufferLength,
        HSE_INVALID_KEY_HANDLE, NULL);
}

hseSrvResponse_t ExportSymKeyReqMuChannel(uint8_t muInstance, uint8_t muChannel,
    hseKeyHandle_t targetKeyHandle, hseKeyInfo_t *pKeyInfo, uint8_t *pOutBuffer, uint16_t *pOutBufferLength,
    hseKeyHandle_t cipherKeyHandle, hseCipherScheme_t *pCipherScheme)
{
    hseKey_t key = {0};
    hseSrvResponse_t hseSrvResponse;
    hseKeyExportParams_t exportParams = {0};

    key.pKeyInfo = pKeyInfo;
    key.keyHandle = targetKeyHandle;
    key.keyValue.pKey2 = pOutBuffer;
    key.keyValue.keyLen2 = *pOutBufferLength;
    exportParams.pKey = &key;

    exportParams.cipherParams.pKeyHandle = &cipherKeyHandle;
    exportParams.cipherParams.pCipherScheme = pCipherScheme;

    exportParams.authParams.pKeyHandle = NULL;

    hseSrvResponse = ExportKeyReqMuChannel(muInstance, muChannel, &exportParams);

    *pOutBufferLength = key.keyValue.keyLen2;

    return hseSrvResponse;
}

/*****************************************************************************************
 *                                      Encrypted
 ****************************************************************************************/
hseSrvResponse_t ImportEncryptedSymKeyReq(hseKeyHandle_t cipherKeyHandle, hseCipherScheme_t cipherScheme,
                                          hseKeyHandle_t targetKeyHandle, hseKeyType_t keyType,
                                          hseKeyFlags_t keyFlags, uint16_t keyByteLength,
                                          uint8_t *pKey, uint16_t encryptedKeyLength)
{
    #if defined(HSE_SPT_IMPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    memset(&keyInfo, 0, sizeof(hseKeyInfo_t));
    keyInfo.keyType = keyType;
    keyInfo.keyFlags = keyFlags;
    keyInfo.keyBitLen = (keyByteLength * 8);
    keyInfo.keyCounter  = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;

    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;
    pImportKeyReq->cipher.cipherKeyHandle = cipherKeyHandle;
    pImportKeyReq->cipher.cipherScheme = cipherScheme;
    pImportKeyReq->targetKeyHandle = targetKeyHandle;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[2] = (HOST_ADDR)pKey;
    pImportKeyReq->keyLen[2] = encryptedKeyLength;

    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)cipherKeyHandle;
    (void)cipherScheme;
    (void)targetKeyHandle;
    (void)keyType;
    (void)keyFlags;
    (void)keyByteLength;
    (void)pKey;
    (void)encryptedKeyLength;
    return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}

hseSrvResponse_t ImportEncryptedRsaKeyReq(hseKeyHandle_t cipherKeyHandle, hseCipherScheme_t cipherScheme,
                                          hseKeyHandle_t targetKeyHandle, hseKeyType_t keyType,
                                          hseKeyFlags_t keyFlags, HOST_ADDR pModulus,
                                          uint16_t modulusLength, HOST_ADDR pPublicExponent,
                                          uint16_t publicExponentLength, HOST_ADDR pPrivateExponent,
                                          uint16_t privateExponentLength)
{
    #if defined(HSE_SPT_IMPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    memset(&keyInfo, 0, sizeof(hseKeyInfo_t));
    keyInfo.keyType = keyType;
    keyInfo.keyFlags = keyFlags;
    keyInfo.keyBitLen = (modulusLength * 8);
    keyInfo.keyCounter  = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;

    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;

    pImportKeyReq->cipher.cipherKeyHandle = cipherKeyHandle;
    pImportKeyReq->cipher.cipherScheme = cipherScheme;
    pImportKeyReq->targetKeyHandle = targetKeyHandle;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[0] = pModulus;
    pImportKeyReq->keyLen[0] = modulusLength;
    pImportKeyReq->pKey[1] = pPublicExponent;
    pImportKeyReq->keyLen[1] = publicExponentLength;
    pImportKeyReq->pKey[2] = pPrivateExponent;
    pImportKeyReq->keyLen[2] = privateExponentLength;

    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)cipherKeyHandle;
    (void)cipherScheme;
    (void)targetKeyHandle;
    (void)keyType;
    (void)keyFlags;
    (void)pModulus;
    (void)modulusLength;
    (void)pPublicExponent;
    (void)publicExponentLength;
    (void)pPrivateExponent;
    (void)privateExponentLength;
    return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}

hseSrvResponse_t ImportEncryptedEccKeyReq(hseKeyHandle_t cipherKeyHandle, hseCipherScheme_t cipherScheme,
                                          hseKeyHandle_t targetKeyHandle, hseKeyType_t keyType,
                                          hseKeyFlags_t keyFlags, hseEccCurveId_t eccCurveId,
                                          uint16_t keyBitLength, const uint8_t* pPubKey,
                                          const uint8_t* pPrivKey, uint16_t encryptedPrivKeyLength)
{
    #if defined(HSE_SPT_IMPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    memset(&keyInfo, 0, sizeof(hseKeyInfo_t));
    keyInfo.keyType = keyType;
    keyInfo.keyFlags = keyFlags;
    keyInfo.keyBitLen = keyBitLength;
    keyInfo.specific.eccCurveId = eccCurveId;
    keyInfo.keyCounter  = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;

    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;

    pImportKeyReq->cipher.cipherKeyHandle = cipherKeyHandle;
    pImportKeyReq->cipher.cipherScheme = cipherScheme;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[0] = (HOST_ADDR)pPubKey;
    pImportKeyReq->keyLen[0] = 2 * BITS_TO_BYTES(keyBitLength);
    pImportKeyReq->pKey[2] = (HOST_ADDR)pPrivKey;
    pImportKeyReq->keyLen[2] = encryptedPrivKeyLength;
    pImportKeyReq->targetKeyHandle = targetKeyHandle;

    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)cipherKeyHandle;
    (void)cipherScheme;
    (void)targetKeyHandle;
    (void)keyType;
    (void)keyFlags;
    (void)eccCurveId;
    (void)keyBitLength;
    (void)pPubKey;
    (void)pPrivKey;
    (void)encryptedPrivKeyLength;
     return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}

hseSrvResponse_t ExportEncryptedSymKeyReq(hseKeyHandle_t targetKeyHandle, hseKeyInfo_t *pKeyInfo,
                                          uint8_t *pOutBuffer, uint16_t *pOutBufferLength,
                                          hseKeyHandle_t cipherKeyHandle, hseCipherScheme_t cipherScheme)
{
    return ExportSymKeyReqMuChannel(muIf, muChannelIdx, targetKeyHandle, pKeyInfo, pOutBuffer, pOutBufferLength, cipherKeyHandle, &cipherScheme);
}

/*****************************************************************************************
 *                                      Authenticated
 ****************************************************************************************/
hseSrvResponse_t ImportAuthSymKeyReq(hseKeyHandle_t authKeyHandle, hseAuthScheme_t authScheme,
                                     hseKeyHandle_t targetKeyHandle, hseKeyType_t keyType,
                                     hseKeyFlags_t keyFlags, uint16_t keyByteLength,
                                     uint8_t* pKey, uint16_t keyLength,
                                     uint8_t* pSign0, uint16_t sign0Length,
                                     uint8_t* pSign1, uint16_t sign1Length)
{
    #if defined(HSE_SPT_IMPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseImportKeySrv_t*  pImportKeyReq;

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;
    pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    pImportKeyReq->keyContainer.authKeyHandle = authKeyHandle;
    pImportKeyReq->keyContainer.authScheme = authScheme;
    pImportKeyReq->keyContainer.authLen[0] = sign0Length;
    pImportKeyReq->keyContainer.authLen[1] = sign1Length;
    pImportKeyReq->keyContainer.pAuth[0] = (HOST_ADDR)pSign0;
    pImportKeyReq->keyContainer.pAuth[1] = (HOST_ADDR)pSign1;

    pImportKeyReq->targetKeyHandle = targetKeyHandle;
    keyInfo.keyType = keyType;
    keyInfo.keyFlags = keyFlags;
    keyInfo.keyBitLen = (keyByteLength * 8);
    keyInfo.keyCounter  = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[2] = (HOST_ADDR)pKey;
    pImportKeyReq->keyLen[2] = keyLength;

    pImportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)authKeyHandle;
    (void)authScheme;
    (void)targetKeyHandle;
    (void)keyType;
    (void)keyFlags;
    (void)keyByteLength;
    (void)pKey;
    (void)keyLength;
    (void)pSign0;
    (void)sign0Length;
    (void)pSign1;
    (void)sign1Length;
     return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}

hseSrvResponse_t ExportAuthSymKeyReq(hseKeyHandle_t authKeyHandle, hseAuthScheme_t authScheme,
                                     hseKeyHandle_t targetKeyHandle, hseKeyInfo_t *pKeyInfo,
                                     uint8_t *pOutBuffer, uint16_t *pOutBufferLength,
                                     uint8_t* pSign0, uint16_t *pSign0Length,
                                     uint8_t* pSign1, uint16_t *pSign1Length)
{
    #if defined(HSE_SPT_EXPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseExportKeySrv_t* pExportKeyReq = &pHseSrvDesc->hseSrv.exportKeyReq;

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    pHseSrvDesc->srvId = HSE_SRV_ID_EXPORT_KEY;
    pExportKeyReq->targetKeyHandle = targetKeyHandle;
    pExportKeyReq->pKeyInfo = (HOST_ADDR)pKeyInfo;
    pExportKeyReq->pKey[2] = (HOST_ADDR)pOutBuffer;
    pExportKeyReq->pKeyLen[2] = (HOST_ADDR)pOutBufferLength;

    pExportKeyReq->keyContainer.authKeyHandle = authKeyHandle;
    pExportKeyReq->keyContainer.authScheme = authScheme;
    pExportKeyReq->keyContainer.pAuth[0] = (HOST_ADDR)pSign0;
    pExportKeyReq->keyContainer.pAuthLen[0] = (HOST_ADDR)pSign0Length;
    pExportKeyReq->keyContainer.pAuth[1] = (HOST_ADDR)pSign1;
    pExportKeyReq->keyContainer.pAuthLen[1] = (HOST_ADDR)pSign1Length;

    pExportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)authKeyHandle;
    (void)authScheme;
    (void)targetKeyHandle;
    (void)pKeyInfo;
    (void)pOutBuffer;
    (void)pOutBufferLength;
    (void)pSign0;
    (void)pSign0Length;
    (void)pSign1;
    (void)pSign1Length;
     return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}

/*****************************************************************************************
 *                                 Encrypted & Authenticated
 ****************************************************************************************/

hseSrvResponse_t ImportAuthEncSymKeyReq
(
    hseKeyHandle_t cipherKeyHandle,
    hseCipherScheme_t cipherScheme,
    hseKeyHandle_t authKeyHandle,
    hseAuthScheme_t authScheme,
    hseKeyHandle_t targetKeyHandle,
    hseKeyType_t keyType,
    hseKeyFlags_t keyFlags,
    uint16_t keyByteLength,
    uint8_t* pEncKey,
    uint16_t encKeyLength,
    uint8_t* pSign0,
    uint16_t sign0Length,
    uint8_t* pSign1,
    uint16_t sign1Length,
    uint8_t* pAuthContainer,
    uint16_t authContainerLen
)
{
    #if defined(HSE_SPT_IMPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseImportKeySrv_t*  pImportKeyReq;

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;

    pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    pImportKeyReq->cipher.cipherKeyHandle = cipherKeyHandle;
    pImportKeyReq->cipher.cipherScheme = cipherScheme;

    pImportKeyReq->keyContainer.authKeyHandle = authKeyHandle;
    pImportKeyReq->keyContainer.authScheme = authScheme;
    pImportKeyReq->keyContainer.authLen[0] = sign0Length;
    pImportKeyReq->keyContainer.authLen[1] = sign1Length;
    pImportKeyReq->keyContainer.pAuth[0] = (HOST_ADDR)pSign0;
    pImportKeyReq->keyContainer.pAuth[1] = (HOST_ADDR)pSign1;
    pImportKeyReq->keyContainer.pKeyContainer = (HOST_ADDR)pAuthContainer;
    pImportKeyReq->keyContainer.keyContainerLen = authContainerLen;

    pImportKeyReq->targetKeyHandle = targetKeyHandle;
    keyInfo.keyType = keyType;
    keyInfo.keyFlags = keyFlags;
    keyInfo.keyBitLen = (keyByteLength * 8);
    keyInfo.keyCounter  = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[2] = (HOST_ADDR)pEncKey;
    pImportKeyReq->keyLen[2] = encKeyLength;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)cipherKeyHandle;
    (void)cipherScheme;
    (void)authKeyHandle;
    (void)authScheme;
    (void)targetKeyHandle;
    (void)keyType;
    (void)keyFlags;
    (void)keyByteLength;
    (void)pEncKey;
    (void)encKeyLength;
    (void)pSign0;
    (void)sign0Length;
    (void)pSign1;
    (void)sign1Length;
    (void)pAuthContainer;
    (void)authContainerLen;
     return HSE_SRV_RSP_NOT_SUPPORTED;

    #endif
}
#ifdef HSE_SPT_KEY_GEN
hseSrvResponse_t GenerateAesKey(hseKeyHandle_t *pTargetKeyHandle, uint8_t isNvm, uint16_t keyBitLen,
                                hseKeyFlags_t keyFlags, hseAesBlockModeMask_t cipherBlockMode)
{
    return GenerateSymKey(HSE_KEY_TYPE_AES, keyBitLen, pTargetKeyHandle, isNvm, keyFlags, cipherBlockMode);
}
#endif
/*****************************************************************************************
 *                                            Classic DH
 ****************************************************************************************/
#ifdef HSE_SPT_CLASSIC_DH
hseSrvResponse_t ImportClassicDhKeyReq
(
    hseKeyHandle_t targetKeyHandle,
    bool_t bIsKeyPair,
    hseKeyFlags_t keyFlags,
    uint16_t keyBitLength,
    const uint8_t* pPrimeModulus,
    const uint8_t* pPubKey,
    uint16_t privKeyBitLength,
    const uint8_t* pPrivKey
)
{
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    memset(&keyInfo, 0, sizeof(hseKeyInfo_t));
    keyInfo.keyType = (bIsKeyPair)? HSE_KEY_TYPE_DH_PAIR : HSE_KEY_TYPE_DH_PUB;
    keyInfo.keyFlags = keyFlags;
    keyInfo.keyBitLen = keyBitLength;
    keyInfo.keyCounter = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;
    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[0] = (HOST_ADDR)pPrimeModulus;
    pImportKeyReq->keyLen[0] = BITS_TO_BYTES(keyBitLength);
    pImportKeyReq->pKey[1] = (HOST_ADDR)pPubKey;
    pImportKeyReq->keyLen[1] = BITS_TO_BYTES(keyBitLength);
    pImportKeyReq->pKey[2] = (HOST_ADDR)pPrivKey;
    pImportKeyReq->keyLen[2] = BITS_TO_BYTES(privKeyBitLength);
    pImportKeyReq->targetKeyHandle = targetKeyHandle;

    pImportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
}
#endif

#ifdef HSE_SPT_CLASSIC_DH
hseSrvResponse_t LoadClassicDhPublicKey(hseKeyHandle_t *pTargetKeyHandle,
                                        uint16_t keyBitLength,
                                        const uint8_t* pPrimeModulus,
                                        const uint8_t* pPublicKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(FALSE, HSE_KEY_TYPE_DH_PUB, keyBitLength, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    hseResponse = ImportClassicDhKeyReq(
            *pTargetKeyHandle,
            FALSE,
            HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
            keyBitLength,
            pPrimeModulus,
            pPublicKey,
            0,
            NULL);

    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }

exit:
    return hseResponse;
}
#endif

#ifdef HSE_SPT_CLASSIC_DH
hseSrvResponse_t LoadClassicDhKeyPair(hseKeyHandle_t *pTargetKeyHandle,
                                      uint16_t keyBitLength,
                                      const uint8_t* pPrimeModulus,
                                      const uint8_t* pPublicKey,
                                      uint16_t privKeyBitLength,
                                      const uint8_t* pPrivateKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(FALSE, HSE_KEY_TYPE_DH_PAIR, keyBitLength, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    hseResponse = ImportClassicDhKeyReq(
            *pTargetKeyHandle,
            TRUE,
            HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
            keyBitLength,
            pPrimeModulus,
            pPublicKey,
            privKeyBitLength,
            pPrivateKey);

    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }

exit:
    return hseResponse;
}
#endif

#ifdef HSE_SPT_CLASSIC_DH_KEY_PAIR_GEN
hseSrvResponse_t GenerateClassicDhKey(uint32_t *pTargetKeyHandle,
                                      uint16_t keyBitLen,
                                      const uint8_t* pPrimeModulus,
                                      uint16_t baseGByteLen,
                                      const uint8_t* pBaseG)
{
    return GenerateClassicDhKeyReq(pTargetKeyHandle,
                                   keyBitLen,
                                   pPrimeModulus,
                                   baseGByteLen,
                                   pBaseG,
                                   NULL);
}
#endif

/*****************************************************************************************
 *                                            ECC
 ****************************************************************************************/

hseSrvResponse_t ImportFormattedEccKeyReq
(
    hseKeyHandle_t targetKeyHandle,
    hseKeyType_t keyType,
    hseKeyFlags_t keyFlags,
    hseEccCurveId_t eccCurveId,
    uint16_t keyBitLength,
    hseEccKeyFormat_t keyFormat,
    const uint8_t* pPubKey,
    const uint8_t* pPrivKey
)
{
    #if defined(HSE_SPT_IMPORT_KEY)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    uint16_t pubKeyByteLen = BITS_TO_BYTES(keyBitLength);

    /* Set the public key length for x & y if curve not ED25519 */
    if ((HSE_EC_25519_ED25519 != eccCurveId) && (HSE_EC_25519_CURVE25519 != eccCurveId) &&
        (HSE_EC_448_ED448 != eccCurveId) && (HSE_EC_448_CURVE448 != eccCurveId))
    {
        if(HSE_KEY_FORMAT_ECC_PUB_RAW == keyFormat)
        {
            pubKeyByteLen = 2 * BITS_TO_BYTES(keyBitLength);
        }
        else if(HSE_KEY_FORMAT_ECC_PUB_COMPRESSED == keyFormat)
        {
            pubKeyByteLen = BITS_TO_BYTES(keyBitLength) + 1;
        }
        else
        {
            pubKeyByteLen = 2 * BITS_TO_BYTES(keyBitLength) + 1;
        }
    }

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    memset(&keyInfo, 0, sizeof(hseKeyInfo_t));
    keyInfo.keyType = keyType;
    keyInfo.keyFlags = keyFlags;
    keyInfo.specific.eccCurveId = eccCurveId;
    keyInfo.keyBitLen = keyBitLength;
    keyInfo.keyCounter = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;
    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[0] = (HOST_ADDR)pPubKey;
    pImportKeyReq->keyLen[0] = pubKeyByteLen;
    pImportKeyReq->pKey[2] = (HOST_ADDR)pPrivKey;
    pImportKeyReq->keyLen[2] = BITS_TO_BYTES(keyBitLength);
    pImportKeyReq->targetKeyHandle = targetKeyHandle;

    pImportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;
    pImportKeyReq->keyFormat.eccKeyFormat = keyFormat;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
    #else
    (void)targetKeyHandle;
    (void)keyType;
    (void)keyFlags;
    (void)eccCurveId;
    (void)keyBitLength;
    (void)pPubKey;
    (void)pPrivKey;
    return HSE_SRV_RSP_NOT_SUPPORTED;
    #endif
}

hseSrvResponse_t ImportFormattedCertEccKeyReq
(
    hseKeyHandle_t targetKeyHandle,
    hseKeyType_t keyType,
    hseKeyFlags_t keyFlags,
    hseEccCurveId_t eccCurveId,
    uint16_t keyBitLength,
    hseEccKeyFormat_t keyFormat,
    const uint8_t* pPubKey,
    uint16_t containerLen,
    const uint8_t* pContainer
)
{
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseImportKeySrv_t *pImportKeyReq = &pHseSrvDesc->hseSrv.importKeyReq;
    uint16_t pubKeyByteLen = BITS_TO_BYTES(keyBitLength);

    /* Set the public key length for x & y if curve not ED25519 */
    if ((HSE_EC_25519_ED25519 != eccCurveId) && (HSE_EC_25519_CURVE25519 != eccCurveId) &&
        (HSE_EC_448_ED448 != eccCurveId) && (HSE_EC_448_CURVE448 != eccCurveId))
    {
        if(HSE_KEY_FORMAT_ECC_PUB_RAW == keyFormat)
        {
            pubKeyByteLen = 2 * BITS_TO_BYTES(keyBitLength);
        }
        else if(HSE_KEY_FORMAT_ECC_PUB_COMPRESSED == keyFormat)
        {
            pubKeyByteLen = BITS_TO_BYTES(keyBitLength) + 1;
        }
        else
        {
            pubKeyByteLen = 2 * BITS_TO_BYTES(keyBitLength) + 1;
        }
    }

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    memset(&keyInfo, 0, sizeof(hseKeyInfo_t));
    keyInfo.keyType = keyType;
    keyInfo.keyFlags = keyFlags;
    keyInfo.specific.eccCurveId = eccCurveId;
    keyInfo.keyBitLen = keyBitLength;
    keyInfo.keyCounter = (HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle)) ? (++gKeySecCount) : 0UL;
    pHseSrvDesc->srvId = HSE_SRV_ID_IMPORT_KEY;
    pImportKeyReq->pKeyInfo = (HOST_ADDR)&keyInfo;
    pImportKeyReq->pKey[0] = (HOST_ADDR)pPubKey;
    pImportKeyReq->keyLen[0] = pubKeyByteLen;
    pImportKeyReq->keyContainer.pKeyContainer = (HOST_ADDR)pContainer;
    pImportKeyReq->keyContainer.keyContainerLen = containerLen;
    pImportKeyReq->targetKeyHandle = targetKeyHandle;

    pImportKeyReq->cipher.cipherKeyHandle = HSE_INVALID_KEY_HANDLE;
    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;
    pImportKeyReq->keyFormat.eccKeyFormat = keyFormat;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
}

/*******************************************************************************
 *                              Other KM Services
 ******************************************************************************/

hseSrvResponse_t EraseKeyReq
(
    hseKeyHandle_t        keyHandle, //HSE_INVALID_KEY_HANDLE to delete more then one key
    hseEraseKeyOptions_t  eraseKeyOptions
)
{
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_ERASE_KEY;
    pHseSrvDesc->hseSrv.eraseKeyReq.keyHandle = keyHandle;
    pHseSrvDesc->hseSrv.eraseKeyReq.eraseKeyOptions = eraseKeyOptions;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
}

#ifdef HSE_B
hseSrvResponse_t EraseNVMDataReq(void)
{
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_ERASE_HSE_NVM_DATA;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);
}
#endif

hseSrvResponse_t GetKeyInfo(hseKeyHandle_t keyHandle, hseKeyInfo_t* reqKeyInfo)
{
     return GetKeyInfoMuChannel(muIf, muChannelIdx, keyHandle, reqKeyInfo);
}

hseSrvResponse_t GetKeyInfoMuChannel(uint8_t u8MuInstance, uint8_t u8MuChannel,
     hseKeyHandle_t keyHandle, hseKeyInfo_t* reqKeyInfo)
{
    #if defined(HSE_SPT_GET_KEY_INFO)
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[u8MuInstance][u8MuChannel];
    hseGetKeyInfoSrv_t*  pGetKeyInfoReq = &pHseSrvDesc->hseSrv.getKeyInfoReq;
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_GET_KEY_INFO;
    pGetKeyInfoReq->keyHandle = keyHandle;
    pGetKeyInfoReq->pKeyInfo = (HOST_ADDR)reqKeyInfo;
    return HSE_Send(u8MuInstance, u8MuChannel, gSyncTxOption, pHseSrvDesc);
    #else
    (void)u8MuInstance;
    (void)u8MuChannel;
    (void)keyHandle;
    (void)reqKeyInfo;
    return HSE_SRV_RSP_NOT_SUPPORTED;
    #endif
}

/*****************************************************************************************
 *                                 Plain
 ****************************************************************************************/

/*******************************************************************************
 *                                 SymKey
 ******************************************************************************/

hseSrvResponse_t LoadAesKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, uint16_t keyByteLength,
                            const uint8_t* pKey)
{
    return LoadSymKey(pTargetKeyHandle, isNvmKey, HSE_KEY_TYPE_AES, keyByteLength, pKey);
}

hseSrvResponse_t LoadAesKeyWithCipherMode(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, uint16_t keyByteLength,
                                          const uint8_t* pKey, hseAesBlockModeMask_t cipherModeMask)
{
    return LoadSymKeyWithCipherMode(pTargetKeyHandle, isNvmKey, HSE_KEY_TYPE_AES, keyByteLength, pKey, cipherModeMask);
}

hseSrvResponse_t LoadTweakKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, uint16_t keyByteLength,
                              const uint8_t* pKey, hseKeyFlags_t keyFlags, hseAesBlockModeMask_t cipherModeMask)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_AES, BYTES_TO_BITS(keyByteLength), pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainSymKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_AES, keyFlags, keyByteLength, pKey, cipherModeMask);

exit:
    return hseResponse;
}

hseSrvResponse_t LoadHmacKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, uint16_t keyByteLength,
                             const uint8_t* pKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_HMAC, keyByteLength * 8U, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainSymKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_HMAC,
            (HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE),
            keyByteLength, pKey, 0U);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadSipHashKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, uint16_t keyByteLength,
                                const uint8_t* pKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_SIPHASH, keyByteLength * 8U, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainSymKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_SIPHASH,
            (HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY) ,
            keyByteLength, pKey, 0U);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadSymKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseKeyType_t keyType,
                            uint16_t keyByteLength, const uint8_t* pKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, keyType, BYTES_TO_BITS(keyByteLength), pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainSymKeyReq(*pTargetKeyHandle, keyType,
            (HSE_KF_USAGE_ENCRYPT|HSE_KF_USAGE_DECRYPT|
            HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE),
            keyByteLength, pKey, 0U);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadSymKeyWithCipherMode(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseKeyType_t keyType,
                                          uint16_t keyByteLength, const uint8_t* pKey, hseAesBlockModeMask_t cipherModeMask)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, keyType, BYTES_TO_BITS(keyByteLength), pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainSymKeyReq(*pTargetKeyHandle, keyType,
            (HSE_KF_USAGE_ENCRYPT|HSE_KF_USAGE_DECRYPT|
            HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE),
            keyByteLength, pKey, cipherModeMask);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadPSK(hseKeyHandle_t *pTargetKeyHandle, uint16_t keyByteLength, const uint8_t *pKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(NVM_KEY, HSE_KEY_TYPE_HMAC, BYTES_TO_BITS(keyByteLength), pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainSymKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_HMAC, HSE_KF_USAGE_DERIVE,
                                       keyByteLength, pKey, 0U);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

/*******************************************************************************
 *                                 SymKey for SMR testing
 ******************************************************************************/

hseSrvResponse_t LoadSmrAuthAesKey(hseKeyHandle_t* pTargetKeyHandle, bool_t isNvmKey,
                            uint16_t keyByteLength, const uint8_t* pKey)
{
    return LoadSmrAuthSymKey(pTargetKeyHandle, isNvmKey, HSE_KEY_TYPE_AES, keyByteLength, pKey);
}

hseSrvResponse_t LoadSmrHmacKey(hseKeyHandle_t* pTargetKeyHandle, bool_t isNvmKey,
                             uint16_t keyByteLength, const uint8_t* pKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_HMAC, keyByteLength * 8U, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainSymKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_HMAC,
            (HSE_KF_USAGE_VERIFY),
            keyByteLength, pKey, 0U);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadSmrAuthSymKey(hseKeyHandle_t* pTargetKeyHandle, bool_t isNvmKey, hseKeyType_t keyType,
                            uint16_t keyByteLength, const uint8_t* pKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, keyType, keyByteLength * 8U, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainSymKeyReq(*pTargetKeyHandle, keyType,
                        (HSE_KF_USAGE_VERIFY), keyByteLength, pKey, 0U);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadSmrDecKey(hseKeyHandle_t* pTargetKeyHandle, bool_t isNvmKey,
                            uint16_t keyByteLength, const uint8_t* pKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_AES, keyByteLength * 8U, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainSymKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_AES,
            (HSE_KF_USAGE_SMR_DECRYPT | HSE_KF_USAGE_VERIFY),
            keyByteLength, pKey, (HSE_KU_AES_BLOCK_MODE_CTR | HSE_KU_AES_BLOCK_MODE_GCM));
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
    }
exit:
    return hseResponse;
}

/*******************************************************************************
 *                                    RSA
 ******************************************************************************/

hseSrvResponse_t LoadRsaPublicKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, uint16_t keyBitLen,
                                  const uint8_t *pModulus, uint16_t publicExponentLength,
                                  const uint8_t *pPublicExponent)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_RSA_PUB, keyBitLen, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainRsaKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_RSA_PUB,
                                (HSE_KF_USAGE_ENCRYPT|HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE),
                                (HOST_ADDR)pModulus, keyBitLen,
                                (HOST_ADDR)pPublicExponent, publicExponentLength,
                                (HOST_ADDR)NULL);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadRsaPrivateKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, uint16_t keyBitLen,
                                   const uint8_t *pModulus, uint16_t publicExponentLength,
                                   const uint8_t *pPublicExponent, const uint8_t *pPrivateExponent)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_RSA_PAIR, keyBitLen, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainRsaKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_RSA_PAIR,
                                (HSE_KF_USAGE_DECRYPT|HSE_KF_USAGE_SIGN | HSE_KF_ACCESS_EXPORTABLE),
                                (HOST_ADDR)pModulus, keyBitLen,
                                (HOST_ADDR)pPublicExponent, publicExponentLength,
                                (HOST_ADDR)pPrivateExponent);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadRsaPair(hseKeyHandle_t* pTargetKeyHandle, bool_t isNvmKey, uint16_t keyBitLen,
                                   const uint8_t *pModulus, uint16_t publicExponentLength,
                                   const uint8_t *pPublicExponent, const uint8_t *pPrivateExponent)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_RSA_PAIR, keyBitLen, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainRsaKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_RSA_PAIR,
                                (HSE_KF_USAGE_ENCRYPT | HSE_KF_USAGE_DECRYPT | HSE_KF_USAGE_SIGN | HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE),
                                (HOST_ADDR)pModulus, keyBitLen,
                                (HOST_ADDR)pPublicExponent, publicExponentLength,
                                (HOST_ADDR)pPrivateExponent);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}


hseSrvResponse_t LoadRsaCertKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseKeyInfo_t* pKeyInfo,
                                uint16_t modulusLength, const uint8_t *pModulus,
                                uint16_t publicExpLength, const uint8_t *pPublicExponent,
                                uint16_t containerLength, const uint8_t *pContainer)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_RSA_PUB_EXT, pKeyInfo->keyBitLen, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportCertRsaKeyReq(*pTargetKeyHandle, pKeyInfo,
                               modulusLength, pModulus,
                               publicExpLength, pPublicExponent,
                               containerLength, pContainer);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}


/*******************************************************************************
 *                                    SMR RSA
 ******************************************************************************/

hseSrvResponse_t LoadSmrRsaPublicKey(hseKeyHandle_t* pTargetKeyHandle, bool_t isNvmKey, uint16_t keyBitLen,
                                  const uint8_t *pModulus, uint16_t publicExponentLength,
                                  const uint8_t *pPublicExponent)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_RSA_PUB, keyBitLen, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportPlainRsaKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_RSA_PUB,
                                (HSE_KF_USAGE_VERIFY),
                                (HOST_ADDR)pModulus, keyBitLen,
                                (HOST_ADDR)pPublicExponent, publicExponentLength,
                                (HOST_ADDR)NULL);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

/*******************************************************************************
 *                                   ECC
 ******************************************************************************/

hseSrvResponse_t LoadEccKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                            uint16_t keyBitLength)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, WRP_KEY_TYPE_ECC_PAIR, WRP_ECC_KEY_SIZE, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PAIR,
                             HSE_KF_USAGE_SIGN | HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE,
                             eccCurveId, keyBitLength, NULL, NULL);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadEccPublicKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                  uint16_t keyBitLength, const uint8_t* pPubKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, WRP_KEY_TYPE_ECC_PUB, WRP_ECC_KEY_SIZE, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PUB,
                             HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE,
                             eccCurveId, keyBitLength, pPubKey, NULL);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadEccCompressedPublicKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                  uint16_t keyBitLength, const uint8_t* pPubKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, WRP_KEY_TYPE_ECC_PUB, WRP_ECC_KEY_SIZE, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportFormattedEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PUB,
                             HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE,
                             eccCurveId, keyBitLength, HSE_KEY_FORMAT_ECC_PUB_COMPRESSED, pPubKey, NULL);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadEccUncompressedPublicKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                  uint16_t keyBitLength, const uint8_t* pPubKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, WRP_KEY_TYPE_ECC_PUB, WRP_ECC_KEY_SIZE, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportFormattedEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PUB,
                             HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE,
                             eccCurveId, keyBitLength, HSE_KEY_FORMAT_ECC_PUB_UNCOMPRESSED, pPubKey, NULL);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadEccCompressedExternalPublicKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                  uint16_t keyBitLength, const uint8_t* pPubKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, WRP_KEY_TYPE_ECC_PUB_EXT, WRP_ECC_KEY_SIZE, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportFormattedCertEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PUB_EXT,
                             HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE,
                             eccCurveId, keyBitLength, HSE_KEY_FORMAT_ECC_PUB_COMPRESSED, pPubKey, BITS_TO_BYTES(keyBitLength) + 1, pPubKey);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadEccUncompressedExternalPublicKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                  uint16_t keyBitLength, const uint8_t* pPubKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, WRP_KEY_TYPE_ECC_PUB_EXT, WRP_ECC_KEY_SIZE, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportFormattedCertEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PUB_EXT,
                             HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE,
                             eccCurveId, keyBitLength, HSE_KEY_FORMAT_ECC_PUB_UNCOMPRESSED, pPubKey, 2 * BITS_TO_BYTES(keyBitLength) + 1, pPubKey);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadEccPublicKeyForKeyExchange(hseKeyHandle_t targetKeyHandle, hseEccCurveId_t eccCurveId,
                                  uint16_t keyBitLength, const uint8_t* pPubKey)
{
    return ImportEccKeyReq(targetKeyHandle, HSE_KEY_TYPE_ECC_PUB,
                           HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                           eccCurveId, keyBitLength, pPubKey, NULL);
}

hseSrvResponse_t LoadSmrEccPublicKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                  uint16_t keyBitLength, const uint8_t* pPubKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_ECC_PUB, keyBitLength, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PUB,
                           HSE_KF_USAGE_VERIFY,
                           eccCurveId, keyBitLength, pPubKey, NULL);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadEccPrivateKey(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                   uint16_t keyBitLength, const uint8_t* pPrivKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, WRP_KEY_TYPE_ECC_PAIR, WRP_ECC_KEY_SIZE, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PAIR,
                           HSE_KF_USAGE_SIGN | HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE,
                           eccCurveId, keyBitLength, NULL, pPrivKey);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadEccKeyPair(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                uint16_t keyBitLength, const uint8_t* pPubKey,
                                const uint8_t* pPrivKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, WRP_KEY_TYPE_ECC_PAIR, WRP_ECC_KEY_SIZE, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PAIR,
                           HSE_KF_USAGE_SIGN | HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE,
                           eccCurveId, keyBitLength, pPubKey, pPrivKey);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadSmrEccKeyPair(hseKeyHandle_t* pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                uint16_t keyBitLength, const uint8_t* pPubKey,
                                const uint8_t* pPrivKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, HSE_KEY_TYPE_ECC_PAIR, keyBitLength, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PAIR,
                           HSE_KF_USAGE_VERIFY,
                           eccCurveId, keyBitLength, pPubKey, pPrivKey);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

hseSrvResponse_t LoadEccKeyPairForKeyExchange(hseKeyHandle_t *pTargetKeyHandle, bool_t isNvmKey, hseEccCurveId_t eccCurveId,
                                  uint16_t keyBitLength, const uint8_t* pPubKey,
                                  const uint8_t* pPrivKey)
{
    hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseResponse = HKF_AllocKeySlot(isNvmKey, WRP_KEY_TYPE_ECC_PAIR, WRP_ECC_KEY_SIZE, pTargetKeyHandle);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    hseResponse = ImportEccKeyReq(*pTargetKeyHandle, HSE_KEY_TYPE_ECC_PAIR,
                             (HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE),
                             eccCurveId, keyBitLength, pPubKey, pPrivKey);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        HKF_FreeKeySlot(pTargetKeyHandle);
        goto exit;
    }
exit:
    return hseResponse;
}

/*******************************************************************************
 *                                   SHE
 ******************************************************************************/
#ifdef HSE_SPT_SHE
hseSrvResponse_t LoadShePlainKey(const uint8_t *pKey)
{

    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    hseSheLoadPlainKeySrv_t* pSheLoadPlainKeyReq = &pHseSrvDesc->hseSrv.sheLoadPlainKeyReq;

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));
    pHseSrvDesc->srvId = HSE_SRV_ID_SHE_LOAD_PLAIN_KEY;
    pSheLoadPlainKeyReq->pKey = (HOST_ADDR) pKey;

    return HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);

}
#endif
hseSrvResponse_t ImportEccKeyReq(hseKeyHandle_t targetKeyHandle, hseKeyType_t keyType,
                                 hseKeyFlags_t keyFlags, hseEccCurveId_t eccCurveId,
                                 uint16_t keyBitLength, const uint8_t* pPubKey,
                                 const uint8_t* pPrivKey)
{
    return ImportFormattedEccKeyReq(targetKeyHandle, keyType,
                                    keyFlags, eccCurveId,
                                    keyBitLength, HSE_KEY_FORMAT_ECC_PUB_RAW,
                                    pPubKey, pPrivKey);
}
#ifdef HSE_SPT_KEY_VERIFY
hseSrvResponse_t VerifySHA(const uint8_t* pKey, hseKeyHandle_t* pKeyHandle)
{
    static uint32_t hashLength = 32U;
    static uint8_t  hashStore[32] = { 0U };
    hseSrvDescriptor_t *pHseSrvDesc = NULL;
    hseKeyVerifySrv_t  *pVerifyKey = NULL;
    hseSrvResponse_t response;

    response = HashReq(HSE_ACCESS_MODE_ONE_PASS, muIf, muChannelIdx, HSE_HASH_ALGO_SHA2_256,
                       0U, 16U, pKey, &hashLength, hashStore, gSyncTxOption, HSE_SGT_OPTION_NONE);
    if(HSE_SRV_RSP_OK != response)
    {
        goto exit;
    }

    pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];
    pHseSrvDesc->srvId = HSE_SRV_ID_KEY_VERIFY;
    pVerifyKey = &pHseSrvDesc->hseSrv.verifyKeyReq;
    pVerifyKey->keyVerAlgo = HSE_KEY_VER_SHA256;
    pVerifyKey->keyHandle  = (*pKeyHandle);
    pVerifyKey->tagLen     = (uint8_t)hashLength;
    pVerifyKey->pTag       = (HOST_ADDR)hashStore;
    pVerifyKey->cmackeyHandle = 0U; /* Not used in SHA256 algo*/
    response = HSE_Send(muIf, muChannelIdx, gSyncTxOption, pHseSrvDesc);

exit:
    return response;
}
#endif

#ifdef __cplusplus
}
#endif

/** @} */

