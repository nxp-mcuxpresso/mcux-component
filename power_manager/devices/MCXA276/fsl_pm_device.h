/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_PM_DEVICE_H_
#define _FSL_PM_DEVICE_H_

#include "fsl_common.h"
#include "fsl_pm_config.h"

/*!
 * @addtogroup PM Framework: Power Manager Framework
 * @brief This section includes Power Mode macros, System Constraints macros, and Wakeup source macros.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @name Power Mode Definition
 * @{
 */
#define PM_LP_STATE_SLEEP              (0U)
#define PM_LP_STATE_DEEP_SLEEP         (1U)
#define PM_LP_STATE_POWER_DOWN         (2U)
#define PM_LP_STATE_DEEP_POWER_DOWN    (3U)
#define PM_LP_STATE_NO_CONSTRAINT      (0xFFU)
/*!
 * @} 
 */

/*! @brief Helper macros
 * @{
 */
#define PM_RESC_MASK(resc_masks, rescIndex)     (((resc_masks)->rescMask[(rescIndex) / 32UL] >> ((rescIndex) % 32UL)) & 1UL)
#define PM_RESC_GROUP(resc_groups, rescIndex)   ((resc_groups)->groupSlice[(rescIndex) / 8UL] >> (4UL * ((rescIndex) % 8UL))) & 0xFUL
/*!
 * @} 
 */

/*! @brief Available constraints for resources
 * @{
 */
typedef enum _resc_name
{
    kResc_Fro_16k,
    kResc_RamRetentionLdo,
    kResc_Core_Vdd_LVd_Act,
    kResc_Core_Vdd_LVd_Lp,
    kResc_Sys_Vdd_LVd,
    kResc_Sys_Vdd_Hvd,
    kResc_Lpcac,
    kResc_FlashArray,
    kResc_SramBlock0,   /* SRAM block X0/X1, B0/B1 */
    kResc_SramBlock1,   /* SRAM block A0 */
    kResc_SramBlock2,   /* SRAM block A1~A4 */
    kResc_SramBlock3,   /* SRAM block C0-C3 */

    kResc_Max_Num = 12U,
} resc_name_t;

/*!
 *  |-----------------------|----------|-------------|----------------|----------------|
 *  | Module                |  Sleep   |  Deep Sleep |  Power Down    |  Deep PDown    |
 *  |-----------------------|----------|-------------|----------------|----------------|
 *  | kResc_Fro_16k         |  On/Off  |  On/Off     |  On/Off        |  On/Off        |
 *  | kResc_RamRetentionLdo |  On/Off  |  On/Off     |  On/Off        |  On/Off        |
 *  | kResc_Core_Vdd_LVd_Act|  On/Off  |  On/Off     |  Off           |  Off           |
 *  | kResc_Core_Vdd_LVd_Lp |  On/Off  |  On/Off     |  Off           |  Off           |
 *  | kResc_Sys_Vdd_LVd     |  On/Off  |  On/Off     |  Off           |  Off           |
 *  | kResc_Sys_Vdd_Hvd     |  On/Off  |  On/Off     |  Off           |  Off           |
 *  | kResc_Lpcac           |  Static  |  Static     |  Static        |  Off           |
 *  | kResc_FlashArray      |  On      |  Static     |  Off           |  Off           |
 *  | kResc_SramBlock0      |  On      |  Retention  |  Retention/Off |  Retention/Off |
 *  | kResc_SramBlock1      |  On      |  Retention  |  Retention/Off |  Retention/Off |
 *  | kResc_SramBlock2      |  On      |  Retention  |  Retention/Off |  Retention/Off |
 *  | kResc_SramBlock3      |  On      |  Retention  |  Retention/Off |  Retention/Off |
 *  |-----------------------|----------|-------------|----------------|----------------|
 */

/*!
 * @brief Helper macros for Resource Contraint Masks
 */
/* Deep power down */
#define PM_RESC_MASK_DEEP_POWER_DOWN0   ((1UL << kResc_Fro_16k) | (1UL << kResc_RamRetentionLdo) | \
                                         (1UL << kResc_SramBlock0) | (1UL << kResc_SramBlock1) | \
                                         (1UL << kResc_SramBlock2) |  (1UL << kResc_SramBlock3))

/* Power down */
#define PM_RESC_MASK_POWER_DOWN0 (PM_RESC_MASK_DEEP_POWER_DOWN0 | (1UL << kResc_Lpcac))

/* Deep sleep */
#define PM_RESC_MASK_DEEP_SLEEP0 (PM_RESC_MASK_POWER_DOWN0 | (1UL << kResc_FlashArray) | \
                                  (1UL << kResc_Core_Vdd_LVd_Act) | (1UL << kResc_Core_Vdd_LVd_Lp) | \
                                  (1UL << kResc_Sys_Vdd_LVd) | (1UL << kResc_Sys_Vdd_Hvd))

/* Sleep */
#define PM_RESC_MASK_SLEEP0 PM_RESC_MASK_DEEP_SLEEP0

/*!
 * @brief Set resources in active, retain, power-off mode.
 */
#define PM_RESC_FRO_16K_ACTIVE                  PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Fro_16k)
#define PM_RESC_FRO_16K_POWEROFF                PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Fro_16k)

#define PM_RESC_RETENTION_LDO_ACTIVE            PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamRetentionLdo)
#define PM_RESC_RETENTION_LDO_POWEROFF          PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamRetentionLdo)

#define PM_RESC_CORE_VDD_LVD_ACT_ACTIVE         PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Core_Vdd_LVd_Act)
#define PM_RESC_CORE_VDD_LVD_ACT_POWEROFF       PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Core_Vdd_LVd_Act)

#define PM_RESC_CORE_VDD_LVD_LP_ACTIVE          PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Core_Vdd_LVd_Lp)
#define PM_RESC_CORE_VDD_LVD_LP_POWEROFF        PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Core_Vdd_LVd_Lp)

#define PM_RESC_SYS_VDD_LVD_ACTIVE      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Sys_Vdd_LVd)
#define PM_RESC_SYS_VDD_LVD_POWEROFF    PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Sys_Vdd_LVd)

#define PM_RESC_SYS_VDD_HVD_ACTIVE      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Sys_Vdd_Hvd)
#define PM_RESC_SYS_VDD_HVD_POWEROFF    PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Sys_Vdd_Hvd)

#define PM_RESC_LPCAC_ACTIVE            PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Lpcac)
#define PM_RESC_LPCAC_POWEROFF          PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Lpcac)

#define PM_RESC_FLASH_ARRAY_ACTIVE      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_FlashArray)
#define PM_RESC_FLASH_ARRAY_STATIC      PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_FlashArray)
#define PM_RESC_FLASH_ARRAY_POWEROFF    PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_FlashArray)

#define PM_RESC_SRAM_BLOCK0_ACTIVE      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_SramBlock0)
#define PM_RESC_SRAM_BLOCK0_STATIC      PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_SramBlock0)
#define PM_RESC_SRAM_BLOCK0_POWEROFF    PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_SramBlock0)

#define PM_RESC_SRAM_BLOCK1_ACTIVE      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_SramBlock1)
#define PM_RESC_SRAM_BLOCK1_STATIC      PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_SramBlock1)
#define PM_RESC_SRAM_BLOCK1_POWEROFF    PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_SramBlock1)

#define PM_RESC_SRAM_BLOCK2_ACTIVE      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_SramBlock2)
#define PM_RESC_SRAM_BLOCK2_STATIC      PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_SramBlock2)
#define PM_RESC_SRAM_BLOCK2_POWEROFF    PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_SramBlock2)

#define PM_RESC_SRAM_BLOCK3_ACTIVE      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_SramBlock3)
#define PM_RESC_SRAM_BLOCK3_STATIC      PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_SramBlock3)
#define PM_RESC_SRAM_BLOCK3_POWEROFF    PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_SramBlock3)
/*!
 * @} 
 */

#if FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER

/*!
 * @name Wakeup enumerations
 * @{
 */
typedef enum _wakeup_input_src_type
{
    kWup_ModInt,                /*!< Internal peripheral interrupt */
} wakeup_input_src_type_t;

typedef enum _wakeup_mod_int_id
{
    kWup_mod_int_lptmr  = 6U,   /*!< lptmr */
} wakeup_mod_int_id_t;

/*!
 * @} 
 */

/*!
 * @name System Wakeup source definitions.
 * @{
 */
/*!
 * @brief Enable on-chip perihperals interrupt as the wakeup source.
 */
#define PM_WSID_INT_LPTMR           PM_ENCODE_WAKEUP_SOURCE_ID(kWup_ModInt, kWup_mod_int_lptmr, LPTMR0_IRQn, 0UL)

/*!
 * @} 
 */
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*******************************************************************************
 * APIs
 ******************************************************************************/
/*!
 * @} 
 */

#endif /* _FSL_PM_DEVICE_H_ */
