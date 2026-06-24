/**
 *   @file    hse_keys_allocator.h
 *
 *   @brief   Function definitions for host keys allocator
 *   @details This file will help with handling of key handles.
 *
 *   @addtogroup [KEYMGMT_FRAMEWORK]
 *   @{
 */
/*--------------------------------------------------------------------------*/
/* Copyright 2020 NXP                                                       */
/*                                                                          */
/* License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025 */
/*--------------------------------------------------------------------------*/
/*==================================================================================================
==================================================================================================*/


#ifndef HSE_KEYS_ALLOCATOR_H
#define HSE_KEYS_ALLOCATOR_H

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
 *                                        INCLUDE FILES
==================================================================================================*/
#include "hse_interface.h"

/*==================================================================================================
 *                                          CONSTANTS
==================================================================================================*/

/*==================================================================================================
 *                                      DEFINES AND MACROS
==================================================================================================*/
#define NVM_KEY 1U
#define RAM_KEY 0U

/*==================================================================================================
 *                                             ENUMS
==================================================================================================*/

/*==================================================================================================
 *                                STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
 *                                GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
 *                                    FUNCTION PROTOTYPES
==================================================================================================*/
/* Does not check for NULL pointers */
hseSrvResponse_t HKF_Init(
    const hseKeyGroupCfgEntry_t *pNvmCatalog,
    const hseKeyGroupCfgEntry_t *pRamCatalog
);

hseSrvResponse_t HKF_AllocKeySlotAdvanced(
    uint8_t isNvmKey,
    hseMuMask_t muMask,
    bool_t strictMuMask,
    hseKeyGroupOwner_t groupOwner,
    hseKeyType_t keyType,
    uint16_t maxKeyBitLength,
    hseKeyHandle_t *pKeyHandle
);

hseSrvResponse_t HKF_AllocKeySlot(
    bool_t isNvmKey,                /* IN  */
    hseKeyType_t keyType,           /* IN  */
    uint16_t maxKeyBitLength,       /* IN  */
    hseKeyHandle_t *pKeyHandle      /* OUT */
);

hseSrvResponse_t HKF_FreeKeySlot(
    hseKeyHandle_t *keyHandle       /* IN */
);

hseSrvResponse_t HKF_IsKeyHandleAllocated(
    hseKeyHandle_t keyHandle       /* IN */
);

void HKF_FreeAllKeys(void);

hseSrvResponse_t HKF_MarkAsAllocated(
    hseKeyHandle_t keyHandle       /* IN */
);

#ifdef __cplusplus
}
#endif

#endif /* HSE_KEYS_ALLOCATOR_H */

/** @} */
