/**
 *   @file    hse_config.c
 *
 *   @brief   Example of HSE configuration
 *   @details Example of services used for configuring HSE persistent attributes and key catalogs.
 *
 *   @addtogroup [HSE_CONFIG]
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
extern "C"
{
#endif

/*=============================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
=============================================================================*/
#include "hse_demo_app_services.h"
#include "hse_host_attrs.h"
#include "hse_host_boot.h"
#include "hse_host_format_key_catalogs.h"
#include "hse_host_import_key.h"
#include "hse_host_mac.h"
#include "hse_mu.h"
#include "hse_srv_attr.h"
#include "hse_host.h"
#include "hse_host_ecc.h"
#include "hse_host_sign.h"
#include "hse_she_commands.h"
#include "hse_host_wrappers.h"
#include "hse_b_catalog_formatting.h"
#include <string.h>
#include "fsl_common.h"

/*=============================================================================
    *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
    * ==========================================================================*/

/*=============================================================================
    *                                       LOCAL MACROS
    * ==========================================================================*/

/*=============================================================================
    *                                      LOCAL CONSTANTS
    * ==========================================================================*/

/*============================================================================
    *                                      LOCAL VARIABLES
    * =========================================================================*/
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
 * @brief ECC Priv key
 */
static const uint8_t eccP256PrivKey[] =
{ 0x00, 0xd0, 0x07, 0xe1, 0xb9, 0xaf, 0xcc, 0x31, 0x2e, 0xec, 0x9c, 0xec, 0xff,
        0xa0, 0x28, 0x07, 0x52, 0xbb, 0xd1, 0x95, 0x31, 0x82, 0xed, 0xef, 0x12,
        0xf3, 0xfc, 0x36, 0x6e, 0x8f, 0x43, 0x56
};
#ifdef HSE_SPT_HMAC
SDK_ALIGN(static const uint8_t hmacKeyInitial[], 8u) =
{ 0x9a, 0x8c, 0xd4, 0x01, 0x8b, 0x1d, 0xe0, 0xaf, 0x06, 0xbb, 0xd7, 0x7a, 0xd3,
        0x28, 0x94, 0xb0, 0x36, 0xb2, 0x1e, 0xc7, 0x29, 0xbb, 0xd1, 0x09, 0x7b,
        0x34, 0xf2, 0x0e, 0x07, 0x0e, 0xbb, 0x29, 0xf9, 0x18, 0xda, 0xb1, 0x02,
        0xe0, 0x60, 0x4f, 0x19, 0x19, 0xc4, 0x58, 0x05, 0x80, 0xbf, 0x69, 0x2e,
        0xad
};
static const uint32_t hmacKeyInitialLength = ARRAY_SIZE(hmacKeyInitial);
#endif

/*AES ECB Data*/
SDK_ALIGN(const uint8_t aesEcbKey[], 8u) =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

const uint32_t aesEcbKeyLength = ARRAY_SIZE(aesEcbKey);

/*=============================================================================
    *                                      GLOBAL CONSTANTS
    * ==========================================================================*/

/*=============================================================================
    *                                      GLOBAL VARIABLES
    * ==========================================================================*/
hseKeyHandle_t aesCmacKeyHandle = HSE_INVALID_KEY_HANDLE;
hseKeyHandle_t AesNVMKeyHandle = HSE_DEMO_NVM_AES256_KEY2;

/*=============================================================================
    *                                   LOCAL FUNCTION PROTOTYPES
    * ==========================================================================*/
hseSrvResponse_t HSE_ConfigKeyCatalogs(void);

/*=============================================================================
*                                       LOCAL FUNCTIONS
* ==========================================================================*/

/**
    Function: HSE_ConfigKeyCatalogs
    @brief    Format key catalogs
    @detailed Example of key catalog for SHE and SB format HSE service
                which defines the layout of the Key store memory space:
                (number of slots, number of keys/slot, key types,
                usage/MU and ownership).
                Key catalogs format will be part of the SYS_IMG
                that provides persistency of HSE configuration
*/
hseSrvResponse_t HSE_ConfigKeyCatalogs(void)
{
    hseSrvResponse_t srvResponse;
    // Format HSE key catalogs
    srvResponse = FormatKeyCatalogs_();
    return srvResponse;
}

/**
    Function: HSE_InstallNvmKeys
    @brief    NVM keys installation example
    @detailed Example of key catalog format HSE service which defines the
                layout of the Key store memory space:
                (number of slots, number of keys/slot, key types, usage/MU
                and ownership).NVM keys will be part of the SYS_IMG.
*/
static hseSrvResponse_t HSE_InstallNvmKeys(void)
{
    hseSrvResponse_t srvResponse;
    hseKeyHandle_t aesNVMAuthKeyHandle = HSE_DEMO_NVM_AES128_AUTHORIZATION_KEY;

    /* Calling the Key Verify service and check the tag of SHA256 computed from stored key
       If the Tag is not found then import the respective key */
    srvResponse = VerifySHA(gAES128AuthorizationKey, &aesNVMAuthKeyHandle);

    if (HSE_SRV_RSP_KEY_EMPTY == srvResponse)
    {
        /* Import Authorization key */
        srvResponse = ImportPlainSymKeyReq(
            HSE_DEMO_NVM_AES128_AUTHORIZATION_KEY,
            HSE_KEY_TYPE_AES,
            (
                HSE_KF_USAGE_AUTHORIZATION |
                HSE_KF_USAGE_VERIFY),
            ARRAY_SIZE(gAES128AuthorizationKey),
            gAES128AuthorizationKey, 0U);

        if (HSE_SRV_RSP_OK != srvResponse)
        {
            goto exit;
        }
    }
    else if (HSE_SRV_RSP_OK != srvResponse)
    {
        testStatus |= KEY_IMPORT_FAILED;
    }
exit:
    return srvResponse;
}

hseSrvResponse_t ImportAsymmetricKeys()
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    #if defined(HSE_SPT_KEY_DERIVE) && defined(HSE_SPT_CLASSIC_DH)
    hseKeyHandle_t DHSharedSecretRamKeyHandle = HSE_DEMO_DH_SHARED_SECRET_HANDLE;
    #endif /* HSE_SPT_KEY_DERIVE */
    hseKeyHandle_t eccKeyPairRAMKeyHandle = HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE;

    /* import keys for ECC operation before calling HSE_Ecdsa_Example fn */
    srvResponse = ImportEccKeyReq(
        HSE_DEMO_NVM_ECC_KEY_HANDLE,
        HSE_KEY_TYPE_ECC_PAIR,
        (HSE_KF_USAGE_SIGN | HSE_KF_USAGE_VERIFY),
        HSE_EC_SEC_SECP256R1,
        KeyBitLen(HSE_EC_SEC_SECP256R1),
        eccP256PubKey,
        eccP256PrivKey);

    if (HSE_SRV_RSP_OK != srvResponse)
        goto exit;

    srvResponse = ImportEccKeyReq(
        HSE_DEMO_NVM_ECC_KEY_HANDLE_PUBLIC,
        HSE_KEY_TYPE_ECC_PUB,
        (HSE_KF_USAGE_VERIFY),
        HSE_EC_SEC_SECP256R1,
        KeyBitLen(HSE_EC_SEC_SECP256R1),
        eccP256PubKey,
        NULL);

    if (HSE_SRV_RSP_OK != srvResponse)
        goto exit;

    /* import session keys before calling HSE_SessionKeys_Example fn */
    /* Generate an ECC key pair in RAM (ephemeral ECDH) */
    srvResponse = GenerateEccKey(
        &eccKeyPairRAMKeyHandle,
        RAM_KEY,
        HSE_EC_SEC_SECP256R1,
        HSE_KF_USAGE_EXCHANGE);

    if (HSE_SRV_RSP_OK != srvResponse)
        goto exit;

    /* Import the peer public key */
    srvResponse = ImportEccKeyReq(
        HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE, HSE_KEY_TYPE_ECC_PUB,
        HSE_KF_USAGE_EXCHANGE, HSE_EC_SEC_SECP256R1,
        KeyBitLen(HSE_EC_SEC_SECP256R1), eccP256PubKey, NULL);

    if (HSE_SRV_RSP_OK != srvResponse)
        goto exit;

    /*Compute DH Shared Secret (ECDH)*/
    #if defined(HSE_SPT_KEY_DERIVE) && defined(HSE_SPT_CLASSIC_DH)
    srvResponse = DHSharedSecretCompute(
        HSE_DEMO_RAM_ECC_PUB_KEY_HANDLE,
        HSE_DEMO_RAM_ECC_PAIR_KEY_HANDLE,
        &DHSharedSecretRamKeyHandle,
        RAM_KEY,
        KeyBitLen(HSE_EC_SEC_SECP256R1));

    if (HSE_SRV_RSP_OK != srvResponse)
        goto exit;
    #endif /* HSE_SPT_KEY_DERIVE */

#ifdef HSE_SPT_RSA
    srvResponse = SetCustAuthorizationKey();
    if (HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }
#endif

    #ifdef HSE_SPT_SHE
    /* import keys before calling HSE_SysAuthorization_Example fn */
    srvResponse = cmd_debug_chal_auth();
    if (HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }
    #endif

exit:
    return srvResponse;
}

/*=============================================================================
*                                       GLOBAL FUNCTIONS
* ==========================================================================*/

/******************************************************************************
 * Function:    HSE_Config
 * Description: Configure NVM attributes, key catalogs and install NVM keys
 *****************************************************************************/
hseSrvResponse_t FormatKeyCatalog(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /* check if the user have SU rights or not, if not then request for it */
    srvResponse = Sys_Auth();
    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    /* format keys */
    srvResponse = HSE_ConfigKeyCatalogs();
    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    #ifdef HSE_SPT_SHE
    srvResponse = Load_Relevant_She_Keys();
    ASSERT(HSE_SRV_RSP_OK == srvResponse);
    #endif

    srvResponse = ImportAsymmetricKeys();
    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    return srvResponse;
}

/******************************************************************************
 * Function:    ImportKeys
 * Description: Import all NVM keys required for crypto services and secure boot
 *****************************************************************************/
hseSrvResponse_t ImportSymmetricKeys(void)
{
    hseSrvResponse_t srvResponse;
    #ifdef HSE_SPT_HMAC
    hseKeyHandle_t hmacNVMKeyHandle0 = HSE_DEMO_NVM_HMAC_KEY0;
    hseKeyHandle_t hmacNVMKeyHandle1 = HSE_DEMO_NVM_HMAC_KEY1;
    #endif
    hseKeyHandle_t aesNVMProvisionKeyHandle = HSE_DEMO_NVM_AES128_PROVISION_KEY;
    hseKeyHandle_t aesNVMBootKeyHandle = HSE_DEMO_NVM_AES128_BOOT_KEY;

    srvResponse = ParseKeyCatalogs();
    ASSERT(HSE_SRV_RSP_OK == srvResponse);
    
    /* import keys */
    srvResponse = HSE_InstallNvmKeys();
    if (HSE_SRV_RSP_OK != srvResponse)
        goto exit;

    /*Calling the Key Verify service and check the tag of SHA256 computed from stored key
    If the Tag is not found then import the respective key*/
    srvResponse = VerifySHA(aesEcbKey, &AesNVMKeyHandle);
    if (HSE_SRV_RSP_KEY_EMPTY == srvResponse)
    {
        /* load AES keys for FAST CMAC operation */
        srvResponse = LoadAesKey(&AesNVMKeyHandle, 1U, aesEcbKeyLength, aesEcbKey);
        if (HSE_SRV_RSP_OK != srvResponse)
        {
            goto exit;
        }
    }
    else if (HSE_SRV_RSP_OK != srvResponse)
    {
        testStatus |= KEY_IMPORT_FAILED;
    }
#if 0
    /* Calling the Key Verify service and check the tag of SHA256 computed from stored key
       If the Tag is not found then import the respective key */
    srvResponse = VerifySHA(aesEcbKey, &aesCmacKeyHandle);
    if (HSE_SRV_RSP_KEY_EMPTY == srvResponse)
    {
        /* load AES keys for operation */
        srvResponse = LoadAesKey(&aesCmacKeyHandle, NVM_KEY, aesEcbKeyLength, aesEcbKey);
        if (HSE_SRV_RSP_OK != srvResponse)
        {
            goto exit;
        }
    }
    else if (HSE_SRV_RSP_OK != srvResponse)
    {
        testStatus |= KEY_IMPORT_FAILED;
    }
#endif
    #ifdef HSE_SPT_HMAC
    /*Calling the Key Verify service and check the tag of SHA256 computed from stored key
    If the Tag is not found then import the respective key*/
    srvResponse = VerifySHA(hmacKeyInitial, &hmacNVMKeyHandle0);
    if (HSE_SRV_RSP_KEY_EMPTY == srvResponse)
    {
        /* import keys before calling HSE_UpdateNvmKey_Example fn */
        srvResponse = ImportPlainSymKeyReq(hmacNVMKeyHandle0, HSE_KEY_TYPE_HMAC,
                                            HSE_KF_USAGE_SIGN | HSE_KF_USAGE_VERIFY, hmacKeyInitialLength, hmacKeyInitial, 0U);
        if (HSE_SRV_RSP_OK != srvResponse)
        {
            goto exit;
        }
    }
    else if (HSE_SRV_RSP_OK != srvResponse)
    {
        testStatus |= KEY_IMPORT_FAILED;
    }
    /*Calling the Key Verify service and check the tag of SHA256 computed from stored key
    If the Tag is not found then import the respective key*/
    srvResponse = VerifySHA(hmacKeyInitial, &hmacNVMKeyHandle1);

    if (HSE_SRV_RSP_KEY_EMPTY == srvResponse)
    {
        srvResponse = ImportPlainSymKeyReq(hmacNVMKeyHandle1, HSE_KEY_TYPE_HMAC,
                                            HSE_KF_USAGE_VERIFY, hmacKeyInitialLength, hmacKeyInitial, 0U);
        if (HSE_SRV_RSP_OK != srvResponse)
        {
            goto exit;
        }
    }
    else if (HSE_SRV_RSP_OK != srvResponse)
    {
        testStatus |= KEY_IMPORT_FAILED;
    }
    #endif

    srvResponse = ImportPlainSymKeyReq(
        HSE_DEMO_RAM_AES128_KEY1,
        HSE_KEY_TYPE_AES,
        HSE_KF_USAGE_SIGN,
        gAESProvisionKeyLength,
        gAES128ProvisionKey,
        0U);
    if (HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /*Calling the Key Verify service and check the tag of SHA256 computed from stored key
    If the Tag is not found then import the respective key*/
    srvResponse = VerifySHA(aesEcbKey, &aesNVMProvisionKeyHandle);
    if (HSE_SRV_RSP_KEY_EMPTY == srvResponse)
    {
        /* load keys for Advanced secure boot */
        srvResponse = ImportPlainSymKeyReqMuChannel(
            MU0,
            1U,
            aesNVMProvisionKeyHandle,
            HSE_KEY_TYPE_AES,
            (HSE_KF_USAGE_SIGN | HSE_KF_USAGE_VERIFY),
            0U,
            aesEcbKeyLength,
            aesEcbKey,
            TRUE,
            0U);
        if (HSE_SRV_RSP_OK != srvResponse)
        {
            goto exit;
        }
    }
    else if (HSE_SRV_RSP_OK != srvResponse)
    {
        testStatus |= KEY_IMPORT_FAILED;
    }
    /*Calling the Key Verify service and check the tag of SHA256 computed from stored key
    If the Tag is not found then import the respective key*/

    srvResponse = VerifySHA(aesEcbKey, &aesNVMBootKeyHandle);
    if (HSE_SRV_RSP_KEY_EMPTY == srvResponse)
    {
        /*Import key linked with SMR#0*/
        srvResponse = ImportPlainSymKeyReqMuChannel(
            MU0,
            1U,
            aesNVMBootKeyHandle,
            HSE_KEY_TYPE_AES,
            HSE_KF_USAGE_VERIFY,
            0U,
            aesEcbKeyLength,
            aesEcbKey,
            TRUE,
            0);
        if (HSE_SRV_RSP_OK != srvResponse)
        {
            goto exit;
        }
    }
    else if (HSE_SRV_RSP_OK != srvResponse)
    {
        testStatus |= KEY_IMPORT_FAILED;
    }

exit:
    return srvResponse;
}

/******************************************************************************
 * Function:    HSE_EraseKeys
 * Description: Erase NVM attributes, key catalogs and NVM keys
 *****************************************************************************/
hseSrvResponse_t HSE_EraseKeys(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    hseSrvDescriptor_t *pHseSrvDesc = &gHseSrvDesc[0U][1U];

    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    // re-format the NVM and RAM catalogs
    pHseSrvDesc->srvId = HSE_SRV_ID_ERASE_HSE_NVM_DATA;
    srvResponse = HSE_Send(0U, 1U, gSyncTxOption, pHseSrvDesc);

    ASSERT(HSE_SRV_RSP_OK == srvResponse);
    return srvResponse;
}

/******************************************************************************
 * Function:    Sys_Auth
 * Description: Grant Super user rights to user
 *****************************************************************************/
hseSrvResponse_t Sys_Auth(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /* read LC first */
    if (HSE_STATUS_CUST_SUPER_USER & HSE_MU_GetHseStatus(MU0))
    {
        /* user already have SU rights */
        srvResponse = HSE_SRV_RSP_OK;
        goto exit;
    }
    else
    {
        #ifdef HSE_SPT_RSA
        srvResponse = Grant_SuperUser_Rights();
        #endif
    }
exit:
    return srvResponse;
}
#ifdef __cplusplus
}
#endif

/** @} */
