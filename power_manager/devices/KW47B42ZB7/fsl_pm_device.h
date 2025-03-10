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

/*!
 * @name Power Mode Definition
 * @{
 */

/* Power Mode Index */
#define PM_LP_STATE_SLEEP           (0U)
#define PM_LP_STATE_DEEP_SLEEP      (1U)
#define PM_LP_STATE_POWER_DOWN      (2U)
#define PM_LP_STATE_DEEP_POWER_DOWN (3U)
#define PM_LP_STATE_NO_CONSTRAINT   (0xFFU)

/*! @} */

/*!
 * @name System basic resource constraints definitions.
 * @{
 */

/*!
 * @brief The enumeration of basic resource name.
 * 
 */
enum _pm_resc_name
{
    kResc_CTCM0 = 0U,
    kResc_CTCM1 = 1U,
    kResc_STCM0 = 2U,
    kResc_STCM1 = 3U,
    kResc_STCM2 = 4U,
    kResc_STCM3 = 5U,
    kResc_STCM4 = 6U,
    kResc_STCM5 = 7U,
    kResc_STCM6 = 8U,
    kResc_STCM7 = 9U,
    kResc_STCM8 = 10U,
    kResc_Fro192M = 11U,
    kResc_Fro6M = 12U,
    kResc_WakePowerDomainPeri = 13U,
    
    /* Following resource is not configurable. */
    kResc_MainPowerDomainPeriOpt1 = 14U,
    kResc_MainPowerDomainPeriOpt2 = 15U,
    kResc_BusSysClk = 16U,
    kResc_CoreClk = 17U,
};

#define PM_CONFIGURABLE_RESOURCE_COUNT (14U)

#define PM_RESC_TO_BIT(resc) ((uint32_t)(1UL << (uint32_t)(resc)))

/* Constraints used by application. */
/*!
 * @name Define constraint of core clock.
 * @{
 */
#define PM_RESC_CORE_CLK_ON PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_CoreClk) /*!< Core clock must be on. */
/*! @} */

/*!
 * @name Define constraint of bus and system clock.
 * @{
 */
#define PM_RESC_BUS_SYS_CLK_ON PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_BusSysClk) /*!< Bus and system clock must be on. */
/*! @} */

/*!
 * @name Define constraints of CTCM0.
 * @{
 */
#define PM_RESC_CTCM0_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_CTCM0)   /*!< Set CTCM0 as Active mode. */
#define PM_RESC_CTCM0_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_CTCM0) /*!< Set CTCM0 as Deep Sleep. */
#define PM_RESC_CTCM0_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_CTCM0);    /*!< Set CTCM0 as power off mode. */
/*! @} */

/*!
 * @name Define constraints of CTCM1.
 * @{
 */
#define PM_RESC_CTCM1_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_CTCM1)   /*!< Set CTCM1 as Active mode. */
#define PM_RESC_CTCM1_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_CTCM1) /*!< Set CTCM1 as deep sleep. */
#define PM_RESC_CTCM1_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_CTCM1);    /*!< Set CTCM1 as power off mode. */
/*! @} */

/*!
 * @name Define constraints of STCM0.
 * @{
 */
#define PM_RESC_STCM0_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_STCM0) /*!< Set STCM0 as Active mode. */
#define PM_RESC_STCM0_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_STCM0) /*!< Set STCM0 as deep sleep. */
#define PM_RESC_STCM0_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_STCM0); /*!< Set STCM0 as power off. */
/*! @} */

/*!
 * @name Define constraints of STCM1.
 * @{
 */
#define PM_RESC_STCM1_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_STCM1)   /*!< Set STCM1 as Active mode. */
#define PM_RESC_STCM1_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_STCM1) /*!< Set STCM1 as deep sleep. */
#define PM_RESC_STCM1_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_STCM1) /*!< Set STCM1 as power off. */
/*! @} */


/*!
 * @name Define constraints of STCM2.
 * @{
 */
#define PM_RESC_STCM2_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_STCM2)   /*!< Set STCM2 as Active mode. */
#define PM_RESC_STCM2_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_STCM2) /*!< Set STCM2 as deep sleep. */
#define PM_RESC_STCM2_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_STCM2) /*!< Set STCM2 as power off. */
/*! @} */


/*!
 * @name Define constraints of STCM3.
 */
#define PM_RESC_STCM3_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_STCM3)   /*!< Set STCM3 as Active mode. */
#define PM_RESC_STCM3_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_STCM3) /*!< Set STCM3 as deep sleep. */
#define PM_RESC_STCM3_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_STCM3) /*!< Set STCM3 as power off. */
/*! @} */

/*!
 * @name Define constraints of STCM4.
 * @{
 */
#define PM_RESC_STCM4_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_STCM4)   /*!< Set STCM4 as Active mode. */
#define PM_RESC_STCM4_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_STCM4) /*!< Set STCM4 as deep sleep. */
#define PM_RESC_STCM4_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_STCM4) /*!< Set STCM4 as power off. */
/*! @} */

/*!
 * @name Define constraints of STCM5.
 * @{
 */
#define PM_RESC_STCM5_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_STCM5)   /*!< Set STCM5 as Active mode. */
#define PM_RESC_STCM5_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_STCM5) /*!< Set STCM5 as deep sleep. */
#define PM_RESC_STCM5_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_STCM5) /*!< Set STCM5 as power off. */
/*! @} */

/*!
 * @name Define constraints of STCM6.
 * @{
 */
#define PM_RESC_STCM6_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_STCM6)   /*!< Set STCM6 as Active mode. */
#define PM_RESC_STCM6_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_STCM6) /*!< Set STCM6 as deep sleep. */
#define PM_RESC_STCM6_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_STCM6) /*!< Set STCM6 as power off. */
/*! @} */

/*!
 * @name Define constraints of STCM7.
 * @{
 */
#define PM_RESC_STCM7_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_STCM7)   /*!< Set STCM7 as Active mode. */
#define PM_RESC_STCM7_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_STCM7) /*!< Set STCM7 as deep sleep. */
#define PM_RESC_STCM7_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_STCM7) /*!< Set STCM7 as power off. */
/*! @} */

/*!
 * @name Define constraints of STCM8.
 * @{
 */
#define PM_RESC_STCM8_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_STCM8)   /*!< Set STCM8 as Active mode. */
#define PM_RESC_STCM8_DEEPSLEEP PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_STCM8) /*!< Set STCM8 as deep sleep. */
#define PM_RESC_STCM8_POWEROFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_STCM8) /*!< Set STCM8 as power off. */
/*! @} */

/*!
 * @name Define constraints of FRO_192M
 * @{
 */
#define PM_RESC_FRO_192M_ON PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Fro192M)  /*!< Turn on FRO_192M. */
#define PM_RESC_FRO_192M_OFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Fro192M)     /*!< Turn off FRO_192M. */
/*! @} */


/*!
 * @name Define constraints of FRO_6M.
 * @{
 */
#define PM_RESC_FRO_6M_ON PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_Fro6M)  /*!< Turn on FRO_6M. */
#define PM_RESC_FRO_6M_OFF PM_ENCODE_RESC(PM_RESOURCE_OFF, kResc_Fro6M)     /*!< Turn off FRO_6M. */
/*! @} */

/*!
 * @name Define constraint of peripherals operational in CORE_MAIN power domain.
 * @{
 */
#define PM_RESC_MAIN_PD_PERI_OPERATIONAL \
    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_MainPowerDomainPeriOpt2)  /*!< Peripherals in CORE_MAIN are operational.*/
/*! @} */


/*!
 * @name Define constraint of peripheral state retention in CORE_MAIN power domain.
 * @{
 */
#define PM_RESC_MAIN_PD_PERI_STATE_RETENTION \
    PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_MainPowerDomainPeriOpt1) /*!< Peripherals in CORE_MAIN are state retention. */
/*! @} */

/*!
 * @name Define constraints of peripherals in CORE_WAKE power domain.
 * @{
 */
#define PM_RESC_WAKE_PD_PERI_ACTIVE PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, kResc_WakePowerDomainPeri)
#define PM_RESC_WAKE_PD_PERI_OPERATIONAL PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, kResc_WakePowerDomainPeri)
#define PM_RESC_WAKE_PD_PERI_STATE_RETENTION PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, kResc_WakePowerDomainPeri)
/*! @} */

/*! @} */

#if FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER

/*!
 * @name System Wakeup source definitions.
 * @{
 */

/*!
 * @name PTA0 as a wakeup source.
 * @{
 */
#define PM_WSID_PTA0_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 0UL, PORTA_EFT_IRQn, 1UL) /*!< Rising edge */
#define PM_WSID_PTA0_Falling_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 0UL, PORTA_EFT_IRQn, 2UL) /*!< Falling edge */
#define PM_WSID_PTA0_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 0UL, PORTA_EFT_IRQn, 3UL) /*!< Any edge */
/*! @} */


/*!
 * @name PTA2 as a wakeup source.
 * @{
 */
#define PM_WSID_PTA2_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 1UL, PORTA_EFT_IRQn, 1UL) /*!< Rising edge */
#define PM_WSID_PTA2_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 1UL, PORTA_EFT_IRQn, 2UL) /*!< Falling edge */
#define PM_WSID_PTA2_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 1UL, PORTA_EFT_IRQn, 3UL) /*!< Any edge */
/*! @} */

/*!
 * @name PTA4 as a wakeup source.
 * @{
 */
#define PM_WSID_PTA4_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 2UL, PORTA_EFT_IRQn, 1UL) /*!< Rising edge. */
#define PM_WSID_PTA4_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 2UL, PORTA_EFT_IRQn, 2UL) /*!< Falling edge. */
#define PM_WSID_PTA4_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 2UL, PORTA_EFT_IRQn, 3UL) /*!< Any edge. */
/*! @} */

/*!
 * @name PTA17 as a wakeup source.
 * @{
 */
#define PM_WSID_PTA17_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 3UL, PORTA_EFT_IRQn, 1UL) /*!< Rising edge. */
#define PM_WSID_PTA17_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 3UL, PORTA_EFT_IRQn, 2UL) /*!< Falling edge. */
#define PM_WSID_PTA17_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 3UL, PORTA_EFT_IRQn, 3UL) /*!< Any edge. */
/*! @} */


/*!
 * @name PTA19 as a wakeup source.
 * @{
 */
#define PM_WSID_PTA19_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 4UL, PORTA_EFT_IRQn, 1UL) /*!< Rising edge. */
#define PM_WSID_PTA19_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 4UL, PORTA_EFT_IRQn, 2UL) /*!< Falling edge. */
#define PM_WSID_PTA19_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 4UL, PORTA_EFT_IRQn, 3UL) /*!< Any edge. */
/*! @} */

/*!
 * @name PTA21 as a wakeup source.
 * @{ 
 */
#define PM_WSID_PTA21_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 5UL, PORTA_EFT_IRQn, 1UL) /*!< Rising edge. */
#define PM_WSID_PTA21_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 5UL, PORTA_EFT_IRQn, 2UL) /*!<  Falling edge. */
#define PM_WSID_PTA21_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 5UL, PORTA_EFT_IRQn, 3UL) /*!< Any edge. */
/*! @} */


/*!
 * @name PTA22 as a wakeup source.
 * @{
 */
#define PM_WSID_PTA22_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 6UL, PORTA_EFT_IRQn, 1UL)
#define PM_WSID_PTA22_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 6UL, PORTA_EFT_IRQn, 2UL)
#define PM_WSID_PTA22_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 6UL, PORTA_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name PTC0 as a wakeup source.
 * @{
 */
#define PM_WSID_PTC0_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 7UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC0_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 7UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC0_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 7UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC1 as a wakeup source.
 * @{
 */
#define PM_WSID_PTC1_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 8UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC1_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 8UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC1_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 8UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC2 as a wakeup source.
 * @{
 */
#define PM_WSID_PTC2_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 9UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC2_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 9UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC2_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 9UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC4 as a wakeup source.
 */
#define PM_WSID_PTC4_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 10UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC4_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 10UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC4_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 10UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC6 as a wakeup source.
 * @{
 */
#define PM_WSID_PTC6_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 11UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC6_FALLING_EDGE (uint32_t) PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 11UL, (uint32_t) PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC6_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 11UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC7 as a wakeup source.
 * @{
 */
#define PM_WSID_PTC7_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 12UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC7_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 12UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC7_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 12UL, PORTC_EFT_IRQns, 3UL)
/*! @} */

/*!
 * @name Enable PTB0 as a wakeup source.
 * @{
 */
#define PM_WSID_PTB0_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 13UL, PORTB_EFT_IRQn, 1UL)
#define PM_WSID_PTB0_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 13UL, PORTB_EFT_IRQn, 2UL)
#define PM_WSID_PTB0_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 13UL, PORTB_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTB3 as a wakeup source.
 * @{
 */
#define PM_WSID_PTB3_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 14UL, PORTB_EFT_IRQn, 1UL)
#define PM_WSID_PTB3_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 14UL, PORTB_EFT_IRQn, 2UL)
#define PM_WSID_PTB3_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 14UL, PORTB_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTB4 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTB4_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 15UL, PORTB_EFT_IRQn, 1UL)
#define PM_WSID_PTB4_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 15UL, PORTB_EFT_IRQn, 2UL)
#define PM_WSID_PTB4_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 15UL, PORTB_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTB6 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTB6_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 16UL, PORTB_EFT_IRQn, 1UL)
#define PM_WSID_PTB6_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 16UL, PORTB_EFT_IRQn, 2UL)
#define PM_WSID_PTB6_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 16UL, PORTB_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTB8 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTB8_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 17UL, PORTB_EFT_IRQn, 1UL)
#define PM_WSID_PTB8_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 17UL, PORTB_EFT_IRQn, 2UL)
#define PM_WSID_PTB8_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 17UL, PORTB_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTA6 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTA6_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 18UL, PORTA_EFT_IRQn, 1UL)
#define PM_WSID_PTA6_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 18UL, PORTA_EFT_IRQn, 2UL)
#define PM_WSID_PTA6_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 18UL, PORTA_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTA16 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTA16_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 19UL, PORTA_EFT_IRQn, 1UL)
#define PM_WSID_PTA16_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 19UL, PORTA_EFT_IRQn, 2UL)
#define PM_WSID_PTA16_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 19UL, PORTA_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTA18 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTA18_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 20UL, PORTA_EFT_IRQn, 1UL)
#define PM_WSID_PTA18_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 20UL, PORTA_EFT_IRQn, 2UL)
#define PM_WSID_PTA18_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 20UL, PORTA_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTA26 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTA26_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 21UL, PORTA_EFT_IRQn, 1UL)
#define PM_WSID_PTA26_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 21UL, PORTA_EFT_IRQn, 2UL)
#define PM_WSID_PTA26_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 21UL, PORTA_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC5 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTC5_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 22UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC5_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 22UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC5_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 22UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC9 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTC9_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 23UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC9_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 23UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC9_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 23UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC12 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTC12_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 24UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC12_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 24UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC12_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 24UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC14 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTC14_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 25UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC14_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 25UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC14_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 25UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable PTC16 as a wakeup pin.
 * @{
 */
#define PM_WSID_PTC16_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 26UL, PORTC_EFT_IRQn, 1UL)
#define PM_WSID_PTC16_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 26UL, PORTC_EFT_IRQn, 2UL)
#define PM_WSID_PTC16_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 26UL, PORTC_EFT_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable CMP0 output as a wakeup pin.
 * @{
 */
#define PM_WSID_CMP0_OUT_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 27UL, LPCMP0_IRQn, 1UL)
#define PM_WSID_CMP0_OUT_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 27UL, LPCMP0_IRQn, 2UL)
#define PM_WSID_CMP0_OUT_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 27UL, LPCMP0_IRQn, 3UL)
/*! @} */

/*!
 * @name Enable CMP1 output as a wakeup pin.
 * @{
 */
#define PM_WSID_CMP1_OUT_RISING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 28UL, LPCMP0_IRQn, 1UL)
#define PM_WSID_CMP1_OUT_FALLING_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 28UL, LPCMP0_IRQn, 2UL)
#define PM_WSID_CMP1_OUT_ANY_EDGE PM_ENCODE_WAKEUP_SOURCE_ID(0UL, 28UL, LPCMP0_IRQn, 3UL)
/*! @} */

/*!
 * @brief Enable LPTMR0 Interrupt as a wakeup source.
 */
#define PM_WSID_LPTMR0 PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 0UL, LPTMR0_IRQn, 0UL)

/*!
 * @brief Enable LPTMR0 Asynchronous DMA as wakeup source.
 */
#define PM_WSID_LPTMR0_DMA PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 0UL, LPTMR0_IRQn, 1UL)

/*!
 * @brief Enable LPTMR1 as a wakeup source.
 */
#define PM_WSID_LPTMR1 PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 0UL, LPTMR1_IRQn, 0UL)

/*!
 * @brief Enable LPTMR1 Asynchronous DMA as wakeup source.
 * 
 */
#define PM_WSID_LPTMR1_DMA PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 1UL, LPTMR1_IRQn, 1UL)

/*!
 * @brief Enable GPIOD Low pins as wakeup source.
 */
#define PM_WSID_GPIOD_LOW (uint32_t) PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 1UL, (uint32_t) GPIOD_INT0_IRQn, 0UL)

/*!
 * @brief Enable GPIOD High pins as wakeup source.
 */
#define PM_WSID_GPIOD_HIGH PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 2UL, GPIOD_INT1_IRQn, 0UL)

/*!
 * @brief Enable Radio Asynchronous wakeup request as wakeup source.
 */
#define PM_WSID_Radio_Wakeup_Reqest PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 2UL, WUU0_IRQn, 1UL)

/*!
 * @brief Enable Wake-from-radio as wakeup source.
 */
#define PM_WSID_WFR PM_ENCODE_WAKEUP_SOURCE_ID(1U, 3UL, RFMC_IRQn, 0UL)

/*!
 * @brief Enable VBAT module as wakeup source.
 */
#define PM_WSID_WFSPSD PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 4UL, VBAT_IRQn, 0UL)

/*!
 * @brief Enable GPIOD Asynchronous DMA request 0.
 */
#define PM_WSID_GPIOD_LOW_DMA_REQ0 PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 4UL, WUU0_IRQn, 1UL)

/*!
 * @brief 
 * @todo Need to check irq number
 */
#define PM_WSID_TamperDetect_Int PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 5UL, VBAT_IRQn, 0UL)

/*!
 * @brief Enable GPIOD Asynchronous DAM request 1.
 * 
 */
#define PM_WSID_GPIOD_HIGH_DMA_REQ1 PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 5UL, WUU0_IRQn, 1UL)

/*!
 * @brief Enable RTC Alarm as wakeup source.
 */
#define PM_WSID_RTC_ALARM PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 6UL, RTC_Alarm_IRQn, 0UL)

/*!
 * @brief Enable RTC Second as wakeup source.
 */
#define PM_WSID_RTC_SECOND PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 7UL, RTC_Second_IRQn, 0UL)

/*!
 * @brief Enable LPTMR0 Asynchronous trigger.
 */
#define PM_WSID_LPTMR0_TRIG PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 8UL, WUU0_IRQn, 1UL)

/*!
 * @brief Enable LPTMR1 Asynchronous trigger.
 */
#define PM_WSID_LPTMR1_TRIG PM_ENCODE_WAKEUP_SOURCE_ID(1UL, 9UL, WUU0_IRQn, 1UL)

/*! @} */

#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*! @} */
#endif /* _FSL_PM_DEVICE_H_ */
