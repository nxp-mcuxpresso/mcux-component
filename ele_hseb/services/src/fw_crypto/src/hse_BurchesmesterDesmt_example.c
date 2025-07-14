/*
 * @file hse_BD_example.c
 *
 * @brief Header file for Burmester Desmedt Example in crypto services
 * @details Contains the API's for demonstrating the use of Burmester Desmedt crypto service in DemoApp
 */

/*==================================================================================================
*
*   Copyright-2022 NXP.
*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/

#include "hse_BD_example.h"

#ifdef HSE_SPT_BURMESTER_DESMEDT
/*==================================================================================================
 *                                      LOCAL CONSTANTS
 ==================================================================================================*/
static const uint8_t ECC_SECP224R1_PRIME_P[] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

//Elliptic curve coefficient a
static const uint8_t ECC_SECP224R1_CURVEPARAM_A[] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE
};

//Elliptic curve coefficient b
static const uint8_t ECC_SECP224R1_CURVEPARAM_B[] =
{
    0xB4, 0x05, 0x0A, 0x85, 0x0C, 0x04, 0xB3, 0xAB, 0xF5, 0x41, 0x32, 0x56, 0x50, 0x44, 0xB0, 0xB7,
    0xD7, 0xBF, 0xD8, 0xBA, 0x27, 0x0B, 0x39, 0x43, 0x23, 0x55, 0xFF, 0xB4
};

//Elliptic curve base point G(x,y)
static const uint8_t ECC_SECP224R1_BASEPOINT[] =
{
    0xB7, 0x0E, 0x0C, 0xBD, 0x6B, 0xB4, 0xBF, 0x7F, 0x32, 0x13, 0x90, 0xB9, 0x4A, 0x03, 0xC1, 0xD3,
    0x56, 0xC2, 0x11, 0x22, 0x34, 0x32, 0x80, 0xD6, 0x11, 0x5C, 0x1D, 0x21, 0xBD, 0x37, 0x63, 0x88,
    0xB5, 0xF7, 0x23, 0xFB, 0x4C, 0x22, 0xDF, 0xE6, 0xCD, 0x43, 0x75, 0xA0, 0x5A, 0x07, 0x47, 0x64,
    0x44, 0xD5, 0x81, 0x99, 0x85, 0x00, 0x7E, 0x34
};

//Elliptic curve order n
static const uint8_t ECC_SECP224R1_ORDER_N[] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x16, 0xA2,
    0xE0, 0xB8, 0xF0, 0x3E, 0x13, 0xDD, 0x29, 0x45, 0x5C, 0x5C, 0x2A, 0x3D
};

static const uint8_t fcaA1[] =
{
    0x33, 0xb5, 0x8b, 0xf6, 0xc8, 0x4a, 0xb9, 0x7e, 0xa4, 0xd6, 0xde, 0x9a, 0x59, 0x25, 0x25, 0xdf,
    0xa2, 0xf7, 0x24, 0x50, 0x23, 0x64, 0x2c, 0xe6, 0xf4, 0xd4, 0x90, 0xbb
};

static const uint8_t fcaA2[] =
{
    0x0c, 0x24, 0xfe, 0xa3, 0xe5, 0x04, 0x95, 0xd2, 0xb4, 0x13, 0xdb, 0xac, 0xfe, 0x88, 0xd2, 0x13,
    0x89, 0x4f, 0x9e, 0x70, 0x98, 0xca, 0x17, 0x91, 0xad, 0xde, 0xca, 0xc7
};

static const uint8_t fcaA3[] =
{
    0x33, 0xb5, 0xbc, 0x34, 0x6f, 0x4a, 0xa4, 0x51, 0x33, 0x7a, 0x6e, 0x34, 0xed, 0x5a, 0xd0, 0x32,
    0x96, 0xd4, 0xda, 0xb1, 0xbe, 0x44, 0x93, 0x65, 0xac, 0xfe, 0xab, 0x58
};

static const uint8_t fcaZ1[] =
{
    0x1E, 0xC5, 0x91, 0xD3, 0x19, 0xD9, 0x87, 0x2E, 0x2F, 0x62, 0xC6, 0x9D, 0xC1, 0xC4, 0xCA, 0xE6,
    0x7D, 0x3E, 0xBD, 0x03, 0x19, 0x20, 0xB9, 0xBB, 0x7D, 0x06, 0xE9, 0x94, 0x51, 0xEC, 0x05, 0xC4,
    0x1E, 0x12, 0x53, 0xB4, 0x28, 0x29, 0xA5, 0x13, 0xA8, 0x30, 0x17, 0x45, 0x2A, 0x96, 0x91, 0xBB,
    0xE2, 0x3C, 0xB3, 0x97, 0x0D, 0x71, 0xD9, 0xC3
};

static const uint8_t fcaZ2[] =
{
    0x0E, 0x82, 0x0B, 0x7F, 0x49, 0x2E, 0x06, 0x41, 0x4A, 0x66, 0xED, 0xB8, 0x2B, 0x7C, 0x1B, 0xAB,
    0xB5, 0xEC, 0xB3, 0x67, 0x30, 0xCC, 0xBD, 0x69, 0x29, 0x99, 0xD5, 0x33, 0x82, 0xE8, 0x95, 0x98,
    0x01, 0x69, 0x27, 0x30, 0xB2, 0x5C, 0xD3, 0x5F, 0x71, 0xC8, 0x94, 0x2E, 0x5C, 0x4D, 0x5A, 0xCA,
    0x80, 0x2B, 0x3A, 0x13, 0xEC, 0x92, 0x84, 0x6C
};

static const uint8_t fcaZ3[] =
{
    0xC0, 0x83, 0x55, 0x21, 0x78, 0xB0, 0xAC, 0x17, 0x3E, 0xF5, 0xCB, 0xF9, 0xF5, 0xB6, 0x16, 0xD7,
    0x70, 0x64, 0x49, 0x28, 0xC2, 0x01, 0xF1, 0xBB, 0x7A, 0xA3, 0xDA, 0x1C, 0x89, 0x6C, 0x52, 0x0F,
    0x3C, 0x4D, 0x94, 0xEB, 0xA4, 0xC8, 0xAD, 0xCB, 0x35, 0x33, 0x6E, 0x55, 0x58, 0x25, 0xCD, 0x8A,
    0xB3, 0x10, 0xF1, 0x6A, 0x7A, 0xC7, 0xDB, 0xAC
};

static const uint8_t fcaX1[] =
{
    0x85, 0xF2, 0x74, 0x78, 0xDF, 0x00, 0x7E, 0x15, 0xD1, 0x01, 0xDB, 0x07, 0xE3, 0x59, 0x2E, 0x0A,
    0xEC, 0x73, 0xCF, 0xEC, 0x96, 0x0C, 0x16, 0x70, 0xD0, 0xA3, 0xDE, 0xF2, 0xBB, 0xC2, 0xE9, 0x14,
    0x5F, 0xB3, 0x46, 0xDC, 0xA9, 0xDB, 0xBB, 0xE0, 0x4A, 0x22, 0x9C, 0x9E, 0x12, 0x84, 0xBB, 0x67,
    0x0C, 0x3F, 0x5D, 0x5B, 0xA9, 0x64, 0x73, 0xE8
};

static const uint8_t fcaX2[] =
{
    0x8E, 0xCC, 0xBB, 0x99, 0x0D, 0xF9, 0x42, 0x99, 0x09, 0x2A, 0xD4, 0xF9, 0x96, 0x4F, 0x17, 0x0E,
    0xE3, 0xBE, 0x6A, 0x79, 0xA8, 0xEA, 0xA2, 0xCC, 0x5B, 0xD0, 0x1D, 0x06, 0xAE, 0x98, 0xC1, 0xDB,
    0xF4, 0x85, 0x17, 0xA8, 0x1B, 0xF9, 0xB0, 0x44, 0x31, 0x2A, 0x42, 0x1B, 0x2B, 0x5C, 0x1A, 0x2B,
    0xE6, 0xD4, 0xA1, 0x23, 0x0A, 0x9C, 0xB8, 0x77
};


static const uint8_t fcaX3[] =
{
    0x65, 0x8F, 0x92, 0xEB, 0xBD, 0x00, 0x3F, 0x33, 0x02, 0x76, 0xB3, 0x6D, 0xA5, 0x26, 0xF8, 0x29,
    0x07, 0x17, 0x0B, 0xEF, 0x76, 0x64, 0xF0, 0xD9, 0xF3, 0xD6, 0x20, 0x24, 0xC4, 0x82, 0x0C, 0xAB,
    0x8B, 0x3C, 0xBF, 0x84, 0xFF, 0xFF, 0xC8, 0x3A, 0x21, 0x57, 0xFD, 0xE4, 0x49, 0x21, 0x47, 0x85,
    0xFF, 0xBB, 0x24, 0x85, 0x72, 0x26, 0x19, 0x76
};

static const uint8_t fcaK[] =
{
    0xA4, 0xF7, 0xCE, 0x86, 0xB4, 0x8B, 0xB1, 0xBF, 0x59, 0xC3, 0xE0, 0x69, 0xE6, 0x89, 0x9B, 0x5C,
    0xD0, 0x1B, 0xF9, 0xF7, 0xE7, 0xBC, 0xCC, 0x0A, 0x3C, 0xB3, 0x37, 0x17, 0x3E, 0xAC, 0x91, 0x38,
    0x2D, 0xCC, 0xA4, 0xCE, 0xF6, 0x4F, 0x2B, 0x2E, 0x92, 0xDD, 0x44, 0x6E, 0x50, 0x50, 0x52, 0xBF,
    0x44, 0x33, 0x0F, 0xE8, 0x0F, 0x78, 0x91, 0x2F
};

/******************************************************************************
 * Function:    HSE_BurmesterDesmedt_Example
 * Description: Example of Burmester Desmedt sent Synchronously
 *
 *****************************************************************************/
/******************************************************************************
 * Function:    BDComputeSecondPublicKey
 * Description: Burmester Desmudt Protocol compute Public Key
 *****************************************************************************/
static hseSrvResponse_t BDComputeSecondPublicKey( hseKeyHandle_t deviceKeyHandle, hseKeyHandle_t pubKeyHandle )
{
    hseSrvResponse_t hseResponse;
    uint8_t localMuIf = 0U;
    uint8_t localMuChannelIdx = 1U;

    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[localMuIf][localMuChannelIdx];
    hseBurmesterDesmedtSrv_t *pBdReq;
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    pHseSrvDesc->srvId = HSE_SRV_ID_BURMESTER_DESMEDT;
    pBdReq = &(pHseSrvDesc->hseSrv.burmesterDesmedtReq);

    pBdReq->bdStep                = HSE_BD_STEP_COMPUTE_SECOND_PUBLIC_KEY;
    pBdReq->deviceKeyHandle       = deviceKeyHandle;
    pBdReq->pubKeyHandle          = pubKeyHandle;

    hseResponse = HSE_Send(localMuIf, localMuChannelIdx, gSyncTxOption, pHseSrvDesc);

    return hseResponse;
}
/******************************************************************************
* Function:    HSE_BurmesterDesmedt_Example
* Description: Example of Burmester Desmedt sent Synchronously
*
* *****************************************************************************/
hseSrvResponse_t HSE_BurmesterDesmedt_Example(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    hseKeyInfo_t keyInfo = {0};
    uint8_t sharedSecret[448];
    uint16_t sharedSecretLen = 56;
    hseKeyHandle_t hmacKeyHandle = HSE_DEMO_RAM_HMAC_KEY0;
    hseKeyHandle_t BDSharedSecretKeyHandle = HSE_INVALID_KEY_HANDLE;
    srvResponse = ParseKeyCatalogs();
    if(HSE_SRV_RSP_OK !=srvResponse)
    goto exit;
    /* load P224 curve into HSE */
    srvResponse = LoadEccUserCurve(HSE_EC_USER_CURVE1,
                                 224, 224,
                                 ECC_SECP224R1_CURVEPARAM_A, ECC_SECP224R1_CURVEPARAM_B,
                                 ECC_SECP224R1_BASEPOINT, ECC_SECP224R1_ORDER_N,
                                 ECC_SECP224R1_PRIME_P);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /**************************** Node 1 Test ********************************************/

    /* Load Node 1's Device Key */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE,
                HSE_KEY_TYPE_ECC_PAIR,
                (HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE),
                HSE_EC_USER_CURVE1,
                224,
                fcaZ1,
                fcaA1
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Load Z3 in Slot 0 (Node 2 Predecessor) */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE,
                HSE_KEY_TYPE_ECC_PUB,
                HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                HSE_EC_USER_CURVE1,
                224,
                fcaZ3,
                NULL
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Load Z2 in Slot 1 (Node 2 Successor) */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE_SLOT1,
                HSE_KEY_TYPE_ECC_PUB,
                HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                HSE_EC_USER_CURVE1,
                224,
                fcaZ2,
                NULL
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Perform BD Step 1 */
    srvResponse = BDComputeSecondPublicKey(HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE, HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Load X2 in Slot 3 */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE_SLOT3,
                HSE_KEY_TYPE_ECC_PUB,
                HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                HSE_EC_USER_CURVE1,
                224,
                fcaX2,
                NULL
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Perform BD Step 2 */
    srvResponse = BDComputeSharedSecret(HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE, HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE, 3U, &BDSharedSecretKeyHandle ,RAM_KEY,224);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    srvResponse = GetKeyInfo(BDSharedSecretKeyHandle, &keyInfo);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    ASSERT(448 == keyInfo.keyBitLen);

    /* Copy the shared secret to an exportable HMAC slot */
    keyInfo.keyType = HSE_KEY_TYPE_HMAC;
    keyInfo.keyFlags = (HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE);
    keyInfo.keyBitLen = 448;
    srvResponse = HSEKeyDeriveExtractKeyReq(BDSharedSecretKeyHandle, 0U,&hmacKeyHandle, RAM_KEY, keyInfo );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    srvResponse = ExportPlainSymKeyReq(HSE_DEMO_RAM_HMAC_KEY0, &keyInfo, sharedSecret, &sharedSecretLen);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    ASSERT(56U == sharedSecretLen);
    ASSERT(0U == memcmp(sharedSecret, fcaK, 56U));

    /**************************** Node 2 Test ********************************************/

    /* Load Node 1's Device Key */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE,
                HSE_KEY_TYPE_ECC_PAIR,
                (HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE),
                HSE_EC_USER_CURVE1,
                224,
                fcaZ2,
                fcaA2
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Load Z3 in Slot 0 (Node 2 Predecessor) */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE,
                HSE_KEY_TYPE_ECC_PUB,
                HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                HSE_EC_USER_CURVE1,
                224,
                fcaZ1,
                NULL
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Load Z2 in Slot 1 (Node 2 Successor) */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE_SLOT1,
                HSE_KEY_TYPE_ECC_PUB,
                HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                HSE_EC_USER_CURVE1,
                224,
                fcaZ3,
                NULL
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Perform BD Step 1 */
    srvResponse = BDComputeSecondPublicKey(HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE, HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Load X2 in Slot 3 */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE_SLOT3,
                HSE_KEY_TYPE_ECC_PUB,
                HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                HSE_EC_USER_CURVE1,
                224,
                fcaX3,
                NULL
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Perform BD Step 2 */
    srvResponse = BDComputeSharedSecret(HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE, HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE, 3U, &BDSharedSecretKeyHandle,RAM_KEY,224);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    srvResponse = GetKeyInfo(BDSharedSecretKeyHandle, &keyInfo);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    ASSERT(448 == keyInfo.keyBitLen);

    /* Copy the shared secret to an exportable HMAC slot */
    keyInfo.keyType = HSE_KEY_TYPE_HMAC;
    keyInfo.keyFlags = (HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE);
    keyInfo.keyBitLen = 448;
    srvResponse = HSEKeyDeriveExtractKeyReq(BDSharedSecretKeyHandle, 0U, &hmacKeyHandle, RAM_KEY,keyInfo );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    srvResponse = ExportPlainSymKeyReq(HSE_DEMO_RAM_HMAC_KEY0, &keyInfo, sharedSecret, &sharedSecretLen);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    ASSERT(56U == sharedSecretLen);
    ASSERT(0U == memcmp(sharedSecret, fcaK, 56U));

    /**************************** Node 3 Test ********************************************/

    /* Load Node 1's Device Key */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE,
                HSE_KEY_TYPE_ECC_PAIR,
                (HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE),
                HSE_EC_USER_CURVE1,
                224,
                fcaZ3,
                fcaA3
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Load Z3 in Slot 0 (Node 2 Predecessor) */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE,
                HSE_KEY_TYPE_ECC_PUB,
                HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                HSE_EC_USER_CURVE1,
                224,
                fcaZ2,
                NULL
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Load Z2 in Slot 1 (Node 2 Successor) */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE_SLOT1,
                HSE_KEY_TYPE_ECC_PUB,
                HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                HSE_EC_USER_CURVE1,
                224,
                fcaZ1,
                NULL
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Perform BD Step 1 */
    srvResponse = BDComputeSecondPublicKey(HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE, HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Load X2 in Slot 3 */
    srvResponse = ImportEccKeyReq(
                HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE_SLOT3,
                HSE_KEY_TYPE_ECC_PUB,
                HSE_KF_USAGE_EXCHANGE | HSE_KF_ACCESS_EXPORTABLE,
                HSE_EC_USER_CURVE1,
                224,
                fcaX1,
                NULL
            );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Perform BD Step 2 */
    srvResponse = BDComputeSharedSecret(HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE, HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE, 3U, &BDSharedSecretKeyHandle,RAM_KEY,224);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    srvResponse = GetKeyInfo(BDSharedSecretKeyHandle, &keyInfo);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    ASSERT(448 == keyInfo.keyBitLen);

    /* Copy the shared secret to an exportable HMAC slot */
    keyInfo.keyType = HSE_KEY_TYPE_HMAC;
    keyInfo.keyFlags = (HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE);
    keyInfo.keyBitLen = 448;
    srvResponse = HSEKeyDeriveExtractKeyReq( BDSharedSecretKeyHandle, 0U, &hmacKeyHandle,RAM_KEY,keyInfo );
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    srvResponse = ExportPlainSymKeyReq(HSE_DEMO_RAM_HMAC_KEY0, &keyInfo, sharedSecret, &sharedSecretLen);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    ASSERT(56U == sharedSecretLen);
    ASSERT(0U == memcmp(sharedSecret, fcaK, 56U));

exit:
    return srvResponse;
}
#endif
