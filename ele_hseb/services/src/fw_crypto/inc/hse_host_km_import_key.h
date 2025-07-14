/**
*   @file    hse_host_km_import_key.h
*
*   @brief   This file implements wrappers for key import.
*
*   @addtogroup [HOST_FRAMEWORK]
*   @{
*/
/*==================================================================================================
*
*   Copyright 2019-2021 NXP
*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/
#ifndef HSE_HOST_KM_IMPORT_KEY_H
#define HSE_HOST_KM_IMPORT_KEY_H

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
 * 1) system and project includes
 * 2) needed interfaces from external units
 * 3) internal and external interfaces from this unit
==================================================================================================*/

#include "hse_interface.h"

/*==================================================================================================
 *                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/

/*==================================================================================================
 *                                     FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
 *                                          CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      DEFINES AND MACROS
==================================================================================================*/

/*==================================================================================================
 *                                             ENUMS
==================================================================================================*/

/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
                                 GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/*************************************************************************************************
* Description:  Imports a key given the key handle, key info and key value(s)
************************************************************************************************/
hseSrvResponse_t HSE_ImportKey
(
    uint8_t u8MuInstance,
    hseKeyHandle_t targetKeyHandle,
    hseKeyInfo_t *pKeyInfo,
    const uint8_t *pKey0, uint32_t keyLen0,
    const uint8_t *pKey1, uint32_t keyLen1,
    const uint8_t *pKey2, uint32_t keyLen2
    #if defined(HSE_KEY_FORMAT_ECC_PUB_RAW)
    ,hseEccKeyFormat_t eccKeyFormat
    #endif
);


/*************************************************************************************************
* Description:  Imports a symmetric key given the key handle, type (AES/HMAC) and key flags
************************************************************************************************/
hseSrvResponse_t HSE_ImportSymKey
(
    hseKeyHandle_t handle,
    hseKeyType_t type,
    hseKeyFlags_t flags,
    const uint8_t *pKey,
    uint32_t len
);

/*************************************************************************************************
* Description:  Imports an ECC key (pub/pair) given the key handle, type (PUB/PAIR), flags and curve
************************************************************************************************/
hseSrvResponse_t HSE_ImportEccKey
(
    hseKeyHandle_t handle,
    hseKeyType_t type,
    hseKeyFlags_t flags,
    hseEccCurveId_t eccCurveId,
    const uint8_t* pPubKey,
    const uint8_t* pPrivKey
    #if defined(HSE_KEY_FORMAT_ECC_PUB_RAW)
    ,hseEccKeyFormat_t eccKeyFormat
    #endif
);

/*************************************************************************************************
* Description:  Imports a RSA key (pub/pair) given the key handle, type (PUB/PAIR), flags and public exponent length
************************************************************************************************/
hseSrvResponse_t HSE_ImportRsaKey
(
    hseKeyHandle_t handle,
    hseKeyType_t type,
    hseKeyFlags_t flags,
    const uint8_t* pN,
    uint16_t modLen,
    const uint8_t* pE,
    uint16_t eLen,
    const uint8_t* pD
);

/*************************************************************************************************
* Description:  Imports a Symmetric key in an authenticated key container
************************************************************************************************/
hseSrvResponse_t HSE_ImportAuthSymKey
(
    const hseKeyHandle_t targetHandle,
    const hseKeyHandle_t authHandle,
    const hseAuthScheme_t *pAuthScheme,
    const uint8_t *pKeyContainer,
    const uint16_t containerLen,
    const uint16_t keyInfoOffset,
    const uint16_t keyDataOffset,
    const uint16_t keyLen,
    const uint8_t *pAuth0,
    const uint8_t *pAuth1,
    const uint16_t authLen0,
    const uint16_t authLen1
);

#ifdef __cplusplus
}
#endif

#endif /* HSE_HOST_KM_IMPORT_KEY_H */

/** @} */
