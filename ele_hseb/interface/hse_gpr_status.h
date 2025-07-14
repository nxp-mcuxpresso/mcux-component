/**
*   @file    hse_gpr_status.h
*
*   @brief   HSE GPR Status
*   @details This file contains the HSE GPR Status
*
*   @addtogroup hse_gpr HSE GPR Status
*   @ingroup class_interface
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
/*==================================================================================================
==================================================================================================*/

#ifndef HSE_GPR_STATUS_H
#define HSE_GPR_STATUS_H

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/

#include "std_typedefs.h"
#include "hse_platform.h"

#define HSE_START_PRAGMA_PACK
#include "hse_compiler_abs.h"

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

#if defined(HSE_B)
    /** @brief    CONFIG_REG4 is in Configuration GPR Description (refer to #hseTamperConfigStatus_t and #hseEccErrorStatus_t).
                  This register is to inform the application that there is a loss of data due to ECC error and user needs necessary action.
     */
    #define HSE_GPR_STATUS_ADDRESS                 (0x4039C02CUL)

    /** @brief    HSE SYS-IMG Dataset Erase Status bits (register address is #HSE_GPR_STATUS_ADDRESS)
     *  @details  This status is updated when a particular dataset has been erased by HSE Firmware because of
     *            corruption due to ECC error (this can happen if a reset occurred during a flash programming/erase operation),
     *            or because some invalid data was found.
     */
    typedef uint32_t hseEccErrorStatus_t;

    #define HSE_ECC_KEYSTORE_STATUS                 ((hseEccErrorStatus_t)1U << 16U) /**< @brief CONFIG_REG4[16] - This bit is set when the key store area is erased. */
    #define HSE_ECC_CONFIG_DATA_STATUS              ((hseEccErrorStatus_t)1U << 17U) /**< @brief CONFIG_REG4[17] - This bit is set when the config data area is erased. */
    #define HSE_ECC_MONOTONIC_COUNTER_STATUS        ((hseEccErrorStatus_t)1U << 18U) /**< @brief CONFIG_REG4[18] - This bit is set when the monotonic counter area is erased. */
#endif /* HSE_B */

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

#define HSE_STOP_PRAGMA_PACK
#include "hse_compiler_abs.h"

#ifdef __cplusplus
}
#endif

#endif /* HSE_GPR_STATUS_H */

/** @} */
