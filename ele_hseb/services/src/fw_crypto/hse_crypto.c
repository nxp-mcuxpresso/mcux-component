/**
*   @file    hse_crypto.c
*
*   @brief   Example of HSE cryptographic services.
*   @details Example of sending crypto services requests to HSE over MU (sym/asym; sync/async).
*
*   @addtogroup hse_crypto
*   @{
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
#ifdef __cplusplus
extern "C"{
#endif

/*=============================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
=============================================================================*/
#include "hse_demo_app_services.h"
#include "hse_host.h"
#include "hse_host_Kdf_examples.h"
#include "hse_get_key_info.h"
#include "hse_hash_example.h"
#include "hse_rsa_example.h"
#include "hse_aead_example.h"
#include "hse_sys_auth_example.h"
#include "hse_BD_example.h"
#include "hse_aes_example.h"
#include "hse_fastcmac_example.h"
#include "hse_ecc_example.h"
#include "hse_sessionkeys_example.h"
#include "hse_UpdateNvmKeys_example.h"
#include "hse_publish_keystore_RamToFlash_example.h"
#include "fsl_debug_console.h"

/*
 * ============================================================================
 *                            LOCAL CONSTANTS
 * ============================================================================
*/
const uint8_t testMsg[] = "The quick brown fox jumps over the lazy dog.";
#if !defined(CHAR_ARRAY_SIZE_WITHOUT_TRAILING_ZERO)
#define CHAR_ARRAY_SIZE_WITHOUT_TRAILING_ZERO(x) (sizeof(x) / sizeof((x)[0]) - 1)
#endif

const uint8_t secp256v1Key[] = {0x48, 0x26, 0xD8, 0x96, 0xB2, 0x17, 0x4E, 0x9F, 0x82, 0x21, 0x95, 0x11, 0x67, 0xE5, 0x64, 0xE8, 0xE7, 0x9A, 0x17, 0x55, 0x71, 0xE0, 0x2B, 0xD5, 0x7F, 0xDC, 0x4E, 0x0C, 0x4D, 0xD6, 0x31, 0xA4, 0x09, 0x2E, 0xC5, 0x52, 0x6E, 0xD0, 0x2B, 0xEF, 0xFD, 0xDD, 0x6D, 0x76, 0xA9, 0x60, 0x1D, 0xB5, 0xAE, 0x8B, 0x5B, 0x9F, 0x38, 0xC5, 0xA0, 0x2D, 0x6E, 0x73, 0x86, 0xA7, 0xBD, 0xAF, 0xF0, 0xA3};

#ifdef HSE_SPT_SHA2_256
const uint8_t testMsgSha256Secp256v1_r[] = {0x4C, 0x82, 0x16, 0xA9, 0xEB, 0xEE, 0xFB, 0xAC, 0xD3, 0x68, 0x34, 0x73, 0x43, 0x00, 0x2D, 0xC5, 0xEE, 0x78, 0x15, 0x96, 0xC5, 0x71, 0xBD, 0xD7, 0x41, 0x3B, 0x4B, 0x45, 0x5C, 0x77, 0xDD, 0x90};
const uint8_t testMsgSha256Secp256v1_s[] = {0x42, 0x32, 0x76, 0xB4, 0xA5, 0x04, 0x15, 0x3F, 0xB1, 0x80, 0x67, 0xFC, 0x54, 0xEE, 0x6E, 0xFE, 0x88, 0x86, 0xC9, 0x56, 0xB2, 0x33, 0x70, 0x01, 0xF2, 0xBD, 0xBE, 0xC6, 0x57, 0x22, 0x93, 0x66};
#endif
/*
 * ============================================================================
 *                               GLOBAL VARIABLES
 * ============================================================================
*/
volatile CryptoServicesTestStatus_t gCryptoServicesExecuted = CRYPTO_SERVICE_SUCCESS_NONE;
volatile CryptoServicesTestStartedStatus_t  gCryptoServicesStarted =  CRYPTO_SERVICE_NONE;

#if defined(HSE_SPT_EC_SEC_SECP256R1)&&defined(HSE_SPT_ECC_COMPRESSED_KEYS)
ecdsaVerifTestCase_t ecdsaVerifTestCases[] =
{
    #if defined(HSE_SPT_SHA2_256) && defined(HSE_SPT_EC_SEC_SECP256R1)
    {
        .curveId  = HSE_EC_SEC_SECP256R1,
        .hashAlgo = HSE_HASH_ALGO_SHA2_256,
        .pMsg     = testMsg,
        .msgLen   = CHAR_ARRAY_SIZE_WITHOUT_TRAILING_ZERO(testMsg),
        .pPubKey  = secp256v1Key,
        .r        = testMsgSha256Secp256v1_r,
        .rLen     = ARRAY_SIZE(testMsgSha256Secp256v1_r),
        .s        = testMsgSha256Secp256v1_s,
        .sLen     = ARRAY_SIZE(testMsgSha256Secp256v1_s)
    },
#endif
};
#endif


/* ============================================================================
 *                              GLOBAL FUNCTIONS
 * ============================================================================
*/
/******************************************************************************
 * Function:    HSE_Crypto
 * Description: HSE crypto services examples
 *              Examples of symmetric/asymmetric, sync/async HSE crypto
 *              operations and SYS authorization
 *****************************************************************************/
hseSrvResponse_t HSE_Crypto(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    /* Key Catalogs must be configured */
    ASSERT(CHECK_HSE_STATUS(HSE_STATUS_INSTALL_OK));

    /*Note: Keys for all below cryptographic operations have already
    * been imported in Generic_ImportKey() called in main() function*/
    #ifdef HSE_SPT_AES
    /*run AES related tests*/
    gCryptoServicesStarted|=AES_EXAMPLE_STARTED;
    srvResponse = HSE_Aes_Example();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= AES_EXAMPLES_SUCCESS;
        PRINTF("AES crypto - passed\r\n");
    }
     #endif

    /*run HASH related tests*/
    gCryptoServicesStarted|=HASH_EXAMPLE_STARTED;
    srvResponse = HSE_HashAsync_Example();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= HASH_EXAMPLES_SUCCESS;
        PRINTF("HASH crypto - passed\r\n");
    }

    #ifdef HSE_SPT_ECC_COMPRESSED_KEYS
    /*run ECC related tests*/
    gCryptoServicesStarted|= ECC_KEYS_EXAMPLE_STARTED;
    srvResponse = HSE_Ecdsa_Example();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= ECC_KEYS_EXAMPLES_SUCCESS;
        PRINTF("ECC compressed crypto passed\r\n");
    }
    #endif

    #ifdef HSE_SPT_KEY_DERIVE
    /*run session key related tests*/
    gCryptoServicesStarted|= SESSION_KEY_EXAMPLE_STARTED;
    srvResponse = HSE_SessionKeys_Example();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= SESSION_KEY_EXAMPLES_SUCCESS;
        PRINTF("Session key - passed\r\n");
    }
    #endif

    /*run Fast Cmac with Counter Protocol */
    #if defined (HSE_SPT_MONOTONIC_COUNTERS) && defined (HSE_SPT_CMAC_WITH_COUNTER) && defined (HSE_SPT_FAST_CMAC)
    gCryptoServicesStarted|= FAST_CMAC_WITH_COUNTER_EXAMPLE_STARTED;
    srvResponse = HSE_FastCmacwithCounter_Example();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= FAST_CMAC_WITH_COUNTER_EXAMPLES_SUCCESS;
        PRINTF("FAST CMAC crypto - passed\r\n");
    }
    #endif

    /*run Burmester Desmedt Protocol */
    #ifdef HSE_SPT_BURMESTER_DESMEDT
    gCryptoServicesStarted|= BURMESTER_DESMEDT_EXAMPLE_STARTED;
    srvResponse = HSE_BurmesterDesmedt_Example();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= BURMESTER_DESMEDT_EXAMPLES_SUCCESS;
        PRINTF("Burmester Desmedt Protocol - passed\r\n");
    }
    #endif

    /*run sys authorization related tests*/
    #if defined(HSE_SPT_RSA) && defined(HSE_SPT_SHA2_512)
    gCryptoServicesStarted|=SYS_AUTHORIZATION_EXAMPLE_STARTED;
    srvResponse = HSE_SysAuthorization_Example();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= SYS_AUTHORIZATION_EXAMPLE_SUCCESS;
        PRINTF("Sys authorization - passed\r\n");
    }
    #endif

    /*run update NVM keys related tests*/
    #ifdef HSE_SPT_GMAC
    gCryptoServicesStarted|=UPDATE_NVM_KEY_EXAMPLE_STARTED;
    srvResponse = HSE_UpdateNvmKey_Example();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= UPDATE_NVM_KEY_EXAMPLE_SUCCESS;
        PRINTF("NVM key update - passed\r\n");
    }
    #endif

    /*run ECC Key Export related tests*/
    #ifdef HSE_SPT_ECC_COMPRESSED_KEYS
    gCryptoServicesStarted|= ECC_EXPORT_PUBLIC_KEY_STARTED;
    srvResponse = TestEccPublicKeyExportSuite();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= ECC_EXPORT_PUBLIC_KEY_SUCCESS;
        PRINTF("ECC public key export - passed\r\n");
    }
    #endif

    #ifdef HSE_SPT_ECC_COMPRESSED_KEYS
    /*Importing of ECC public key using import key
    ECC signature verification using the public key imported*/
    gCryptoServicesStarted|= ECC_IMPORT_PUBLIC_KEY_SIGNATURE_VERIFICATION_STARTED;
    srvResponse = TestEcdsaVerifTestCase(ecdsaVerifTestCases+0);
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= ECC_IMPORT_PUBLIC_KEY_SIGNATURE_VERIFICATION_SUCCESS;
        PRINTF("ECC public key import - passed\r\n");
    }
    #endif

    #ifdef HSE_SPT_AES
    /* Get Key Info: for AES keys*/
    gCryptoServicesStarted|=GET_KEY_INFO_STARTED;
    srvResponse = HSE_Get_Key_Info_Example();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= GET_KEY_INFO_SUCCESS;
        PRINTF("AES get key info - passed\r\n");
    }
    #endif

    #if defined(HSE_SPT_RSA) && (defined(HSE_SPT_RSAES_NO_PADDING)||defined(HSE_SPT_RSAES_OAEP)||defined(HSE_SPT_RSAES_PKCS1_V15))
    /* RSA Examples*/
    gCryptoServicesStarted|=RSA_EXAMPLE_STARTED;
    srvResponse =  HSE_RSA_Examples();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= RSA_EXAMPLE_SUCCESS;
        PRINTF("RSA crypto - passed\r\n");
    }
    #endif

    #ifdef HSE_SPT_AEAD
    /*AEAD  Examples*/
    gCryptoServicesStarted|=AEAD_EXAMPLE_STARTED;
    srvResponse = HseAeadExamples();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= AEAD_EXAMPLE_SUCCESS;
        PRINTF("AEAD crypto - passed\r\n");
    }
    #endif

/*  Disable to prevent flash write
    #ifdef HSE_SPT_PUBLISH_NVM_KEYSTORE_RAM_TO_FLASH
    gCryptoServicesStarted|=PUBLISH_KEYSTORE_RAMTOFLASH_EXAMPLE_STARTED;
    srvResponse = HSE_publish_keystore_RamToFlash_example_();
    if( HSE_SRV_RSP_OK == srvResponse)
    {
        gCryptoServicesExecuted |= PUBLISH_KEYSTORE_RAMTOFLASH_EXAMPLE_SUCCESS;
        PRINTF("HSE publish keystore to flash - passed\r\n");
    }
    #endif
*/
    ASSERT( (uint8_t)gCryptoServicesStarted == (uint8_t)gCryptoServicesExecuted );
    testStatus |= CRYPTOGRAPHIC_SERVICES_SUCCESS;
    return srvResponse;
}

#ifdef __cplusplus
}
#endif

/** @} */
