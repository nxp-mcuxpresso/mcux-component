/*
 * @filename hse_aead_example.c
 * @brief Source file for AEAD example in crypto services
 * @details Contains the API's for demonstrating the use of AEAD crypto service in DemoApp
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
#include "hse_aead_example.h"

#ifdef HSE_SPT_AEAD

/*==================================================================================================
                                 LOCAL VARIABLE
==================================================================================================*/

/*
 * @brief Authenticated Encryption with Associated data(AEAD) keys for CCM algorithm
 */
static const uint8_t Aead_Ccm_Key[16] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f};
/*
 * @brief AEAD Nounce/Initiaisation Vector(IV) values for CCM algorithm
 */
static const uint8_t Aead_Ccm_iv[7] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
/*
 * @brief AEAD plain text value for CCM algorithm
 */
static const uint8_t Aead_Ccm_plain[] = {0x20, 0x21, 0x22, 0x23};
/*
 * @brief AEAD Authenticated data(AD) value for CCM algorithm
 */
static const uint8_t Aead_Ccm_aad[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
/*
 * @brief AEAD expected tag values after encryption of plaintext via CCM algorithm
 */
static uint8_t Aead_Ccm_tag_expected[] = {0x4d, 0xac, 0x25, 0x5d};
/*
 * @brief AEAD expected Cipher value after encryption of plaintext via CCM algorithm
 */
static const uint8_t Aead_Ccm_cipher_expected[] = {0x71, 0x62, 0x01, 0x5b};
/*
 * @brief AEAD keys for GCM algorithm
 */
static const uint8_t Aead_Gcm_Key[16]  =
{ 0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c, 0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08 };
/*
 * AEAD Nounce/Initialisation Vector(IV) value for GCM algorithm
 */
static const uint8_t Aead_Gcm_iv[12] =
{ 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88 };
/*
 * @brief AEAD plain text value for GCM algorithm
 */
static const uint8_t Aead_Gcm_plain[] =
{ 0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5, 0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
        0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda, 0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a,
        0x72, 0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53, 0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6,
        0xb5, 0x25, 0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57, 0xba, 0x63, 0x7b, 0x39 };
/*
 * @brief AEAD Authenticated data(AD) value for CCM algorithm
 */
static const uint8_t Aead_Gcm_aad[] =
{ 0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
        0xab, 0xad, 0xda, 0xd2 };
/*
 * @brief AEAD expected tag values after encryption of plaintext via GCM algorithm
 */
static uint8_t Aead_Gcm_tag_expected[] =
{ 0x5b, 0xc9, 0x4f, 0xbc, 0x32, 0x21, 0xa5, 0xdb, 0x94, 0xfa, 0xe9, 0x5a, 0xe7, 0x12, 0x1a, 0x47 };
/*
 * @brief AEAD expected Cipher value after encryption of plaintext via GCM algorithm
 */
static const uint8_t Aead_Gcm_cipher_expected[] =
{ 0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24, 0x4b, 0x72, 0x21, 0xb7, 0x84, 0xd0, 0xd4, 0x9c,
        0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0, 0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1,
        0x2e, 0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93, 0x1c, 0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84,
        0xaa, 0x05, 0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97, 0x3d, 0x58, 0xe0, 0x91 };

/*
 * ============================================================================
 *                               FUNCTIONS DEFINATIONS
 * ============================================================================
*/

/*
 * @brief demonstrate the CCM algorithm of AEAD
 * @description This API will generate the keys for AEAD
 *              and do encryption/decryption via AEAD CCM algo
 *              For Encryption:
 *               it would be needing following Inputs:
 *              1) AES Keys
 *              2) Plaintext
 *              3) Authenticated data(AD)
 *              4) Nounce/Initialization Vector values
 *              5) Expected tag
 *              6) Empty array to store the encrypted cipher text
 *               Output would be:
 *              1) Ciphertext
 *
 *              For Decryption
 *               it would be needing following inputs:
 *              1) AES Keys
 *              2) Ciphertext
 *              3) Authenticated data(AD)
 *              4) Nounce/Initialization Vector values
 *              5) Expected tag
 *              6) Empty array to store the decrypted text: Plaintext
 *               Output would be:
 *              1) Decrypted data that should be equal to plaintext that is send for encryption
 * @return servive response is returned
 *
 */
static hseSrvResponse_t HseAeadCCM()
{
    hseSrvResponse_t hseSrvResponse= HSE_SRV_RSP_GENERAL_ERROR;
    (void) hseSrvResponse;
    int resp;
    hseKeyInfo_t AeadCcmkeyinfo = {0};
    hseKeyHandle_t AeadCCMRamKeyHandle = HSE_INVALID_KEY_HANDLE;

    uint32_t AeadCcmEncryptOutputLength = 0;
    (void)AeadCcmEncryptOutputLength;
    uint8_t AeadCcmEncryptOutput[256] = {0};
    AeadCcmEncryptOutputLength = ARRAY_SIZE(AeadCcmEncryptOutput);


    uint32_t AeadCcmDecryptOutputLength = 0;
    (void)AeadCcmDecryptOutputLength;
    uint8_t AeadCcmDecryptOutput[256] = {0};
    AeadCcmDecryptOutputLength = ARRAY_SIZE(AeadCcmDecryptOutput);


    hseSrvResponse = LoadAesKey(&AeadCCMRamKeyHandle, FALSE, ARRAY_SIZE(Aead_Ccm_Key), Aead_Ccm_Key);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    hseSrvResponse =  GetKeyInfo(AeadCCMRamKeyHandle, &AeadCcmkeyinfo);
    if(HSE_SRV_RSP_OK != hseSrvResponse) //
    {
        goto Exit;
    }


    hseSrvResponse = AesCcmEncrypt(AeadCCMRamKeyHandle, ARRAY_SIZE(Aead_Ccm_iv), Aead_Ccm_iv,  ARRAY_SIZE(Aead_Ccm_aad), Aead_Ccm_aad,
                                 ARRAY_SIZE(Aead_Ccm_plain),  Aead_Ccm_plain, ARRAY_SIZE(Aead_Ccm_tag_expected), Aead_Ccm_tag_expected,AeadCcmEncryptOutput);
    resp = memcmp(AeadCcmEncryptOutput, Aead_Ccm_cipher_expected, ARRAY_SIZE(Aead_Ccm_cipher_expected));
    ASSERT(0 == resp);

    hseSrvResponse = AesCcmDecrypt(AeadCCMRamKeyHandle, ARRAY_SIZE(Aead_Ccm_iv), Aead_Ccm_iv,  ARRAY_SIZE(Aead_Ccm_aad), Aead_Ccm_aad,
                                 ARRAY_SIZE(Aead_Ccm_plain),AeadCcmEncryptOutput, ARRAY_SIZE(Aead_Ccm_tag_expected), Aead_Ccm_tag_expected, AeadCcmDecryptOutput);
    resp = memcmp(AeadCcmDecryptOutput, Aead_Ccm_plain, ARRAY_SIZE(Aead_Ccm_plain));
    ASSERT(0 == resp);

    Exit:
    return hseSrvResponse;


}

/*
 * @brief demonstrate the GCM algorithm of AEAD
 * @description This API will generate the keys for AEAD
 *              and do encryption/decryption via AEAD GCM algo
 *              For Encryption:
 *               it would be needing following Inputs:
 *              1) AES Keys
 *              2) Plaintext
 *              3) Authenticated data(AD)
 *              4) Nounce/Initialization Vector values
 *              5) Expected tag
 *              6) Empty array to store the encrypted cipher text
 *               Output would be:
 *              1) Ciphertext
 *
 *              For Decryption
 *               it would be needing following inputs:
 *              1) AES Keys
 *              2) Ciphertext
 *              3) Authenticated data(AD)
 *              4) Nounce/Initialization Vector values
 *              5) Expected tag
 *              6) Empty array to store the decrypted text: Plaintext
 *               Output would be:
 *              1) Decrypted data that should be equal to plaintext that is send for encryption
 * @return servive response is returned
 *
 */

static hseSrvResponse_t HseAeadGCM()
{
    hseSrvResponse_t hseSrvResponse= HSE_SRV_RSP_GENERAL_ERROR;
    (void) hseSrvResponse;
    int resp;
    hseKeyInfo_t AeadGcmkeyinfo = {0};
    hseKeyHandle_t AeadGcmRamKeyHandle = HSE_INVALID_KEY_HANDLE;

    uint32_t AeadGcmEncryptOutputLength = 0;
    (void)AeadGcmEncryptOutputLength;
    uint8_t AeadGcmEncryptOutput[256] = {0};
    AeadGcmEncryptOutputLength = ARRAY_SIZE(AeadGcmEncryptOutput);


    uint32_t AeadGcmDecryptOutputLength = 0;
    (void)AeadGcmDecryptOutputLength;
    uint8_t AeadGcmDecryptOutput[256] = {0};
    AeadGcmDecryptOutputLength = ARRAY_SIZE(AeadGcmDecryptOutput);


    hseSrvResponse = LoadAesKey(&AeadGcmRamKeyHandle, FALSE, ARRAY_SIZE(Aead_Gcm_Key), Aead_Gcm_Key);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    hseSrvResponse =  GetKeyInfo(AeadGcmRamKeyHandle, &AeadGcmkeyinfo);
    if(HSE_SRV_RSP_OK != hseSrvResponse) //
    {
       goto Exit;
    }

    hseSrvResponse = AesGcmEncrypt(AeadGcmRamKeyHandle, ARRAY_SIZE(Aead_Gcm_iv), Aead_Gcm_iv,  ARRAY_SIZE(Aead_Gcm_aad), Aead_Gcm_aad,
                                ARRAY_SIZE(Aead_Gcm_plain),  Aead_Gcm_plain, ARRAY_SIZE(Aead_Gcm_tag_expected), Aead_Gcm_tag_expected,AeadGcmEncryptOutput,0);
    resp = memcmp(AeadGcmEncryptOutput, Aead_Gcm_cipher_expected, ARRAY_SIZE(Aead_Gcm_cipher_expected));
    ASSERT(0 == resp);

    hseSrvResponse = AesGcmDecrypt(AeadGcmRamKeyHandle, ARRAY_SIZE(Aead_Gcm_iv), Aead_Gcm_iv,  ARRAY_SIZE(Aead_Gcm_aad), Aead_Gcm_aad,
                                ARRAY_SIZE(Aead_Gcm_plain),AeadGcmEncryptOutput, ARRAY_SIZE(Aead_Gcm_tag_expected), Aead_Gcm_tag_expected, AeadGcmDecryptOutput,0);
    resp = memcmp(AeadGcmDecryptOutput, Aead_Gcm_plain, ARRAY_SIZE(Aead_Gcm_plain));
    ASSERT(0 == resp);

    Exit:
    return hseSrvResponse;

}
/*
 * @brief This API would be directly used in hse_crypto file
 *        for demonstrating the AEAD algos(CCM and GCM)
 * @return service response would be returned
 */
hseSrvResponse_t HseAeadExamples()
{
    AeadExampleStarted AeadExampleStarted_k = AEAD_STARTED_NONE;
    AeadExampleStatus AeadExampleStatus_k = AEAD_EXECUTED_NONE;

    hseSrvResponse_t hseSrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    (void) hseSrvResponse;

    AeadExampleStarted_k |= AEAD_CCM_EXAMPLE_STARTED;
    hseSrvResponse = HseAeadCCM();
    if(HSE_SRV_RSP_OK != hseSrvResponse)
    {
        goto Exit;
    }
    AeadExampleStatus_k |= AEAD_CCM_EXAMPLE_SUCCESSFUL;

    AeadExampleStarted_k |= AEAD_GCM_EXAMPLE_STARTED;
    hseSrvResponse = HseAeadGCM();
    if(HSE_SRV_RSP_OK != hseSrvResponse)
    {
        goto Exit;
    }
    AeadExampleStatus_k |= AEAD_GCM_EXAMPLE_SUCCESSFUL;

    ASSERT((uint8_t)AeadExampleStarted_k == (uint8_t)AeadExampleStatus_k);

    Exit:
    return hseSrvResponse;

}
#endif
