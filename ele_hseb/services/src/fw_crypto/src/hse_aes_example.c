/*
 * @file hse_aes_example.c
 *
 * @brief Header file for AES example in crypto services
 * @details Contains the API's for demonstrating the use of AES crypto service in DemoApp
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
#include "hse_aes_example.h"


/*==================================================================================================
                                 LOCAL VARIABLEs
==================================================================================================*/

static const uint8_t aesEcbPlaintext[16] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73,
        0x93, 0x17, 0x2a
};

/*
 * @brief AES ECB Cipher Text
 */
static const uint8_t aesEcbCiphertext[] =
{ 0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 0xa8, 0x9e, 0xca, 0xf3, 0x24,
        0x66, 0xef, 0x97
};

/*
 * @brief AES ECB plain and Cipher text length
 */
static const uint32_t aesEcbPlaintextLength = ARRAY_SIZE(aesEcbPlaintext);
static const uint32_t aesEcbCiphertextLength = ARRAY_SIZE(aesEcbCiphertext);

/*
 * @brief variable for time calculation for Fast CMAC generate and verify keys
 */
static volatile uint32_t FastCmacGenerateTime = 0U;
static volatile uint32_t FastCmacVerifyTime = 0U;
static volatile uint32_t TotalFastCmacGenerateTime = 0U;
static volatile uint32_t TotalFastCmacVerifyTime = 0U;
static uint8_t testoutput[BUFFER_SIZE] = {0};

/*
 * ============================================================================
 *                               LOCAL FUNCTIONS DEFINATIONS
 * ============================================================================
*/

/******************************************************************************
 * Function:    HSE_Aes_Example
 * Description:  Example of AES requests sent synchronously.
 *****************************************************************************/
hseSrvResponse_t HSE_Aes_Example(void)
{
    hseKeyHandle_t aesEcbKeyHandle = HSE_DEMO_RAM_AES128_KEY0;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /* import keys for AES operation before calling HSE_Aes_Example fn*/
    srvResponse = ImportPlainSymKeyReq(
            aesEcbKeyHandle ,
            HSE_KEY_TYPE_AES,
            (HSE_KF_USAGE_ENCRYPT|HSE_KF_USAGE_DECRYPT),
            aesEcbKeyLength,
            aesEcbKey,
            0U);

    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    /* --------- AES ECB Encrypt Request --------- */

    memset(testoutput, 0, BUFFER_SIZE);

    /* Send the request */
    srvResponse = AesEncrypt( aesEcbKeyHandle, HSE_CIPHER_BLOCK_MODE_ECB,
        0UL, aesEcbPlaintextLength, aesEcbPlaintext, testoutput, HSE_SGT_OPTION_NONE);

    /* Check response and output */
    if( (HSE_SRV_RSP_OK != srvResponse) ||
        (0 != memcmp(testoutput, aesEcbCiphertext, aesEcbCiphertextLength)) )
    {
        goto exit;
    }

    /* --------- AES ECB Decrypt Request --------- */

    memset(testoutput, 0, BUFFER_SIZE);

    /* Send the request */
    srvResponse = AesDecrypt(aesEcbKeyHandle, HSE_CIPHER_BLOCK_MODE_ECB,
             0UL, aesEcbCiphertextLength, aesEcbCiphertext, testoutput, HSE_SGT_OPTION_NONE);

     /*Check response and output*/
    if( (HSE_SRV_RSP_OK != srvResponse) ||
        (0 != memcmp(testoutput, aesEcbPlaintext, aesEcbPlaintextLength)))
    {
        goto exit;
    }

    /* --------- AES Fast CMAC Generate/Verify Requests with STM measuring (50 reqs) --------- */
    for (uint32_t i = 0; i < MAX_REQS_FOR_FAST_CMAC; i++)
    {
    memset(testoutput, 0, BUFFER_SIZE);

    /* Generate */
    srvResponse = AesFastCmacGenerate(AesNVMKeyHandle,
    NUM_OF_ELEMS(aesEcbPlaintext)*8U, aesEcbPlaintext, (16U*8U), testoutput);


    /* calculate time when STM is derived from PLL of frequency 48MHz */
    TotalFastCmacGenerateTime += (FastCmacGenerateTime/48);
    if(HSE_SRV_RSP_OK != srvResponse)
    goto exit;

    /* Verify */
    srvResponse = AesFastCmacVerify(AesNVMKeyHandle,
    NUM_OF_ELEMS(aesEcbPlaintext)*8U, aesEcbPlaintext, (16U*8U), testoutput);

    if(HSE_SRV_RSP_OK != srvResponse)
    goto exit;
    }

exit:
   return srvResponse;
}
