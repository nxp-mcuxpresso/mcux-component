/*
 * @file hse_get_key_info.c
 *
 * @brief This file contains functions to get the information about the keys
 *        For details of the function refer its declaration in hse_get_key_info.h
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
#include "hse_get_key_info.h"
/*
 * ============================================================================
 *                               GLOBAL VARIABLES
 * ============================================================================
*/

/*
 * @brief variable to store the key info when HSE_GET_KEY_INFO service is called
 */
hseKeyInfo_t KeyInfo = {0};

/*
 * ============================================================================
 *                               LOCAL FUNCTIONS DEFINATIONS
 * ============================================================================
*/

/* @function:      HSE_Get_Key_Info_Example
 * @description:  Function to get the information about the Key by passing its handle
 *                This API will import the keys of AES-256 and then get the key values
 *                using its handle.
 * @returns:
 * HSE_SRV_RSP_OK                    HSE service successfully executed with no error
 * HSE_SRV_RSP_GENERAL_ERROR         This error code is returned if an error not covered by the
 *                                    error codes above is detected inside HSE
 *
 */
hseSrvResponse_t HSE_Get_Key_Info_Example(void)
{
    hseKeyHandle_t aesEcbKeyHandle = HSE_DEMO_RAM_AES256_KEY2;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    (void) srvResponse;

    srvResponse = ImportPlainSymKeyReq(aesEcbKeyHandle, HSE_KEY_TYPE_AES,
                                      (HSE_KF_USAGE_ENCRYPT) ,  aesEcbKeyLength, aesEcbKey, 1);
    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    srvResponse = GetKeyInfo(aesEcbKeyHandle, &KeyInfo);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }
    else
    {
        if(((aesEcbKeyLength *8) != KeyInfo.keyBitLen)          ||
               (HSE_KF_USAGE_ENCRYPT != KeyInfo.keyFlags)       ||
               (HSE_KEY_TYPE_AES != KeyInfo.keyType)            ||
               (HSE_RAM_KEY_COUNTER_VALUE != KeyInfo.keyCounter) ||
               (HSE_SMR_VALUE_RAM_KEYS != KeyInfo.smrFlags))
        {
            srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
        }
        goto exit;
    }

exit:
      return srvResponse;
}

/*
 * @function HSE_Get_Key_Info
 * @description: Function to get the information of any key by just passing its handle
 *
 * @parameters:
 *             KeyHandle: key handle reference
 *             KeyInfo: pointer variable of type hseKeyInfo which will store the
 *                      information about the key.
 * @return HSE Service response is returned
 */
hseSrvResponse_t HSE_Get_Key_Info(hseKeyHandle_t KeyHandle, hseKeyInfo_t *KeyINFO)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    srvResponse = GetKeyInfo(KeyHandle, KeyINFO);
    return srvResponse;
}
