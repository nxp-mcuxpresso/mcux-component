/*
 * hse_sessionkeys_example.c
 *
  @brief Source file for Session keys in crypto services
 * @details Contains the API's for demonstrating the use of Session keys crypto service in DemoApp
 */
/*--------------------------------------------------------------------------*/
/* Copyright 2022 NXP                                                       */
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
#include "hse_sessionkeys_example.h"

#ifdef HSE_SPT_KEY_DERIVE
/*==================================================================================================
                               LOCAL CONSTANTS
==================================================================================================*/


static const uint8_t session_key_message[] =
        "The surest way to corrupt a youth is to instruct him to hold in higher esteem those who think alike than those who think differently - Friedrich Nietzsche 2020";


static const uint8_t KdfSP800_108_FixedInfo_1[] = { 0x5a, 0xc3, 0x73, 0xd4, 0x2e, 0xd9, 0x24, 0x27, 0xd8,
0xff, 0x6c, 0xff, 0xf7, 0xea, 0xe1, 0x3d, 0x66, 0xd3, 0xc7, 0xe5, 0x36, 0xcc, 0x74, 0x98, 0x59,
0xe2, 0xa4, 0x9e, 0x3e, 0xea, 0x2a, 0xd8, 0x46, 0xc9, 0xfb, 0xb7, 0xdd, 0xd9, 0x9a, 0x1e, 0x6a,
0x54, 0xa8, 0x9a, 0x87, 0xdb, 0x98, 0xdb, 0x6b, 0x82, 0x29, 0xf5, 0x77, 0xb5, 0x52, 0xe0, 0x9a, 0xee,
0xd5, 0xe6 };

//Fixed Info for KDF example
static const uint8_t KdfSP800_108_SrcKey_1[] = { 0x00, 0x93, 0x00, 0xd2, 0x65, 0xd1, 0xf1, 0xb2, 0x8b, 0x50, 0x5d, 0xcc, 0xc1, 0x62, 0xf4, 0xf8 };


#ifdef HSE_SPT_KDF_SP800_108
  static hseKeyInfo_t KdfSP800_108_SECRET_KEY_INFO_1 =
{
    .keyType = HSE_KEY_TYPE_AES,
    .keyBitLen = 128UL,
    .keyFlags = (HSE_KF_USAGE_DERIVE | HSE_KF_ACCESS_EXPORTABLE)
};

static hseKey_t KdfSP800_108_SECRET_KEY_1 = {
    .keyHandle = HSE_INVALID_KEY_HANDLE,
    .pKeyInfo = &KdfSP800_108_SECRET_KEY_INFO_1,
    .keyValue = {
        .pKey2 = (uint8_t *)KdfSP800_108_SrcKey_1,
        .keyLen2 = ARRAY_SIZE(KdfSP800_108_SrcKey_1),
    },
};
static hseKeyImportParams_t KdfSP800_108_ImportSecret_1 =
{
    .pKey = &KdfSP800_108_SECRET_KEY_1
};
static hseKdfSP800_108Scheme_t KdfSP800_108_Scheme_1 =
{
    .mode = HSE_KDF_SP800_108_COUNTER,
    .counterByteLength = HSE_KDF_SP800_108_COUNTER_LEN_DEFAULT,
    .kdfCommon = {
        .srcKeyHandle = HSE_INVALID_KEY_HANDLE,
        .targetKeyHandle = HSE_INVALID_KEY_HANDLE,
        .keyMatLen = BITS_TO_BYTES(512UL),
        .kdfPrf = HSE_KDF_PRF_CMAC,
        .pInfo = (HOST_ADDR)KdfSP800_108_FixedInfo_1,
        .infoLength = ARRAY_SIZE(KdfSP800_108_FixedInfo_1)
    }
};
#endif

static hseKeyHandle_t srcKey = HSE_INVALID_KEY_HANDLE;
static hseKeyHandle_t targetSharedSecretKey = HSE_INVALID_KEY_HANDLE;

#ifdef HSE_SPT_GMAC
static const uint8_t iv[] =
{ 0xff, 0xbc, 0x51, 0x6a, 0x8f, 0xbe, 0x61, 0x52, 0xaa, 0x42, 0x8c, 0xdd, 0x80,
        0x0c, 0x06, 0x2d
};
#endif

/*
 * @brief ECC Pub key
 */
static const uint8_t eccP256PubKey[] =
{ 0xd6, 0x60, 0x62, 0x71, 0x13, 0x1e, 0x7e, 0x7e, 0x61, 0x7a, 0x81, 0xaa, 0x11,
        0xf0, 0x9e, 0x7e, 0xd5, 0x63, 0x11, 0x82, 0x88, 0x23, 0x36, 0x7a, 0x86,
        0x9b, 0x45, 0x40, 0x40, 0xb3, 0xf9, 0x05, 0xcf, 0x48, 0x97, 0x76, 0x61,
        0x31, 0xaa, 0x8b, 0x7f, 0x80, 0x45, 0x3a, 0x15, 0xbf, 0x90, 0xf7, 0x51,
        0x78, 0x78, 0x57, 0x9d, 0x5a, 0x4f, 0x97, 0x3a, 0xea, 0x5b, 0xb1, 0x15,
        0x42, 0xe0, 0x7f
};
/*
 * ============================================================================
 *                             FUNCTIONS DEFINITION
 * ============================================================================
*/
#ifdef HSE_SPT_KDF_SP800_108
static hseSrvResponse_t KdfSP800_108Test
(
    hseKeyImportParams_t *pImportSecretParams,
    hseKdfSP800_108Scheme_t *pKdfScheme
)
{
    hseSrvResponse_t hseResponse;

    pImportSecretParams->authParams.pKeyHandle = NULL;
    pImportSecretParams->cipherParams.pKeyHandle = NULL;
    hseResponse = ImportKeyReq(pImportSecretParams);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    hseResponse = HSEKdfSP800_108Req(pKdfScheme);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    exit:
    return hseResponse;
}
static hseSrvResponse_t KdfSP800_108ReqTest()
{
    hseSrvResponse_t hseResponse;
    HKF_AllocKeySlot(RAM_KEY, KdfSP800_108_ImportSecret_1.pKey->pKeyInfo->keyType,
                     KdfSP800_108_ImportSecret_1.pKey->pKeyInfo->keyBitLen, &srcKey);
    HKF_AllocKeySlot(RAM_KEY, HSE_KEY_TYPE_SHARED_SECRET, KdfSP800_108_Scheme_1.kdfCommon.keyMatLen * 8U, &targetSharedSecretKey);
    KdfSP800_108_ImportSecret_1.pKey->keyHandle = srcKey;
    KdfSP800_108_Scheme_1.kdfCommon.srcKeyHandle = srcKey;
    KdfSP800_108_Scheme_1.kdfCommon.targetKeyHandle = targetSharedSecretKey;
    hseResponse = KdfSP800_108Test(&KdfSP800_108_ImportSecret_1, &KdfSP800_108_Scheme_1);
    goto exit;
    exit:
    return hseResponse;
}
#endif

/******************************************************************************
 * Function:    HSE_SessionKeys_Example
 * Description: Key Exchange example using HSE
 *              ECC key generation, ECC key import, ephemeral ECDH,
 *              Key derivation (KDF SP800_108),
 *              Key extraction - AES, HMAC keys from derived key material
 *****************************************************************************/
#ifdef HSE_SPT_KEY_DERIVE
hseSrvResponse_t HSE_SessionKeys_Example(void)
{
    hseSrvResponse_t hseResponse;
    hseKeyHandle_t eccRAMKeyHandle = HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE;
    #if defined(HSE_SPT_CLASSIC_DH)
    hseKeyHandle_t DHSharedSecretRAMKeyHandle = HSE_DEMO_DH_SHARED_SECRET_HANDLE;
    #endif
    hseKeyHandle_t AESDerivedKeyInfoHandle1 = HSE_DEMO_RAM_AES256_KEY1;
    hseKeyHandle_t AESDerivedKeyInfoHandle0 = HSE_DEMO_RAM_AES256_KEY0;

    hseResponse= ParseKeyCatalogs();
    if(HSE_SRV_RSP_OK !=hseResponse)
    goto exit;
    //Generate an ECC key pair in RAM (ephemeral ECDH)
    hseResponse = GenerateEccKey(
            &eccRAMKeyHandle,
            RAM_KEY,
            HSE_EC_SEC_SECP256R1,
            HSE_KF_USAGE_EXCHANGE);
    if(HSE_SRV_RSP_OK != hseResponse)
        goto exit;

    //Import the peer public key
    hseResponse = ImportEccKeyReq(
            HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE, HSE_KEY_TYPE_ECC_PUB,
            HSE_KF_USAGE_EXCHANGE, HSE_EC_SEC_SECP256R1,
            KeyBitLen(HSE_EC_SEC_SECP256R1), eccP256PubKey, NULL);
    if(HSE_SRV_RSP_OK != hseResponse)
        goto exit;

    #if defined(HSE_SPT_CLASSIC_DH)
    //Compute DH Shared Secret (ECDH)
    hseResponse = DHSharedSecretCompute(
            HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE,
            HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE,
            &DHSharedSecretRAMKeyHandle,
            RAM_KEY,
            KeyBitLen(HSE_EC_SEC_SECP256R1));

       if(HSE_SRV_RSP_OK != hseResponse)
        goto exit;
    #endif

    #ifdef HSE_SPT_KDF_SP800_108
    hseResponse = KdfSP800_108ReqTest();
    if(HSE_SRV_RSP_OK != hseResponse)
    goto exit;

    //Extract from the derived key material 3 keys: 192-bits AES, 256-bits AES, HMAC
    {
        //Declare the information about the 192 bits AES key to be extracted
        hseKeyInfo_t aes192KeyInfo = {
            .keyType = HSE_KEY_TYPE_AES,      //Will generate an AES key
            .keyFlags = (HSE_KF_USAGE_ENCRYPT|HSE_KF_USAGE_DECRYPT),//Usage flags for this key - Encrypt/Decrypt
            .keyBitLen = 192U,                //192 bits key
        };
        //Declare the information about the HMAC key to be extracted
        hseKeyInfo_t hmacKeyInfo = {
            .keyType = HSE_KEY_TYPE_HMAC,                        //Will generate a HMAC key
            .keyFlags = (HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY), //Usage flags for this key - Sign/Verify
            .keyBitLen = 512U,                                   //512 bits key
        };
        //Declare the information about the 256 bits AES key to be extracted
        hseKeyInfo_t aes256KeyInfo = {
            .keyType = HSE_KEY_TYPE_AES,                             //Will generate an AES key
            .keyFlags = (HSE_KF_USAGE_ENCRYPT |HSE_KF_USAGE_DECRYPT| //Usage flags for this key - Encrypt/Decrypt/Sign/Verify - AEAD
                HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY),
            .keyBitLen = 256U,                                       //256 bits key
        };

        //Extract the 192 bits AES key from the beginning of the derived key
        hseResponse = HSEKeyDeriveExtractKeyReq
                (
                KdfSP800_108_Scheme_1.kdfCommon.targetKeyHandle,
                0U,
                &AESDerivedKeyInfoHandle1,
                RAM_KEY,
                aes192KeyInfo
                );
        if(HSE_SRV_RSP_OK != hseResponse)
            goto exit;
        //Extract the HMAC key from the remaining derived key material
        hseResponse = HSEKeyDeriveExtractKeyReq
                (
                        KdfSP800_108_Scheme_1.kdfCommon.targetKeyHandle,
                        0U,
                        &AESDerivedKeyInfoHandle0,
                        RAM_KEY,
                        hmacKeyInfo
                );
        if(HSE_SRV_RSP_OK != hseResponse)
            goto exit;

        //Extract the 256 bits AES key from the remaining derived key material
        hseResponse = HSEKeyDeriveExtractKeyReq
                (
                        KdfSP800_108_Scheme_1.kdfCommon.targetKeyHandle,
                        BITS_TO_BYTES(aes192KeyInfo.keyBitLen),
                        &AESDerivedKeyInfoHandle1,
                        RAM_KEY,
                        aes256KeyInfo
                );
        if(HSE_SRV_RSP_OK != hseResponse)
            goto exit;
    #endif

    }

    //Keys sanity checks: AES encrypt/decrypt, HMAC sign/verify
    {
        uint8_t tag[64] = {0U};
        uint32_t tagLen = 64UL;
        uint8_t cipherText[NUM_OF_ELEMS(session_key_message)] = {0U};
        uint8_t plainText[NUM_OF_ELEMS(session_key_message)] = {0U};

        //AES key sanity check - Encrypt/Decrypt
        hseResponse = AesEncrypt(
                HSE_DEMO_RAM_AES256_KEY1,
                HSE_CIPHER_BLOCK_MODE_CBC,
                iv,
                NUM_OF_ELEMS(session_key_message),
                session_key_message,
                cipherText,
                0U );
        if(HSE_SRV_RSP_OK != hseResponse)
            goto exit;

        hseResponse = AesDecrypt(
                HSE_DEMO_RAM_AES256_KEY1,
                HSE_CIPHER_BLOCK_MODE_CBC,
                iv,
                NUM_OF_ELEMS(cipherText),
                cipherText,
                plainText,
                0U );
        if( (HSE_SRV_RSP_OK != hseResponse) ||
            (0 != memcmp(session_key_message, plainText, NUM_OF_ELEMS(session_key_message))) )
            goto exit;

        //Alternatively using only 2 calls - AEAD GCM Encrypt/Decrypt
        tagLen = 16UL;
        memset(tag, 0, 64);
        memset(cipherText, 0, NUM_OF_ELEMS(session_key_message));
        memset(plainText, 0, NUM_OF_ELEMS(session_key_message));

        hseResponse = AesGcmEncrypt(
                HSE_DEMO_RAM_AES256_KEY1,
                NUM_OF_ELEMS(iv),
                iv,
                0UL,
                NULL,
                NUM_OF_ELEMS(session_key_message),
                session_key_message,
                tagLen,
                tag,
                cipherText,
                0U );
        if(HSE_SRV_RSP_OK != hseResponse)
            goto exit;

        hseResponse = AesGcmDecrypt(
                HSE_DEMO_RAM_AES256_KEY1,
                NUM_OF_ELEMS(iv),
                iv,
                0UL,
                NULL,
                NUM_OF_ELEMS(cipherText),
                cipherText,
                tagLen,
                tag,
                plainText,
                0U );
        if( (HSE_SRV_RSP_OK != hseResponse) ||
            (0 != memcmp(session_key_message, plainText, NUM_OF_ELEMS(session_key_message))) )
            goto exit;
    }

exit:
    return hseResponse;
}
#endif

#endif
