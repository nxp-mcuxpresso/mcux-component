/*
 * hse_UpdateNvmKeys_example.c
 *
  @brief Source file for Update NVM keys example in crypto services
 * @details Contains the API's for demonstrating the use of Update NVM keys crypto service in DemoApp
 *
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

#include "hse_UpdateNvmKeys_example.h"

#ifdef HSE_SPT_GMAC
/*==================================================================================================
                               LOCAL CONSTANTS
==================================================================================================*/

/*
 * @brief HMAC key 1 - initial and second version - Update NVM key example
 */
static const uint8_t hmacKeyUpdated[] =
{ 0xf8, 0x69, 0x02, 0xe5, 0xe5, 0xdb, 0x47, 0x8e, 0xc6, 0xe2, 0x78, 0x69, 0x27,
        0x28, 0xa8, 0x12, 0xc4, 0xcd, 0x87, 0x45, 0xf9, 0x0a, 0x7d, 0x9f, 0x79,
        0x15, 0xf5, 0xa9, 0x43, 0x45, 0xfc, 0x12, 0xd2, 0x77, 0x0a, 0x3c, 0x94,
        0xb0, 0x1f, 0xfb, 0x9e, 0x04, 0x12, 0x99, 0x9e, 0xb6, 0x26, 0x1d, 0x11,
        0xa0
};

static const uint8_t iv[] =
{ 0xff, 0xbc, 0x51, 0x6a, 0x8f, 0xbe, 0x61, 0x52, 0xaa, 0x42, 0x8c, 0xdd, 0x80,
        0x0c, 0x06, 0x2d
};
/*
 * ============================================================================
 *                             FUNCTIONS DEFINITION
 * ============================================================================
*/

/******************************************************************************
 * Function:    HSE_UpdateNvmKey_Example
 * Description: Example of how to update a NVM key with SU rights
 *              (must be imported in an authenticated key container)
 *              Import RSA keys, Authorization Request/Response,
 *              RSA Sign generation, Erase Key
 *****************************************************************************/
hseSrvResponse_t HSE_UpdateNvmKey_Example(void)
{
    hseSrvResponse_t hseResponse;
     /*Declare the HMAC key info that will update initial version*/
    hseKeyInfo_t keyInfo = {
        .keyType = HSE_KEY_TYPE_HMAC,                        //Will generate a HMAC key
        .keyFlags = (HSE_KF_USAGE_SIGN|HSE_KF_USAGE_VERIFY), //Usage flags for this key - Sign/Verify
        .keyBitLen = BYTES_TO_BITS(NUM_OF_ELEMS(hmacKeyUpdated)), //Key size in bits
        .keyCounter = 1U                                     //The key counter must be incremented(greater than current counter of the key in NVM slot)*/
    };

    /*
     * Declare the authentication scheme that HSE will use
     * (along with the provision key) to verify the authenticity
     */
    /*    hseAuthScheme_t authScheme = {
    -         .macScheme.macAlgo = HSE_MAC_ALGO_GMAC,
    -         .macScheme.sch.gmac.pIV = PTR_TO_HOST_ADDR(iv),
    -         .macScheme.sch.gmac.ivLength = NUM_OF_ELEMS(iv),
    -     };*/
    uint8_t tag[16U] = {0U};
    uint32_t tagLen = 16UL;

    /*
     * Declare a key container:
     * Authenticated buffer that MUST contain:
     *          - key data (i.e. pKey[0], pKey[1], pKey[2], depending on key type)
     *          - key info (which should have a higher key counter)
     *              - optional for public keys (RSA/ECC_PUB)
     * Optional, may contain any other information
     * For more details checkout `hse_srv_key_import_export.h` or dedicated chapter in HSE RM
     */
    uint8_t keyContainer[1056] = {0U};
    /*    uint16_t keyInfoOffset, keyDataOffset;*/
    uint16_t offset = 0U;

    /*
     * Populate the NVM HMAC key slot with initial key -
     * first import can be done in plain (SU rights)
     */
    /*hseResponse = ImportPlainSymKeyReq(NVM_HMAC_KEY0, HSE_KEY_TYPE_HMAC,
        HSE_KF_USAGE_SIGN, NUM_OF_ELEMS(hmacKeyInitial), hmacKeyInitial);
    if(HSE_SRV_RSP_OK != hseResponse)
        goto exit;*/


    /*
     * Fill the container with the necessary data
     * (Optional) - leave some space for any specific data
     */
    offset += 50U;

    //HMAC - Symmetric key - Must copy the key info
    //keyInfoOffset = offset;
    (void)memcpy(&keyContainer[offset], &keyInfo, sizeof(keyInfo));
    offset += sizeof(keyInfo);

    //(Optional) - leave some space for any specific data
    offset += 100U;

    //HMAC - Symmetric key - Must copy the key data in the container
    //keyDataOffset = offset;
    (void)memcpy(&keyContainer[offset], hmacKeyUpdated, NUM_OF_ELEMS(hmacKeyUpdated));

    /*
     * (Optional) - add any other data
     * Key container is ready - it must be authenticated
     * Import in RAM a copy of the AES provision key to sign the
     * container using HSE
     * NOTE: Intended use case: The container comes already signed
     * (/ is signed externally)
     */
    hseResponse = ImportPlainSymKeyReq(HSE_DEMO_RAM_AES128_KEY1, HSE_KEY_TYPE_AES,
        HSE_KF_USAGE_SIGN, gAESProvisionKeyLength, gAES128ProvisionKey,0U);
    if(HSE_SRV_RSP_OK != hseResponse)
        goto exit;

    //Sign the container - generate GMAC
    hseResponse = AesGmacGenerate(HSE_DEMO_RAM_AES128_KEY1, NUM_OF_ELEMS(iv), iv,
            NUM_OF_ELEMS(keyContainer), keyContainer, &tagLen, tag,HSE_SGT_OPTION_NONE);
    if(HSE_SRV_RSP_OK != hseResponse)
        goto exit;
exit:
    return hseResponse;
}
#endif
