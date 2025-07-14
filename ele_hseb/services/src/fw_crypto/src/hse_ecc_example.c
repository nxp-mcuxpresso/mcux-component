/*
 * @file hse_ecc_example.c
 *
 * @brief Source file for ECC Example in crypto services
 * @details Contains the API's for demonstrating the use of ECC crypto service in DemoApp
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

#include "hse_ecc_example.h"

#ifdef HSE_SPT_ECC_COMPRESSED_KEYS
static uint8_t signR[256] = {0};
static uint8_t signS[256] = {0};
static uint32_t signRLen = ARRAY_SIZE(signR);
static uint32_t signSLen = ARRAY_SIZE(signS);
#endif

/*============================================================================
   *                                      LOCAL VARIABLES
   * =========================================================================*/

#ifdef HSE_SPT_ECC_COMPRESSED_KEYS
/*
 * @brief ECC messages
 */
static const uint8_t message[] =
        "The surest way to corrupt a youth is to instruct him to hold in higher esteem those who think alike than those who think differently - Friedrich Nietzsche 2020";
#endif

#ifdef HSE_SPT_ECC_COMPRESSED_KEYS
/*
 * @brief SECP Pub key
 */
static const uint8_t secp256v1PubKey_2[]  = {0xF5, 0xCF, 0xC6, 0x06, 0xEF, 0x6E, 0x52, 0x0F, 0xA9, 0xBA,
 0xDA, 0x8F, 0x03,0x39, 0x71, 0x17, 0x5C, 0xF2, 0xCB, 0xD5, 0x16, 0x31, 0xE0, 0x30, 0x03, 0xE4, 0xE7,
 0x00, 0x30, 0x79, 0xC1, 0xB2, 0x7B, 0xE8, 0xF8, 0x4C, 0xE4, 0xA0, 0xEA, 0x71, 0x0B, 0x60, 0x37, 0xD9,
 0xF6, 0x23, 0x47, 0xFF, 0x99, 0x7D, 0x36,0xAC, 0xD6, 0x51, 0x87, 0x67, 0x03, 0xFD, 0x06, 0x74, 0xF2,
 0xE3, 0xAC, 0xF0};
#endif

/****************************************************************************
* Function:    HSE_Ecdsa_Example
* Description: Example of ECC key pair generation and ECDSA Sign/Verify operations
*       Send a request to generate ECC exportable key pair, usable for signing and verification.
*      - Send request to HSE & Verify the response is OK
*      - Initialize a request to ECDSA sign with the
*        generated key a test vector
*      - Send request to HSE & Verify the response is OK
*      - Initialize a request to ECDSA verify with the
*        generated key the signed test vector
 *****************************************************************************/
 #ifdef HSE_SPT_ECC_COMPRESSED_KEYS
hseSrvResponse_t HSE_Ecdsa_Example(void)
{
    hseSrvResponse_t hseResponse;
    hseKeyHandle_t eccKeyPairRAMKeyHandle = HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE;

    //Generate an exportable ECC key pair in RAM (ephemeral ECDH)
    hseResponse = GenerateEccKey(
            &eccKeyPairRAMKeyHandle,
            RAM_KEY,
            HSE_EC_SEC_SECP256R1,
            HSE_KF_USAGE_SIGN | HSE_KF_USAGE_VERIFY | HSE_KF_ACCESS_EXPORTABLE);
    if(HSE_SRV_RSP_OK != hseResponse)
        goto exit;

    /* ECDSA Sign request */
     hseResponse = EcdsaSign(
             HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE,
             HSE_HASH_ALGO_SHA2_256,
             ARRAY_SIZE(message), message,FALSE, 0U,
             &signRLen, signR, &signSLen, signS);
     if(HSE_SRV_RSP_OK != hseResponse)
         goto exit;

     /* ECDSA Verify request */
     hseResponse = EcdsaVerify(
              HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE, HSE_HASH_ALGO_SHA2_256,
             ARRAY_SIZE(message), message, FALSE, 0U,
             &signRLen, signR, &signSLen, signS);
     if(HSE_SRV_RSP_OK != hseResponse)
         goto exit;

exit:
    return hseResponse;
}
/*******************************************************************************
 * Function:    TestCompressedEccKeyExport
 *
 * Description: Test that a given key can be exported in compressed format
 *
 * Returns:     nothing
 ********************************************************************************/
static hseSrvResponse_t TestCompressedEccKeyExport(uint8_t curveId, const uint8_t* pKey)
{
    hseSrvResponse_t hseStatus = HSE_SRV_RSP_GENERAL_ERROR;
    uint8_t keyBuf[BITS_TO_BYTES(HSE_MAX_ECC_KEY_BITS_LEN) + 1];//67
    uint16_t keyBufLen = sizeof(keyBuf);//67 bytes
    hseKeyInfo_t keyInfo;
    uint8_t signByte;
    uint8_t keyByteLen = BITS_TO_BYTES(KeyBitLen(curveId));//32 bytes
    hseKeyHandle_t eccPublicRAMKeyHandle = HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE;
    hseStatus = LoadEccPublicKey(&eccPublicRAMKeyHandle  ,
                                 RAM_KEY,
                                 curveId,
                                 KeyBitLen(curveId),
                                 pKey);
     if(HSE_SRV_RSP_OK != hseStatus)
     goto exit;
    hseStatus = ExportPlainFormattedEccPubKeyReq(HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE  ,
                                                 &keyInfo,
                                                 &keyBufLen,
                                                 keyBuf,
                                                 HSE_KEY_FORMAT_ECC_PUB_COMPRESSED);
    if(HSE_SRV_RSP_OK != hseStatus)
     goto exit;

    //key length must be curve byte length + 1
    if((1 + keyByteLen) != keyBufLen)
    {  //33                //33
    hseStatus=HSE_SRV_RSP_VERIFY_FAILED;
    goto exit;
    }

    // check that x coordinate matches
    if(0 != (memcmp(keyBuf + 1, pKey, keyByteLen)))
    {
    hseStatus=HSE_SRV_RSP_VERIFY_FAILED;
    goto exit;
    }

    // check that the sign of the y coordinate matches
    if(0x01 == (pKey[2 * keyByteLen - 1] & 0x01))
    {
        signByte = 0x03;
    }
    else
    {
        signByte = 0x02;
    }
    if(signByte!= keyBuf[0])
    {
    hseStatus=HSE_SRV_RSP_VERIFY_FAILED;
    goto exit;
    }
    exit:
    return hseStatus;
}

/*******************************************************************************
 * Function:    TestUncompressedEccKeyExport
 *
 * Description: Test that a given key can be exported in uncompressed format
 *
 * Returns:     nothing
 ********************************************************************************/
static hseSrvResponse_t TestUncompressedEccKeyExport(uint8_t curveId, const uint8_t* pKey)
{
    hseSrvResponse_t hseStatus = HSE_SRV_RSP_GENERAL_ERROR;
    uint8_t keyBuf[2 * BITS_TO_BYTES(HSE_MAX_ECC_KEY_BITS_LEN) + 1];//133
    uint16_t keyBufLen = sizeof(keyBuf);//133
    hseKeyInfo_t keyInfo;
    uint8_t keyByteLen = BITS_TO_BYTES(KeyBitLen(curveId));//32 bytes
    hseKeyHandle_t eccPublicRAMKeyHandle = HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE;
    hseStatus = LoadEccPublicKey(&eccPublicRAMKeyHandle  ,
                                 RAM_KEY,
                                 curveId,
                                 KeyBitLen(curveId), //256 bits
                                 pKey);
    if(HSE_SRV_RSP_OK != hseStatus)
        goto exit;

    hseStatus = ExportPlainFormattedEccPubKeyReq(HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE  ,
                                                 &keyInfo,
                                                 &keyBufLen,
                                                 keyBuf,
                                                 HSE_KEY_FORMAT_ECC_PUB_UNCOMPRESSED);
    if(HSE_SRV_RSP_OK != hseStatus)
        goto exit;

    // key length must be 2 * curve byte length + 1
    if((1 + 2 * keyByteLen)!= keyBufLen)//65
    {
        hseStatus=HSE_SRV_RSP_VERIFY_FAILED;
        goto exit;
    }

    // check that the key matches
    if(0 != memcmp(keyBuf + 1, pKey, 2 * keyByteLen))
    {
        hseStatus=HSE_SRV_RSP_VERIFY_FAILED;
        goto exit;
    }

    // check that the first byte is 0x04 (which marks an uncompressed key)
    if(0x04!= keyBuf[0])
    {
        hseStatus=HSE_SRV_RSP_VERIFY_FAILED;
        goto exit;
    }
exit:
    return hseStatus;
}
/*******************************************************************************
 * Function:    TestRawEccKeyExport
 *
 * Description: Test that a given key can be exported in raw format
 *
 * Returns:     nothing
 ********************************************************************************/
 static hseSrvResponse_t TestRawEccKeyExport(uint8_t curveId, const uint8_t* pKey)
{
    hseSrvResponse_t hseStatus = HSE_SRV_RSP_GENERAL_ERROR;
    uint8_t keyBuf[2 * BITS_TO_BYTES(HSE_MAX_ECC_KEY_BITS_LEN)];//132
    uint16_t keyBufLen = sizeof(keyBuf);//132
    hseKeyInfo_t keyInfo;
    uint8_t keyByteLen = BITS_TO_BYTES(KeyBitLen(curveId));//32
    hseKeyHandle_t eccPublicRAMKeyHandle = HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE;

    hseStatus = LoadEccPublicKey(&eccPublicRAMKeyHandle   ,
                                 RAM_KEY,
                                 curveId,
                                 KeyBitLen(curveId),
                                 pKey);
    if(HSE_SRV_RSP_OK != hseStatus)
        goto exit;

    hseStatus = ExportPlainFormattedEccPubKeyReq(HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE  ,
                                                 &keyInfo,
                                                 &keyBufLen,
                                                 keyBuf,
                                                 HSE_KEY_FORMAT_ECC_PUB_RAW);
    if(HSE_SRV_RSP_OK != hseStatus)
        goto exit;

    // key length must be 2 * curve byte length
    if((2 * keyByteLen)!= keyBufLen)//64
    {
        hseStatus=HSE_SRV_RSP_VERIFY_FAILED;
        goto exit;
    }

    // check that the key matches
    if(0 != memcmp(keyBuf, pKey, 2 * keyByteLen))//64
    {
        hseStatus=HSE_SRV_RSP_VERIFY_FAILED;
        goto exit;
    }
exit:
    return hseStatus;
}

/**
* @test_id TestCompressedEcccPublicKeyExportSuite
* @brief Test compressed key export
* @details Test that HSE can export keys in compressed uncompressed and raw format
* @test_type      Functional
* @test_procedure Steps:
*   For each key:
*   - Load key into HSE
*   - Export key in compressed,uncompressed and raw format
*   - Check that HSE returns HSE_SRV_RSP_OK
*   - Check that exported key is correct
* @pass_criteria Verification Points are successful
*/
hseSrvResponse_t TestEccPublicKeyExportSuite(void)
{
    hseSrvResponse_t hseResponse;
    hseResponse = TestCompressedEccKeyExport(HSE_EC_SEC_SECP256R1, secp256v1PubKey_2);
    if(HSE_SRV_RSP_OK != hseResponse)
    goto exit;
    hseResponse = TestUncompressedEccKeyExport(HSE_EC_SEC_SECP256R1, secp256v1PubKey_2);
    if(HSE_SRV_RSP_OK != hseResponse)
    goto exit;
    hseResponse = TestRawEccKeyExport(HSE_EC_SEC_SECP256R1, secp256v1PubKey_2);
    if(HSE_SRV_RSP_OK != hseResponse)
    goto exit;
exit:
    return hseResponse;
}


/*******************************************************************************
 * Function:    TestEcdsaVerifTestCase
 *
 * Description: Run through a single ECDSA verification test case.
 *              Import the public key, then try to verify the signature
 *
 * Returns:     whether the test passes or not
 ********************************************************************************/
hseSrvResponse_t TestEcdsaVerifTestCase(ecdsaVerifTestCase_t* pTestCase)
{
    hseSrvResponse_t hseStatus = HSE_SRV_RSP_GENERAL_ERROR;
    uint32_t rLen = 0U;
    uint32_t sLen = 0U;
    hseKeyHandle_t eccRAMKeyHandle = HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE;

    hseStatus = ParseKeyCatalogs();
    if(HSE_SRV_RSP_OK !=hseStatus)
        goto exit;

    hseStatus = LoadEccPublicKey(&eccRAMKeyHandle,RAM_KEY,pTestCase->curveId,
                                 KeyBitLen(pTestCase->curveId), pTestCase->pPubKey);
    if (HSE_SRV_RSP_OK != hseStatus)
    {
        goto exit;
    }
    rLen = pTestCase->rLen;
    sLen = pTestCase->sLen;

    hseStatus = EcdsaVerify(HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE, pTestCase->hashAlgo, pTestCase->msgLen, pTestCase->pMsg, FALSE, 0,
                            &rLen, pTestCase->r, &sLen, pTestCase->s);
    if (HSE_SRV_RSP_OK != hseStatus)
    {
        goto exit;
    }

exit:
    return hseStatus;
}

#endif
