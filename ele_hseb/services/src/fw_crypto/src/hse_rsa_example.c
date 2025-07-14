/*
 * @file hse_rsa_example.c
 * @brief Source file for RSA examples in Crypto services
 * @details Contains the API's for demonstrating the use of RSA crypto services in Demo APP
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
#include "hse_rsa_example.h"

#ifdef HSE_SPT_RSA
/*==================================================================================================
                               LOCAL CONSTANTS
==================================================================================================*/

#if defined(HSE_SPT_RSAES_NO_PADDING)||defined(HSE_SPT_RSAES_OAEP)||defined(HSE_SPT_RSAES_PKCS1_V15)
/*
 * @brief Input message for RSA encryption
 */
SDK_ALIGN(static const uint8_t InputRSAMessageEnc[28], 8u) =
{ 0x8b, 0xba, 0x6b, 0xf8, 0x2a, 0x6c, 0x0f, 0x86, 0xd5, 0xf1, 0x75, 0x6e, 0x97, 0x95, 0x68, 0x70,
        0xb0, 0x89, 0x53, 0xb0, 0x6b, 0x4e, 0xb2, 0x05, 0xbc, 0x16, 0x94, 0xee };
#endif

/*
 * @brief Modulus value(n) for RSA keys
 */
const uint8_t ModulusEnc[256] =
{ 0xae, 0x45, 0xed, 0x56, 0x01, 0xce, 0xc6, 0xb8, 0xcc, 0x05, 0xf8, 0x03, 0x93, 0x5c, 0x67, 0x4d,
        0xdb, 0xe0, 0xd7, 0x5c, 0x4c, 0x09, 0xfd, 0x79, 0x51, 0xfc, 0x6b, 0x0c, 0xae, 0xc3, 0x13,
        0xa8, 0xdf, 0x39, 0x97, 0x0c, 0x51, 0x8b, 0xff, 0xba, 0x5e, 0xd6, 0x8f, 0x3f, 0x0d, 0x7f,
        0x22, 0xa4, 0x02, 0x9d, 0x41, 0x3f, 0x1a, 0xe0, 0x7e, 0x4e, 0xbe, 0x9e, 0x41, 0x77, 0xce,
        0x23, 0xe7, 0xf5, 0x40, 0x4b, 0x56, 0x9e, 0x4e, 0xe1, 0xbd, 0xcf, 0x3c, 0x1f, 0xb0, 0x3e,
        0xf1, 0x13, 0x80, 0x2d, 0x4f, 0x85, 0x5e, 0xb9, 0xb5, 0x13, 0x4b, 0x5a, 0x7c, 0x80, 0x85,
        0xad, 0xca, 0xe6, 0xfa, 0x2f, 0xa1, 0x41, 0x7e, 0xc3, 0x76, 0x3b, 0xe1, 0x71, 0xb0, 0xc6,
        0x2b, 0x76, 0x0e, 0xde, 0x23, 0xc1, 0x2a, 0xd9, 0x2b, 0x98, 0x08, 0x84, 0xc6, 0x41, 0xf5,
        0xa8, 0xfa, 0xc2, 0x6b, 0xda, 0xd4, 0xa0, 0x33, 0x81, 0xa2, 0x2f, 0xe1, 0xb7, 0x54, 0x88,
        0x50, 0x94, 0xc8, 0x25, 0x06, 0xd4, 0x01, 0x9a, 0x53, 0x5a, 0x28, 0x6a, 0xfe, 0xb2, 0x71,
        0xbb, 0x9b, 0xa5, 0x92, 0xde, 0x18, 0xdc, 0xf6, 0x00, 0xc2, 0xae, 0xea, 0xe5, 0x6e, 0x02,
        0xf7, 0xcf, 0x79, 0xfc, 0x14, 0xcf, 0x3b, 0xdc, 0x7c, 0xd8, 0x4f, 0xeb, 0xbb, 0xf9, 0x50,
        0xca, 0x90, 0x30, 0x4b, 0x22, 0x19, 0xa7, 0xaa, 0x06, 0x3a, 0xef, 0xa2, 0xc3, 0xc1, 0x98,
        0x0e, 0x56, 0x0c, 0xd6, 0x4a, 0xfe, 0x77, 0x95, 0x85, 0xb6, 0x10, 0x76, 0x57, 0xb9, 0x57,
        0x85, 0x7e, 0xfd, 0xe6, 0x01, 0x09, 0x88, 0xab, 0x7d, 0xe4, 0x17, 0xfc, 0x88, 0xd8, 0xf3,
        0x84, 0xc4, 0xe6, 0xe7, 0x2c, 0x3f, 0x94, 0x3e, 0x0c, 0x31, 0xc0, 0xc4, 0xa5, 0xcc, 0x36,
        0xf8, 0x79, 0xd8, 0xa3, 0xac, 0x9d, 0x7d, 0x59, 0x86, 0x0e, 0xaa, 0xda, 0x6b, 0x83, 0xbb };
/*
 * @brief Public Exponent(e) value for RSA keys
 */
static const uint8_t PublicExponentEnc[4] =
{ 0x00, 0x01, 0x00, 0x01 };

/*
 * ============================================================================
 *                             FUNCTIONS DEFINITION
 * ============================================================================
*/
#if defined(HSE_SPT_RSAES_NO_PADDING)
/*
 * @brief demonstrates the RSA NO-PADD algo
 * @description This API will do the encryption-decryption via RSA NO-PADD algo
 *              1) RSA-PAIR keys will be used, whose handle is being passed as function argument.
 *              2) Pair keys- will be having both public(n,e) and private RSA keys(n,d).
 *              3) Then Encryption of Input RSA message is performed using RSA-Nopadd algo
 *                 to get the encrypted ciphertext.
 *              4) For encryption public keys of RSA are used.
 *              5) Then decryption of encrypted ciphertext is performed using RSA-NOPADD algo
 *                 to get back the Input message.
 *              6) For decryption private keys of RSA are used.
 *@return HSE service response is returned
 */

static hseSrvResponse_t HSE_RSA_Nopadd_Example(hseKeyHandle_t hseKeyHandle)
{
    hseSrvResponse_t hseSrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    hseKeyInfo_t rsaNoPaddpairkeyinfo = {0};

    uint32_t RSA_NO_PADD_OUTPUT_LENGTH = 0;
    SDK_ALIGN(uint8_t RSA_NO_PADD_OUTPUT_CIPHER[256], 8u) = {0};
    RSA_NO_PADD_OUTPUT_LENGTH = ARRAY_SIZE(RSA_NO_PADD_OUTPUT_CIPHER);

    uint32_t RSA_NO_PADD_CIPHER_DECRYPT_LENGTH = 0;
    SDK_ALIGN(uint8_t RSA_NO_PADD_CIPHER_DECRYPT[256], 8u) = {0};
    RSA_NO_PADD_CIPHER_DECRYPT_LENGTH = ARRAY_SIZE(RSA_NO_PADD_CIPHER_DECRYPT);

    hseSrvResponse =  GetKeyInfo(hseKeyHandle, &rsaNoPaddpairkeyinfo);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    hseSrvResponse = RsaNoPaddEncrypt(hseKeyHandle, ARRAY_SIZE(InputRSAMessageEnc),
                 InputRSAMessageEnc, &RSA_NO_PADD_OUTPUT_LENGTH, RSA_NO_PADD_OUTPUT_CIPHER);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);


    hseSrvResponse = RsaNoPaddDecrypt(hseKeyHandle, ARRAY_SIZE(RSA_NO_PADD_OUTPUT_CIPHER),
        RSA_NO_PADD_OUTPUT_CIPHER, &RSA_NO_PADD_CIPHER_DECRYPT_LENGTH, RSA_NO_PADD_CIPHER_DECRYPT);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    return hseSrvResponse;
}
#endif

#if defined(HSE_SPT_RSAES_OAEP)
/*
 * @brief demonstrates the RSA OAEP algo
 * @description This API will do encryption-decryption via RSA OAEP algo
 *              1) RSA-PAIR keys will be used, whose handle is being passed as function argument.
 *              2) Pair keys- will be having both public(n,e) and private RSA keys(n,d).
 *              3) Then Encryption of Input RSA message is performed using RSA-OAEP algo
 *                 to get the encrypted ciphertext.
 *              4) For encryption public keys of RSA are used.
 *              5) Then decryption of encrypted ciphertext is performed using RSA-OAEP algo
 *                 to get back the Input message.
 *              6) For decryption private keys of RSA are used.
 *@return HSE service response is returned
 */

static hseSrvResponse_t HSE_RSA_Oaep_Example(hseKeyHandle_t hseKeyHandle)
{
    hseSrvResponse_t hseSrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    hseKeyInfo_t rsaOAEPpairkeyinfo = {0};

    uint32_t RSA_OAEP_ENCRYPT_OUTPUT_LENGTH = 0;
    uint8_t RSA_OAEP_ENCRYPT_OUTPUT_CIPHER[256] = {0};
    RSA_OAEP_ENCRYPT_OUTPUT_LENGTH = ARRAY_SIZE(RSA_OAEP_ENCRYPT_OUTPUT_CIPHER);

    uint32_t RSA_OAEP_CIPHER_DECRYPT_LENGTH = 0;
    uint8_t RSA_OAEP_CIPHER_DECRYPT[256] = {0};
    RSA_OAEP_CIPHER_DECRYPT_LENGTH = ARRAY_SIZE(RSA_OAEP_CIPHER_DECRYPT);

    hseSrvResponse =  GetKeyInfo(hseKeyHandle, &rsaOAEPpairkeyinfo);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    hseSrvResponse = RsaOaepEncrypt(HSE_HASH_ALGO_SHA2_256, 0, NULL,hseKeyHandle, ARRAY_SIZE(InputRSAMessageEnc),
                    InputRSAMessageEnc, &RSA_OAEP_ENCRYPT_OUTPUT_LENGTH, RSA_OAEP_ENCRYPT_OUTPUT_CIPHER);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    hseSrvResponse = RsaOaepDecrypt(HSE_HASH_ALGO_SHA2_256, 0, NULL,hseKeyHandle, ARRAY_SIZE(RSA_OAEP_ENCRYPT_OUTPUT_CIPHER),
            RSA_OAEP_ENCRYPT_OUTPUT_CIPHER, &RSA_OAEP_CIPHER_DECRYPT_LENGTH, RSA_OAEP_CIPHER_DECRYPT);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    return hseSrvResponse;
}
#endif

#if defined(HSE_SPT_RSAES_PKCS1_V15)
/*
 * @brief demonstrates the RSA PKCS1V15 algo
 * @description This API will do encryption-decryption via RSA PKCS1V15 algo
 *              1) RSA-PAIR keys will be used, whose handle is being passed as function argument.
 *              2) Pair keys- will be having both public(n,e) and private RSA keys(n,d).
 *              3) Then Encryption of Input RSA message is performed using RSA-PKCS1V15 algo
 *                 to get the encrypted ciphertext.
 *              4) For encryption public keys of RSA are used.
 *              5) Then decryption of encrypted ciphertext is performed using RSA-PKCS1V15 algo
 *                 to get back the Input message.
 *              6) For decryption private keys of RSA are used.
 *@return HSE service response is returned
 */
static hseSrvResponse_t HSE_RSA_Pkcs1V15_Example(hseKeyHandle_t hseKeyHandle)
{
    hseSrvResponse_t hseSrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    hseKeyInfo_t rsaPkcs1V15pairkeyinfo = {0};

    uint32_t RSA_Pkcs1V15_ENCRYPT_OUTPUT_LENGTH = 0;
    uint8_t RSA_Pkcs1V15_ENCRYPT_OUTPUT_CIPHER[256] = {0};
    RSA_Pkcs1V15_ENCRYPT_OUTPUT_LENGTH = ARRAY_SIZE(RSA_Pkcs1V15_ENCRYPT_OUTPUT_CIPHER);

    uint32_t RSA_Pkcs1V15_CIPHER_DECRYPT_LENGTH = 0;
    uint8_t RSA_Pkcs1V15_CIPHER_DECRYPT[256] = {0};
    RSA_Pkcs1V15_CIPHER_DECRYPT_LENGTH = ARRAY_SIZE(RSA_Pkcs1V15_CIPHER_DECRYPT);

    hseSrvResponse =  GetKeyInfo(hseKeyHandle, &rsaPkcs1V15pairkeyinfo);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    hseSrvResponse = RsaPkcs1V15Encrypt(hseKeyHandle, HSE_HASH_ALGO_SHA2_256, 0, NULL,
                     ARRAY_SIZE(InputRSAMessageEnc),InputRSAMessageEnc,&RSA_Pkcs1V15_ENCRYPT_OUTPUT_LENGTH,  RSA_Pkcs1V15_ENCRYPT_OUTPUT_CIPHER);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    hseSrvResponse = RsaPkcs1V15Decrypt(hseKeyHandle, HSE_HASH_ALGO_SHA2_256, 0, NULL,
                ARRAY_SIZE(RSA_Pkcs1V15_ENCRYPT_OUTPUT_CIPHER),RSA_Pkcs1V15_ENCRYPT_OUTPUT_CIPHER,&RSA_Pkcs1V15_CIPHER_DECRYPT_LENGTH,  RSA_Pkcs1V15_CIPHER_DECRYPT);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);

    return hseSrvResponse;
}
#endif

/*
 * @brief This API would be directly used in hse_crypto file
 *        for demonstrating the RSA algos (NO-PADD,OAEP,PKCS1V15)
 *        RSA Pair keys will be generated in this function and will be used in all above examples.
 *        By passing that as function argument.
 * @return HSE service response would be returned
 */
hseSrvResponse_t HSE_RSA_Examples(void)
{
    RsaExampleStatus RsaExampleStatus_k = RSA_EXECUTED_NONE;
    RsaExampleStarted RsaExampleStarted_k = RSA_STARTED_NONE;
    hseSrvResponse_t hseSrvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseKeyHandle_t rsapairHandle = HSE_INVALID_KEY_HANDLE;

    hseSrvResponse = GenerateRsaKeyPair(&rsapairHandle, 1, BYTES_TO_BITS(ARRAY_SIZE(ModulusEnc)),
                     ARRAY_SIZE
                     (PublicExponentEnc),PublicExponentEnc, NULL);

    #if defined(HSE_SPT_RSAES_NO_PADDING)
    RsaExampleStarted_k |= RSA_NOPADD_EXAMPLE_STARTED;
    hseSrvResponse = HSE_RSA_Nopadd_Example(rsapairHandle);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);
    RsaExampleStatus_k |= RSA_NOPADD_EXAMPLE_SUCCESSFUL;
    #endif

    #if defined(HSE_SPT_RSAES_OAEP)
    RsaExampleStarted_k |= RSA_OAEP_EXAMPLE_STARTED;
    hseSrvResponse = HSE_RSA_Oaep_Example(rsapairHandle);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);
    RsaExampleStatus_k |= RSA_OAEP_EXAMPLE_SUCCESSFUL;
    #endif

    #if defined(HSE_SPT_RSAES_PKCS1_V15)
    RsaExampleStarted_k |= RSA_PKCS1V15_EXAMPLE_STARTED;
    hseSrvResponse = HSE_RSA_Pkcs1V15_Example(rsapairHandle);
    ASSERT(HSE_SRV_RSP_OK == hseSrvResponse);
    RsaExampleStatus_k |= RSA_PKCS1V15_EXAMPLE_SUCCESSFUL;
    #endif

    ASSERT((uint8_t)RsaExampleStatus_k == (uint8_t)RsaExampleStarted_k);
    return hseSrvResponse;
}
#endif
