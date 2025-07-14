/**
*   @file    hse_host_km_import_key.c
*
*   @brief   This file implements wrappers for key import.
*
*   @addtogroup [HOST_FRAMEWORK]
*   @{
*/
/*==================================================================================================
*
*   Copyright 2019-2021 NXP
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

#include "string.h"
#include "global_defs.h"
#include "hse_host.h"
#include "hse_host_km_utils.h"
#include "hse_demo_app_services.h"
#include "hse_defs.h"

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
static uint8_t muChannelIdx = 1U; /* channel 1 */
/*==================================================================================================
 *                                      GLOBAL CONSTANTS
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

/*************************************************************************************************
* Description:  Get the bit length based on the Curve ID
************************************************************************************************/
static uint16_t GetKeyBitLen(hseEccCurveId_t eccCurveId)
{
    switch(eccCurveId)
    {
        case HSE_EC_SEC_SECP256R1:
            return 256U;

        case HSE_EC_SEC_SECP384R1:
            return 384U;

        case HSE_EC_SEC_SECP521R1:
            return 521U;

        case HSE_EC_BRAINPOOL_BRAINPOOLP256R1:
            return 256U;

        case HSE_EC_BRAINPOOL_BRAINPOOLP320R1:
            return 320U;

        case HSE_EC_BRAINPOOL_BRAINPOOLP384R1:
            return 384U;

        case HSE_EC_BRAINPOOL_BRAINPOOLP512R1:
            return 512U;

        case HSE_EC_25519_ED25519:
            return 256U;

        case HSE_EC_25519_CURVE25519:
            return 256U;

        case HSE_EC_448_ED448:
            return 456U;

        case HSE_EC_448_CURVE448:
            return 448U;

        default:
            return 0U;
    }
}

/*************************************************************************************************
* Description:  Get length of ECC/ED public key based on Curve ID
************************************************************************************************/
static uint16_t GetPubKeyLen(hseEccCurveId_t eccCurveId)
{
    return ( (HSE_EC_25519_ED25519 == eccCurveId) || (HSE_EC_25519_CURVE25519 == eccCurveId) ) ?
        BITS_TO_BYTES(GetKeyBitLen(eccCurveId)) :       /* If   ED curve (Twisted/Montgomery) - Q length is curveBitLen */
        BITS_TO_BYTES(GetKeyBitLen(eccCurveId)) * 2U;   /* Else Qx||Qy length is 2 * curveBitLen */
}

/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
 ==================================================================================================*/

/*************************************************************************************************
* Description:  Imports a key given the key handle, key info and key value(s)
************************************************************************************************/
hseSrvResponse_t HSE_ImportKey
(
    uint8_t u8MuInstance,
    hseKeyHandle_t targetKeyHandle,
    hseKeyInfo_t *pKeyInfo,
    const uint8_t *pKey0, uint32_t keyLen0,
    const uint8_t *pKey1, uint32_t keyLen1,
    const uint8_t *pKey2, uint32_t keyLen2
    #if defined(HSE_KEY_FORMAT_ECC_PUB_RAW)
    ,hseEccKeyFormat_t eccKeyFormat
    #endif
)
{
    hseSrvDescriptor_t *pHseSrvDesc;
    hseImportKeySrv_t *pImportKeyReq;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;


    /* NVM key slots can be updated (i.e. importing in a non-empty slot) only if authenticated */
    if(HSE_KEY_CATALOG_ID_NVM == GET_CATALOG_ID(targetKeyHandle))
    {
        hseKeyInfo_t keyInfo;
        srvResponse = HSE_GetKeyInfo(targetKeyHandle, &keyInfo, u8MuInstance);
        if(HSE_SRV_RSP_OK == srvResponse)
        {
            /* NVM slot is already populated - CANNOT update without authentication (see `HSE_UpdateNvmKey_Example`) */
            /* Erase the key to enable import in plain */
            srvResponse = HSE_EraseKey(targetKeyHandle, HSE_ERASE_NOT_USED);
            if(HSE_SRV_RSP_OK != srvResponse)
                goto exit;
        }
        /* Other status than OK or EMPTY is an error */
        else if(HSE_SRV_RSP_KEY_EMPTY != srvResponse)
        {
            goto exit;
        }
    }

    /* Clear the service descriptor placed in shared memory */
    pHseSrvDesc = &gHseSrvDesc[u8MuInstance][muChannelIdx];
    pImportKeyReq = &(pHseSrvDesc->hseSrv.importKeyReq);
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    /* Fill the service descriptor */
    pHseSrvDesc->srvId                      = HSE_SRV_ID_IMPORT_KEY;
    pImportKeyReq->targetKeyHandle          = targetKeyHandle;
    pImportKeyReq->pKeyInfo                 = PTR_TO_HOST_ADDR(pKeyInfo);
    pImportKeyReq->pKey[0]                  = PTR_TO_HOST_ADDR(pKey0);
    pImportKeyReq->keyLen[0]                = keyLen0;
    pImportKeyReq->pKey[1]                  = PTR_TO_HOST_ADDR(pKey1);
    pImportKeyReq->keyLen[1]                = keyLen1;
    pImportKeyReq->pKey[2]                  = PTR_TO_HOST_ADDR(pKey2);
    pImportKeyReq->keyLen[2]                = keyLen2;
    #if defined(HSE_KEY_FORMAT_ECC_PUB_RAW)
    pImportKeyReq->keyFormat.eccKeyFormat   = eccKeyFormat;
    #endif

    /* Key imported in plain - not encrypted => cipherKeyHandle = HSE_INVALID_KEY_HANDLE */
    pImportKeyReq->cipher.cipherKeyHandle   = HSE_INVALID_KEY_HANDLE;
    /* Key imported without authentication - not signed => authKeyHandle = HSE_INVALID_KEY_HANDLE */
    pImportKeyReq->keyContainer.authKeyHandle = HSE_INVALID_KEY_HANDLE;

    /* Send the request synchronously */
    srvResponse = HSE_Send(u8MuInstance, muChannelIdx, gSyncTxOption, pHseSrvDesc);
exit:
    return srvResponse;
}

/*************************************************************************************************
* Description:  Imports a symmetric key given the key handle, type (AES/HMAC) and key flags
************************************************************************************************/
hseSrvResponse_t HSE_ImportSymKey
(
    hseKeyHandle_t handle,
    hseKeyType_t type,
    hseKeyFlags_t flags,
    const uint8_t *pKey,
    uint32_t len
)
{
    /* Declare the information about the key to be imported */
    hseKeyInfo_t keyInfo = {
        .keyType = type,                    /* Will import a symmetric key (AES/HMAC) */
        .keyFlags = flags,                  /* Usage flags for this key (e.g. ENCRYPT/DECRYPT/SIGN/VERIFY/PROVISION/DERIVE) */
        .keyBitLen = BYTES_TO_BITS(len),    /* The length in bits (for AES - 128/192/256; for HMAC any value in the
                                                [HSE_MIN_HMAC_KEY_BITS_LEN..HSE_MAX_HMAC_KEY_BITS_LEN] boundaries) */
        .keyCounter = 0UL,                  /* Relevant only for NVM keys - must be > than previous */
        .smrFlags = 0UL,                    /* Not used here - default value */
    };

    return HSE_ImportKey(MU0, handle, &keyInfo, NULL, 0UL, NULL, 0UL, pKey, len
    #if defined(HSE_KEY_FORMAT_ECC_PUB_RAW)
    ,HSE_KEY_FORMAT_ECC_PUB_RAW
    #endif
    );
}

/*************************************************************************************************
* Description:  Imports an ECC key (pub/pair) given the key handle, type (PUB/PAIR), flags and curve
************************************************************************************************/
hseSrvResponse_t HSE_ImportEccKey
(
    hseKeyHandle_t handle,
    hseKeyType_t type,
    hseKeyFlags_t flags,
    hseEccCurveId_t eccCurveId,
    const uint8_t* pPubKey,
    const uint8_t* pPrivKey
    #if defined(HSE_KEY_FORMAT_ECC_PUB_RAW)
    ,hseEccKeyFormat_t eccKeyFormat
    #endif
)
{
    /* Get the key length in bits depending on the curve ID */
    uint16_t keyBitLen = GetKeyBitLen(eccCurveId);
    /* Get the public key length */
    uint16_t pubKeyLen = GetPubKeyLen(eccCurveId);
    /* pPrivKey may be NULL (when importing an ECC_PUB type) */
    uint16_t privKeyLen = (NULL != pPrivKey) ? BITS_TO_BYTES(GetKeyBitLen(eccCurveId)) : 0U;

    /* Declare the information about the key to be imported */
    hseKeyInfo_t keyInfo = {
        .keyType = type,                                            /* Will import an ECC key (public / pair) */
        .keyFlags = flags,                                          /* Usage flags for this key (e.g. SIGN/VERIFY/PROVISION/EXCHANGE) */
        .keyBitLen = keyBitLen,                                     /* Base length in bits of the key - for ECC corresponds to Curve bit length */
        .keyCounter = 0UL,                                          /* Relevant only for NVM keys - must be > than previous */
        .smrFlags = 0UL,                                            /* Not used here - default value */
        .specific.eccCurveId = eccCurveId,                          /* Specific for ECC key - curve ID */
    };

    return HSE_ImportKey(MU0, handle, &keyInfo, pPubKey, pubKeyLen, NULL, 0UL, pPrivKey, privKeyLen
    #if defined(HSE_KEY_FORMAT_ECC_PUB_RAW)
    ,eccKeyFormat
    #endif
    );
}

/*************************************************************************************************
* Description:  Imports a RSA key (pub/pair) given the key handle, type (PUB/PAIR), flags and public exponent length
************************************************************************************************/
hseSrvResponse_t HSE_ImportRsaKey
(
    hseKeyHandle_t handle,
    hseKeyType_t type,
    hseKeyFlags_t flags,
    const uint8_t* pN,
    uint16_t modLen,
    const uint8_t* pE,
    uint16_t eLen,
    const uint8_t* pD
)
{
    /* pD may be NULL (when importing an RSA_PUB type) */
    uint16_t dLen = (NULL != pD) ? modLen : 0U;

    /* Declare the information about the key to be imported */
    hseKeyInfo_t keyInfo = {
        .keyType = type,                                            /* Will import a RSA key (public / pair) */
        .keyFlags = flags,                                          /* Usage flags for this key (e.g. SIGN/VERIFY/PROVISION/EXCHANGE) */
        .keyBitLen =  BYTES_TO_BITS(modLen),                        /* Length of modulus in bits */
        .keyCounter = 0UL,                                          /* Relevant only for NVM keys - must be > than previous */
        .smrFlags = 0UL,                                            /* Not used here - default value */
        .specific.pubExponentSize = eLen,                           /* Specific for RSA key - size of public exponent */
    };

    return HSE_ImportKey(MU0, handle, &keyInfo, pN, modLen, pE, eLen, pD, dLen
    #if defined(HSE_KEY_FORMAT_ECC_PUB_RAW)
    , HSE_KEY_FORMAT_ECC_PUB_RAW
    #endif
    );
}

/*************************************************************************************************
* Description:  Imports a Symmetric key in an authenticated key container
************************************************************************************************/
hseSrvResponse_t HSE_ImportAuthSymKey
(
    const hseKeyHandle_t targetHandle,
    const hseKeyHandle_t authHandle,
    const hseAuthScheme_t *pAuthScheme,
    const uint8_t *pKeyContainer,
    const uint16_t containerLen,
    const uint16_t keyInfoOffset,
    const uint16_t keyDataOffset,
    const uint16_t keyLen,
    const uint8_t *pAuth0,
    const uint8_t *pAuth1,
    const uint16_t authLen0,
    const uint16_t authLen1
)
{
    hseSrvDescriptor_t *pHseSrvDesc;
    hseImportKeySrv_t *pImportKeyReq;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;


    /* Clear the service descriptor placed in shared memory */
    pHseSrvDesc = &gHseSrvDesc[MU0][muChannelIdx];
    pImportKeyReq = &(pHseSrvDesc->hseSrv.importKeyReq);
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    /* Fill the service descriptor */
    pHseSrvDesc->srvId              = HSE_SRV_ID_IMPORT_KEY;
    pImportKeyReq->targetKeyHandle  = targetHandle;
    pImportKeyReq->pKeyInfo         = PTR_TO_HOST_ADDR(&pKeyContainer[keyInfoOffset]);
    pImportKeyReq->pKey[2]          = PTR_TO_HOST_ADDR(&pKeyContainer[keyDataOffset]);
    pImportKeyReq->keyLen[2]        = keyLen;

    /* Key imported in plain - not encrypted => cipherKeyHandle = HSE_INVALID_KEY_HANDLE */
    pImportKeyReq->cipher.cipherKeyHandle   = HSE_INVALID_KEY_HANDLE;

    /* Key imported authenticated */
    pImportKeyReq->keyContainer.authKeyHandle = authHandle;
    (void)memcpy(&pImportKeyReq->keyContainer.authScheme, pAuthScheme, sizeof(hseAuthScheme_t));
    pImportKeyReq->keyContainer.pKeyContainer = PTR_TO_HOST_ADDR(pKeyContainer);
    pImportKeyReq->keyContainer.keyContainerLen = containerLen;
    pImportKeyReq->keyContainer.pAuth[0] = PTR_TO_HOST_ADDR(pAuth0);
    pImportKeyReq->keyContainer.pAuth[1] = PTR_TO_HOST_ADDR(pAuth1);
    pImportKeyReq->keyContainer.authLen[0] = authLen0;
    pImportKeyReq->keyContainer.authLen[1] = authLen1;

    /* Send the request synchronously */
    srvResponse = HSE_Send(MU0, muChannelIdx, gSyncTxOption, pHseSrvDesc);

    return srvResponse;
}

#ifdef __cplusplus
}
#endif

/** @} */

