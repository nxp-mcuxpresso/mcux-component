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
 * Variables
 ******************************************************************************/
/*! @brief Available resources status. */
enum _resources_status
{
    kResc_Status_On = 0U,
    kResc_Status_Lp,
    KReac_Status_Hold,
    kResc_Status_Off,
};

/*! @brief Device power mode. */
enum _power_mode
{
    kResc_PowerMode_Sleep = 0U,
    kResc_PowerMode_DeepSleep,
    kResc_PowerMode_PowerDown,
    kResc_PowerMode_DeepPowerDown,

    kResc_PowerMode_Max,
};

extern uint8_t rescArray[PM_CONSTRAINT_COUNT][kResc_PowerMode_Max];
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!
 * @name Power Mode Definition
 * @{
 */
/* Power Mode Index */
#define PM_LP_STATE_SLEEP              (0U)
#define PM_LP_STATE_DEEP_SLEEP         (1U)
#define PM_LP_STATE_POWER_DOWN         (2U)
#define PM_LP_STATE_DEEP_POWER_DOWN    (3U)
#define PM_LP_STATE_NO_CONSTRAINT      (0xFFU)
/*! @} */

/*! @brief Helper macros
 * @{
 */
#define PM_RESC_MASK(resc_masks, rescIndex)     (((resc_masks)->rescMask[(rescIndex) / 32UL] >> ((rescIndex) % 32UL)) & 1UL)
#define PM_RESC_GROUP(resc_groups, rescIndex)   ((resc_groups)->groupSlice[(rescIndex) / 8UL] >> (4UL * ((rescIndex) % 8UL))) & 0xFUL
/*! @} */

/*! @brief Available constraints for resources
 * @{
 */
typedef enum _resc_name
{
    /* System modules */
    kResc_CodeCache,
    kResc_Fro_180M,
    kResc_Fro_12M,
    kResc_Fro_16K,
    kResc_Osc_Rtc,
    kResc_Osc_Sys,
    kResc_Apll,
    kResc_Spll,

    /* Power domain modules */
    kResc_LdoCore,
    kResc_RamRetentionLdo,
    kResc_Core_Vdd_Ivs,
    kResc_Core_Vdd_Lvd_Act,
    kResc_Core_Vdd_Hvd_Act,
    kResc_Sys_Vdd_Lvd_Act,
    kResc_Sys_Vdd_Hvd_Act,
    kResc_Io_Vdd_Lvd_Act,
    kResc_Io_Vdd_Hvd_Act,
    kResc_Core_Vdd_Lvd_Lp,
    kResc_Core_Vdd_Hvd_Lp,
    kResc_Sys_Vdd_Lvd_Lp,
    kResc_Sys_Vdd_Hvd_Lp,
    kResc_Io_Vdd_Lvd_Lp,
    kResc_Io_Vdd_Hvd_Lp,

    /* Memory modules */
    kResc_RamA0,
    kResc_RamA1,
    kResc_RamA2,
    kResc_RamA3,
    kResc_RamX0,
    kResc_RamX1,
    kResc_RamX2,
    kResc_RamB0,
    kResc_RamC0,
    kResc_RamC1,
    kResc_RamD0,
    kResc_RamD1,
    kResc_RamE0,
    kResc_RamE1,

    kResc_Max_Num = 37U,
} resc_name_t;
/*! @} */

/*!
 * @name System basic resource constraints definitions used by application.
 * @{
 */
/* System modules */
#define PM_RESC_CODE_CACHE_HOLD         PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_CodeCache)
#define PM_RESC_CODE_CACHE_OFF          PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_CodeCache)

#define PM_RESC_FRO_180M_ON              PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Fro_180M)
#define PM_RESC_FRO_180M_OFF             PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Fro_180M)

#define PM_RESC_FRO_12M_ON              PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Fro_12M)
#define PM_RESC_FRO_12M_OFF             PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Fro_12M)

#define PM_RESC_FRO_16K_ON              PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Fro_16K)
#define PM_RESC_FRO_16K_OFF             PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Fro_16K)

#define PM_RESC_OSC_RTC_ON              PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Osc_Rtc)
#define PM_RESC_OSC_RTC_OFF             PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Osc_Rtc)

#define PM_RESC_OSC_SYC_ON              PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Osc_Sys)
#define PM_RESC_OSC_SYC_OFF             PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Osc_Sys)

#define PM_RESC_APLL_ON                 PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Apll)
#define PM_RESC_APLL_OFF                PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Apll)

#define PM_RESC_SPLL_ON                 PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Spll)
#define PM_RESC_SPLL_OFF                PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Spll)

/* Power domain modules */
#define PM_RESC_LDO_CORE_ON             PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_LdoCore)
#define PM_RESC_LDO_CORE_OFF            PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_LdoCore)

#define PM_RESC_RETENTION_LDO_ON        PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamRetentionLdo)
#define PM_RESC_RETENTION_LDO_OFF       PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamRetentionLdo)

#define PM_RESC_CORE_VDD_IVS_ON         PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Core_Vdd_Ivs)
#define PM_RESC_CORE_VDD_IVS_OFF        PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Core_Vdd_Ivs)

#define PM_RESC_CORE_VDD_LVD_ACT_ON     PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Core_Vdd_Lvd_Act)
#define PM_RESC_CORE_VDD_LVD_ACT_OFF    PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Core_Vdd_Lvd_Act)

#define PM_RESC_CORE_VDD_HVD_ACT_ON     PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Core_Vdd_Hvd_Act)
#define PM_RESC_CORE_VDD_HVD_ACT_OFF    PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Core_Vdd_Hvd_Act)

#define PM_RESC_SYS_VDD_LVD_ACT_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Sys_Vdd_Lvd_Act)
#define PM_RESC_SYS_VDD_LVD_ACT_OFF     PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Sys_Vdd_Lvd_Act)

#define PM_RESC_SYS_VDD_HVD_ACT_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Sys_Vdd_Hvd_Act)
#define PM_RESC_SYS_VDD_HVD_ACT_OFF     PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Sys_Vdd_Hvd_Act)

#define PM_RESC_IO_VDD_LVD_ACT_ON       PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Io_Vdd_Lvd_Act)
#define PM_RESC_IO_VDD_LVD_ACT_OFF      PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Io_Vdd_Lvd_Act)

#define PM_RESC_IO_VDD_HVD_ACT_ON       PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Io_Vdd_Hvd_Act)
#define PM_RESC_IO_VDD_HVD_ACT_OFF      PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Io_Vdd_Hvd_Act)

#define PM_RESC_CORE_VDD_LVD_LP_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Core_Vdd_Lvd_Lp)
#define PM_RESC_CORE_VDD_LVD_LP_OFF     PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Core_Vdd_Lvd_Lp)

#define PM_RESC_CORE_VDD_HVD_LP_ON      PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Core_Vdd_Hvd_Lp)
#define PM_RESC_CORE_VDD_HVD_LP_OFF     PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Core_Vdd_Hvd_Lp)

#define PM_RESC_SYS_VDD_LVD_LP_ON       PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Sys_Vdd_Lvd_Lp)
#define PM_RESC_SYS_VDD_LVD_LP_OFF      PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Sys_Vdd_Lvd_Lp)

#define PM_RESC_SYS_VDD_HVD_LP_ON       PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Sys_Vdd_Hvd_Lp)
#define PM_RESC_SYS_VDD_HVD_LP_OFF      PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Sys_Vdd_Hvd_Lp)

#define PM_RESC_IO_VDD_LVD_LP_ON        PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Io_Vdd_Lvd_Lp)
#define PM_RESC_IO_VDD_LVD_LP_OFF       PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Io_Vdd_Lvd_Lp)

#define PM_RESC_IO_VDD_HVD_LP_ON        PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Io_Vdd_Hvd_Lp)
#define PM_RESC_IO_VDD_HVD_LP_OFF       PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Io_Vdd_Hvd_Lp)

/* SRAM modules */
#define PM_RESC_RAM_A0_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamA0)
#define PM_RESC_RAM_A0_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamA0)
#define PM_RESC_RAM_A0_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamA0)

#define PM_RESC_RAM_A1_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamA1)
#define PM_RESC_RAM_A1_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamA1)
#define PM_RESC_RAM_A1_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamA1)

#define PM_RESC_RAM_A2_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamA2)
#define PM_RESC_RAM_A2_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamA2)
#define PM_RESC_RAM_A2_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamA2)

#define PM_RESC_RAM_A3_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamA3)
#define PM_RESC_RAM_A3_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamA3)
#define PM_RESC_RAM_A3_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamA3)

#define PM_RESC_RAM_X0_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamX0)
#define PM_RESC_RAM_X0_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamX0)
#define PM_RESC_RAM_X0_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamX0)

#define PM_RESC_RAM_X1_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamX1)
#define PM_RESC_RAM_X1_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamX1)
#define PM_RESC_RAM_X1_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamX1)

#define PM_RESC_RAM_X2_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamX2)
#define PM_RESC_RAM_X2_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamX2)
#define PM_RESC_RAM_X2_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamX2)

#define PM_RESC_RAM_B0_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamB0)
#define PM_RESC_RAM_B0_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamB0)
#define PM_RESC_RAM_B0_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamB0)

#define PM_RESC_RAM_C0_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamC0)
#define PM_RESC_RAM_C0_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamC0)
#define PM_RESC_RAM_C0_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamC0)

#define PM_RESC_RAM_C1_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamC1)
#define PM_RESC_RAM_C1_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamC1)
#define PM_RESC_RAM_C1_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamC1)

#define PM_RESC_RAM_D0_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamD0)
#define PM_RESC_RAM_D0_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamD0)
#define PM_RESC_RAM_D0_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamD0)

#define PM_RESC_RAM_D1_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamD1)
#define PM_RESC_RAM_D1_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamD1)
#define PM_RESC_RAM_D1_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamD1)

#define PM_RESC_RAM_E0_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamE0)
#define PM_RESC_RAM_E0_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamE0)
#define PM_RESC_RAM_E0_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamE0)

#define PM_RESC_RAM_E1_ON               PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_RamE1)
#define PM_RESC_RAM_E1_HOLD             PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_RamE1)
#define PM_RESC_RAM_E1_OFF              PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_RamE1)
/*! @} */


#if FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER
/*!
 * @name Wakeup enumerations
 * @{
 */
typedef enum _wakeup_input_src_type
{
    kWup_ModInt,            /*!< Internal peripheral interrupt */
} wakeup_input_src_type_t;


typedef enum _wakeup_mod_int_id
{
    kWup_ModIntLptmr = 6U,  /*!< LPTMR0 */
} wakeup_mod_int_id_t;

/*! @} */

/*!
 * @name System Wakeup source definitions.
 * @{
 */
/*!
 * @brief Enable on-chip perihperals interrupt as the wakeup source.
 */
#define PM_WSID_LPTMR0 PM_ENCODE_WAKEUP_SOURCE_ID(kWup_ModInt, kWup_ModIntLptmr, LPTMR0_IRQn, 0UL)

/*! @} */
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*******************************************************************************
 * APIs
 ******************************************************************************/
void ConstructResourceInformation(uint8_t rescArray[][kResc_PowerMode_Max]);

/*! @} */

#endif /* _FSL_PM_DEVICE_H_ */
