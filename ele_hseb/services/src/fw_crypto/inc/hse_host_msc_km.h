/**
*   @file    hse_host_msc_km.h
*
*   @brief   This file implements wrappers for MSC(Managed Security Component) key management services.
*
*   @addtogroup [HOST_FRAMEWORK]
*   @{
*/
/*==================================================================================================
*
*   Copyright 2022 NXP
*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/
#ifndef HSE_HOST_MSC_KM_H
#define HSE_HOST_MSC_KM_H

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
* Description:  Configures the Key Handle Translation Table
************************************************************************************************/
hseSrvResponse_t HSE_ConfigKHTT(void);

/*************************************************************************************************
* Description:  Push one or more HSE keys in ACE key store
************************************************************************************************/
hseSrvResponse_t HSE_PushMscKey(hseKeyHandle_t keyHandle);

/*************************************************************************************************
* Description:  Get the associated MSC key slot index for the key handle
************************************************************************************************/
hseSrvResponse_t GetMscKeySlotIdxFromKhtt(hseKeyHandle_t keyHandle, uint8_t *pMscKeySlotIdx);

#ifdef __cplusplus
}
#endif

#endif /* HSE_HOST_MSC_KM_H */

/** @} */

