/*
 * hse_publish_keystore_RamToFlash_example.c
 * @brief Header file for publish_keystore_RamToFlash example in crypto services
 * @details Contains the API's for demonstrating the use of publish_keystore_RamToFlash_example crypto service in DemoApp
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
#include "hse_publish_keystore_RamToFlash_example.h"
/*==================================================================================================
                                LOCAL VARIABLE
==================================================================================================*/
#ifdef HSE_SPT_PUBLISH_NVM_KEYSTORE_RAM_TO_FLASH
static uint8_t muIf = 0U;         /* MU0 */
static uint8_t muChannelIdx = 1U; /* channel 1 */
static hseTxOptions_t txOptions = {HSE_TX_SYNCHRONOUS, NULL, NULL};

/*
* @brief variables to measure time before and after publish service ID
*/
//static uint32_t stmTime_before_publish_key_srvid = 0;
//static uint32_t stmTime_after_publish_key_srvid = 0;
/*
* @brief AES ECB keys
*/
static const uint8_t aesEcbKey2[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
/*
* @brief AES ECB keys data length
*/
static const uint32_t aesEcbKey2Length = ARRAY_SIZE(aesEcbKey2);
/*
* @brief AES CCM keys
*/
static const uint8_t aesCcmKey3[] ={ 0x1b, 0x87, 0x98, 0x09, 0x12, 0xfe, 0xa4, 0xb7,
                                        0xa1, 0xfa, 0x85, 0xf4, 0xd9, 0x0f, 0x9f, 0x33};
/*
* @brief AES CCM keys data length
*/
static const uint32_t aesCcmKey3Length = ARRAY_SIZE(aesCcmKey3);


/*
    * ============================================================================
    *                               FUNCTIONS DEFINATIONS
    * ============================================================================
*/


static hseSrvResponse_t ImportNVMkeysAPI(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    hseKeyHandle_t aesAuthKeyHandle = HSE_DEMO_NVM_AES128_AUTHORIZATION_KEY;
    srvResponse = ImportPlainSymKeyReq(aesAuthKeyHandle, HSE_KEY_TYPE_AES,
                          (HSE_KF_USAGE_AUTHORIZATION |HSE_KF_USAGE_VERIFY),
                          ARRAY_SIZE(gAES128AuthorizationKey),
                          gAES128AuthorizationKey, 0U);
    ASSERT(HSE_SRV_RSP_OK == srvResponse);
    HKF_FreeKeySlot(&aesAuthKeyHandle);

    hseKeyHandle_t aesKey2Handle = HSE_DEMO_NVM_AES256_KEY2;
    srvResponse = LoadAesKey(&aesKey2Handle, NVM_KEY, aesEcbKey2Length, aesEcbKey2);
    ASSERT(HSE_SRV_RSP_OK == srvResponse);
    HKF_FreeKeySlot(&aesKey2Handle);

    hseKeyHandle_t aesKey3Handle = HSE_DEMO_NVM_AES256_KEY3;
    srvResponse = LoadAesKeyWithCipherMode(&aesKey3Handle, NVM_KEY, aesCcmKey3Length, aesCcmKey3, HSE_KU_AES_BLOCK_MODE_CCM);
    ASSERT(HSE_SRV_RSP_OK == srvResponse);
    HKF_FreeKeySlot(&aesKey3Handle);

    return srvResponse;

}
static hseSrvResponse_t HSE_Publish_Keystore_RamtoFlash(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    hseSrvDescriptor_t* pHseSrvDesc = &gHseSrvDesc[muIf][muChannelIdx];

    pHseSrvDesc->srvId = HSE_SRV_ID_PUBLISH_NVM_KEYSTORE_RAM_TO_FLASH;
    srvResponse = HSE_Send(muIf, muChannelIdx, txOptions, pHseSrvDesc);
    return srvResponse;
}
/*
 * @description The scope of this API, is to demonstrate the use of HSE_SRV_ID_PUBLISH_NVM_KEYSTORE_RAM_TO_FLASH
 * This service ID is used to request the firmware to write the NVM keys from secure RAM mirror keystore into the data flash.
 *
 * Now backstory is that when NVM keys are updating(import/load/generate/erase/export) HSE-FW, copy them into data flash
 * and also mirror them at secure RAM.So their are multiple write operations in data flash, when the key store is updating.
 * NVM keys are only used from mirrored secure RAM memory after loading them. So we can reduce the number of write
 * operations in data flash, by not writing the keys on them and at the time of updating them. When all key store are updated then
 * we can send the HSE_SRV_ID_PUBLISH_NVM_KEYSTORE_RAM_TO_FLASH to  write the NVM keys from secure RAM mirror keystore into the data flash.
 *
 * HSE_ENABLE_PUBLISH_KEY_STORE_RAM_TO_FLASH_ATTR_ID: this attribute is used to CNF the update of NVM keys both at mirror secure ram and data flash
 * When CNF as HSE_CNF_NO, NVM keys are updated at both the memory location, so when performing the import operation of keys time taken would be more
 * when CNF as HSE_CNF_YES, in which keys are updated at only secure RAM memory whose time would be much less when import operations are performed.
 *
 * When CNF as HSE_CNF_YES, for updating the keys to data flash we have to pass the HSE_SRV_ID_PUBLISH_NVM_KEYSTORE_RAM_TO_FLASH by using HSE_Publish_Keystore_RamtoFlash.
 *
 */

hseSrvResponse_t HSE_publish_keystore_RamToFlash_example_(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /*
    HSE_ENABLE_PUBLISH_KEY_STORE_RAM_TO_FLASH_ATTR_ID set as HSE_CFG_NO( by default)
    */
    hsePublishNvmKeystoreRamtToFlash_t publish_nvm_key_cfg = HSE_CFG_NO;
    srvResponse = SetAttr(HSE_ENABLE_PUBLISH_KEY_STORE_RAM_TO_FLASH_ATTR_ID, sizeof(hsePublishNvmKeystoreRamtToFlash_t), (void *) (&publish_nvm_key_cfg));
    ASSERT(HSE_SRV_RSP_OK == srvResponse);
    srvResponse = ImportNVMkeysAPI(); //importing the NVM keys
    ASSERT(HSE_SRV_RSP_OK == srvResponse);


    /*
     Publish NVM keys(hsePublishNvmKeystoreRamtToFlash_t) are configured yes, so now NVM keys will be updated only at secure RAM
     To make the NVM keys update at data flash we have to pass the HSE_SRV_ID_PUBLISH_NVM_KEYSTORE_RAM_TO_FLASH.
    */
    hsePublishNvmKeystoreRamtToFlash_t y = HSE_CFG_YES;
    srvResponse = SetAttr(HSE_ENABLE_PUBLISH_KEY_STORE_RAM_TO_FLASH_ATTR_ID, sizeof(hsePublishNvmKeystoreRamtToFlash_t), (void *) (&y));
    ASSERT(HSE_SRV_RSP_OK == srvResponse);


    srvResponse = ImportNVMkeysAPI(); //importing the NVM keys
    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    /* Now pass the Publish NVM key service Id, to make the keys update at data flash also*/
    srvResponse = HSE_Publish_Keystore_RamtoFlash(); // This will now update the keys to data flash also
    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    return PUBLISH_KEYSTORE_RAM_TO_FLASH_SUCCESSFUL;

}
#endif /* defined (HSE_SPT_PUBLISH_NVM_KEYSTORE_RAM_TO_FLASH) */
