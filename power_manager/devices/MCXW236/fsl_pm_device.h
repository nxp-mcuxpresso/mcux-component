/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FSL_PM_DEVICE_H
#define _FSL_PM_DEVICE_H

#include "fsl_common.h"
#include "fsl_pm_config.h"


#define PM_LP_STATE_SLEEP           (0U)
#define PM_LP_STATE_DEEP_SLEEP      (1U)
#define PM_LP_STATE_PD_WITH_CPU_RET (2U)
#define PM_LP_STATE_DPD             (3U)
#define PM_LP_STATE_POWER_OFF       (4U)
#define PM_LP_STATE_NO_CONSTRAINT   (0xFFU)


/*!
 * @brief Enumeration of power manager resource names.
 *
 * This enumeration defines all the resources that can be managed by the power manager,
 * including wake-up sources and system resources.
 */
enum _pm_resc_name
{
    /* Wake-up constraints */
    kResc_NMISrcWake = (uint32_t)WDT_BOD_IRQn,          /*!< NMI source wake-up */
    kResc_Dma0Wake = (uint32_t)DMA0_IRQn,               /*!< DMA0 wake-up */
    kResc_Gint0Wake = (uint32_t)GINT0_IRQn,             /*!< Group interrupt 0 wake-up */
    kResc_PinInt0Wake = (uint32_t)PIN_INT0_IRQn,        /*!< Pin interrupt 0 wake-up */
    kResc_PinInt1Wake = (uint32_t)PIN_INT1_IRQn,        /*!< Pin interrupt 1 wake-up */
    kResc_PinInt2Wake = (uint32_t)PIN_INT2_IRQn,        /*!< Pin interrupt 2 wake-up */
    kResc_PinInt3Wake = (uint32_t)PIN_INT3_IRQn,        /*!< Pin interrupt 3 wake-up */
    kResc_UtickWake   = (uint32_t)UTICK0_IRQn,          /*!< Micro-tick timer wake-up */
    kResc_MrtWake = (uint32_t)MRT0_IRQn,                /*!< Multi-rate timer wake-up */
    kResc_Ctimer0Wake = (uint32_t)CTIMER0_IRQn,         /*!< Standard counter/timer 0 wake-up */
    kResc_Ctimer1Wake = (uint32_t)CTIMER1_IRQn,         /*!< Standard counter/timer 1 wake-up */
    kResc_SctWake = (uint32_t)SCT0_IRQn,                /*!< SCT wake-up */
    kResc_Ctimer3Wake = (uint32_t)CTIMER3_IRQn,         /*!< Standard counter/timer 3 wake-up */
    kResc_Flexcomm0Wake = (uint32_t)FLEXCOMM0_IRQn,     /*!< Flexcomm 0 wake-up */
    kResc_Flexcomm1Wake = (uint32_t)FLEXCOMM1_IRQn,     /*!< Flexcomm 1 wake-up */
    kResc_Flexcomm2Wake = (uint32_t)FLEXCOMM2_IRQn,     /*!< Flexcomm 2 wake-up */
    kResc_BleLinkLayerWake = (uint32_t)BLE_LL_IRQn,     /*!< BLE link layer wake-up */
    kResc_BleSleepTimerWake = (uint32_t)BLE_SLP_TMR_IRQn, /*!< BLE sleep timer wake-up */
    kResc_WdtWake = (uint32_t)WDT_IRQn,                 /*!< Watchdog timer wake-up */
    kResc_Bod1Wake = (uint32_t)BOD1_IRQn,               /*!< Brown-out detector 1 wake-up */
    kResc_Bod2Wake = (uint32_t)BOD2_IRQn,               /*!< Brown-out detector 2 wake-up */
    kResc_RtcWake = (uint32_t)RTC_IRQn,                 /*!< RTC wake-up */
    kResc_WakeDslpWake = (uint32_t)WAKE_DSLP_IRQn,      /*!< Deep sleep wake-up */
    kResc_PinInt4Wake = (uint32_t)PIN_INT4_IRQn,        /*!< Pin interrupt 4 wake-up */
    kResc_PinInt5Wake = (uint32_t)PIN_INT5_IRQn,        /*!< Pin interrupt 5 wake-up */
    kResc_PinInt6Wake = (uint32_t)PIN_INT6_IRQn,        /*!< Pin interrupt 6 wake-up */
    kResc_PinInt7Wake = (uint32_t)PIN_INT7_IRQn,        /*!< Pin interrupt 7 wake-up */
    kResc_Ctimer2Wake = (uint32_t)CTIMER2_IRQn,         /*!< Standard counter/timer 2 wake-up */
    kResc_Ctimer4Wake = (uint32_t)CTIMER4_IRQn,         /*!< Standard counter/timer 4 wake-up */
    kResc_OsEventWake = (uint32_t)OS_EVENT_IRQn,        /*!< OS event timer wake-up */
    kResc_SpifiWake = (uint32_t)SPIFI_IRQn,             /*!< SPIFI wake-up */
    kResc_SecGpioInt0Irq0Wake = (uint32_t)SEC_GPIO_INT0_IRQ0_IRQn, /*!< Secure GPIO interrupt 0 IRQ0 wake-up */
    kResc_SecGpioInt0Irq1Wake = (uint32_t)SEC_GPIO_INT0_IRQ1_IRQn, /*!< Secure GPIO interrupt 0 IRQ1 wake-up */
    kResc_PluWake = (uint32_t)PLU_IRQn,                 /*!< PLU wake-up */
    kResc_SecVioWake = (uint32_t)SEC_VIO_IRQn,          /*!< Security violation wake-up */
    kResc_TrngWake = (uint32_t)TRNG_IRQn,               /*!< True random number generator wake-up */
    kResc_Dma1Wake = (uint32_t)DMA1_IRQn,               /*!< DMA1 wake-up */
    kResc_WakePadWake = (uint32_t)WAKE_PAD_IRQn,        /*!< Wake pad wake-up */

    /* System resources */
    kResc_CpuClk = kResc_WakePadWake + 1,                /*!< CPU clock resource */
    kResc_Flash,                                         /*!< Flash resource */
    kResc_Dcdc,                                          /*!< DC-DC converter resource */
    kResc_Bod1,                                          /*!< Brown-out detector 1 resource */
    kResc_Bod2,                                          /*!< Brown-out detector 2 resource */
    kResc_Fro192M,                                       /*!< 192MHz FRO resource */
    kResc_Fro32k,                                        /*!< 32kHz FRO resource */
    kResc_Xtal32k,                                       /*!< 32kHz crystal oscillator resource */
    kResc_Fro1M,                                         /*!< 1MHz FRO resource */
    kResc_Trng,                                          /*!< True random number generator resource */
    kResc_DcdcBypass,                                    /*!< DC-DC bypass mode resource */
    kResc_BleWup,                                        /*!< BLE wake-up resource */
    kResc_HFDSM,                                         /*!< High frequency delta-sigma modulator resource */
};


/* Constraints used by application. */

/*!
 * @name Define constraint of core clock.
 * @{
 */
/*! @brief NMI source wake-up constraints */
#define PM_RESC_NMISRC_WAKE_ON PM_RESC_FULL_ON(kResc_NMISrcWake)   /*!< Enable NMI source wake-up */
#define PM_RESC_NMISRC_WAKE_OFF PM_RESC_OFF(kResc_NMISrcWake)      /*!< Disable NMI source wake-up */

/*! @brief DMA0 wake-up constraints */
#define PM_RESC_DMA0_WAKE_ON    PM_RESC_FULL_ON(kResc_Dma0Wake)    /*!< Enable DMA0 wake-up */
#define PM_RESC_DMA0_WAKE_OFF   PM_RESC_OFF(kResc_Dma0Wake)        /*!< Disable DMA0 wake-up */

/*! @brief Group interrupt 0 wake-up constraints */
#define PM_RESC_GINT0_WAKE_ON   PM_RESC_FULL_ON(kResc_Gint0Wake)   /*!< Enable GINT0 wake-up */
#define PM_RESC_GINT0_WAKE_OFF  PM_RESC_OFF(kResc_Gint0Wake)       /*!< Disable GINT0 wake-up */

/*! @brief Pin interrupt 0 wake-up constraints */
#define PM_RESC_PININT0_WAKE_ON  PM_RESC_FULL_ON(kResc_PinInt0Wake) /*!< Enable pin interrupt 0 wake-up */
#define PM_RESC_PININT0_WAKE_OFF PM_RESC_OFF(kResc_PinInt0Wake)    /*!< Disable pin interrupt 0 wake-up */

/*! @brief Pin interrupt 1 wake-up constraints */
#define PM_RESC_PININT1_WAKE_ON  PM_RESC_FULL_ON(kResc_PinInt1Wake) /*!< Enable pin interrupt 1 wake-up */
#define PM_RESC_PININT1_WAKE_OFF PM_RESC_OFF(kResc_PinInt1Wake)    /*!< Disable pin interrupt 1 wake-up */

/*! @brief Pin interrupt 2 wake-up constraints */
#define PM_RESC_PININT2_WAKE_ON  PM_RESC_FULL_ON(kResc_PinInt2Wake) /*!< Enable pin interrupt 2 wake-up */
#define PM_RESC_PININT2_WAKE_OFF PM_RESC_OFF(kResc_PinInt2Wake)    /*!< Disable pin interrupt 2 wake-up */

/*! @brief Pin interrupt 3 wake-up constraints */
#define PM_RESC_PININT3_WAKE_ON  PM_RESC_FULL_ON(kResc_PinInt3Wake) /*!< Enable pin interrupt 3 wake-up */
#define PM_RESC_PININT3_WAKE_OFF PM_RESC_OFF(kResc_PinInt3Wake)    /*!< Disable pin interrupt 3 wake-up */

/*! @brief Micro-tick timer wake-up constraints */
#define PM_RESC_UTICK_WAKE_ON    PM_RESC_FULL_ON(kResc_UtickWake)  /*!< Enable UTICK wake-up */
#define PM_RESC_UTICK_WAKE_OFF   PM_RESC_OFF(kResc_UtickWake)      /*!< Disable UTICK wake-up */

/*! @brief Multi-rate timer wake-up constraints */
#define PM_RESC_MRT_WAKE_ON      PM_RESC_FULL_ON(kResc_MrtWake)    /*!< Enable MRT wake-up */
#define PM_RESC_MRT_WAKE_OFF     PM_RESC_OFF(kResc_MrtWake)        /*!< Disable MRT wake-up */

/*! @brief Counter/timer 0 wake-up constraints */
#define PM_RESC_CTIMER0_WAKE_ON  PM_RESC_FULL_ON(kResc_Ctimer0Wake) /*!< Enable CTIMER0 wake-up */
#define PM_RESC_CTIMER0_WAKE_OFF PM_RESC_OFF(kResc_Ctimer0Wake)    /*!< Disable CTIMER0 wake-up */

/*! @brief Counter/timer 1 wake-up constraints */
#define PM_RESC_CTIMER1_WAKE_ON  PM_RESC_FULL_ON(kResc_Ctimer1Wake) /*!< Enable CTIMER1 wake-up */
#define PM_RESC_CTIMER1_WAKE_OFF PM_RESC_OFF(kResc_Ctimer1Wake)    /*!< Disable CTIMER1 wake-up */

/*! @brief SCT wake-up constraints */
#define PM_RESC_SCT_WAKE_ON      PM_RESC_FULL_ON(kResc_SctWake)    /*!< Enable SCT wake-up */
#define PM_RESC_SCT_WAKE_OFF     PM_RESC_OFF(kResc_SctWake)        /*!< Disable SCT wake-up */

/*! @brief Counter/timer 3 wake-up constraints */
#define PM_RESC_CTIMER3_WAKE_ON  PM_RESC_FULL_ON(kResc_Ctimer3Wake) /*!< Enable CTIMER3 wake-up */
#define PM_RESC_CTIMER3_WAKE_OFF PM_RESC_OFF(kResc_Ctimer3Wake)    /*!< Disable CTIMER3 wake-up */

/*! @brief Flexcomm 0 wake-up constraints */
#define PM_RESC_FLEXCOMM0_WAKE_ON  PM_RESC_FULL_ON(kResc_Flexcomm0Wake) /*!< Enable FLEXCOMM0 wake-up */
#define PM_RESC_FLEXCOMM0_WAKE_OFF PM_RESC_OFF(kResc_Flexcomm0Wake)    /*!< Disable FLEXCOMM0 wake-up */

/*! @brief Flexcomm 1 wake-up constraints */
#define PM_RESC_FLEXCOMM1_WAKE_ON  PM_RESC_FULL_ON(kResc_Flexcomm1Wake) /*!< Enable FLEXCOMM1 wake-up */
#define PM_RESC_FLEXCOMM1_WAKE_OFF PM_RESC_OFF(kResc_Flexcomm1Wake)    /*!< Disable FLEXCOMM1 wake-up */

/*! @brief Flexcomm 2 wake-up constraints */
#define PM_RESC_FLEXCOMM2_WAKE_ON  PM_RESC_FULL_ON(kResc_Flexcomm2Wake) /*!< Enable FLEXCOMM2 wake-up */
#define PM_RESC_FLEXCOMM2_WAKE_OFF PM_RESC_OFF(kResc_Flexcomm2Wake)    /*!< Disable FLEXCOMM2 wake-up */

/*! @brief BLE link layer wake-up constraints */
#define PM_RESC_BLE_LINK_LAYER_WAKE_ON  PM_RESC_FULL_ON(kResc_BleLinkLayerWake) /*!< Enable BLE link layer wake-up */
#define PM_RESC_BLE_LINK_LAYER_WAKE_OFF PM_RESC_OFF(kResc_BleLinkLayerWake)    /*!< Disable BLE link layer wake-up */

/*! @brief BLE sleep timer wake-up constraints */
#define PM_RESC_BLE_SLEEP_TIMER_WAKE_ON  PM_RESC_FULL_ON(kResc_BleSleepTimerWake) /*!< Enable BLE sleep timer wake-up */
#define PM_RESC_BLE_SLEEP_TIMER_WAKE_OFF PM_RESC_OFF(kResc_BleSleepTimerWake)    /*!< Disable BLE sleep timer wake-up */

/*! @brief Watchdog timer wake-up constraints */
#define PM_RESC_WDT_WAKE_ON             PM_RESC_FULL_ON(kResc_WdtWake)  /*!< Enable WDT wake-up */
#define PM_RESC_WDT_WAKE_OFF            PM_RESC_OFF(kResc_WdtWake)      /*!< Disable WDT wake-up */

/*! @brief Brown-out detector 1 wake-up constraints */
#define PM_RESC_BOD1_WAKE_ON            PM_RESC_FULL_ON(kResc_Bod1Wake) /*!< Enable BOD1 wake-up */
#define PM_RESC_BOD1_WAKE_OFF           PM_RESC_OFF(kResc_Bod1Wake)     /*!< Disable BOD1 wake-up */

/*! @brief Brown-out detector 2 wake-up constraints */
#define PM_RESC_BOD2_WAKE_ON            PM_RESC_FULL_ON(kResc_Bod2Wake) /*!< Enable BOD2 wake-up */
#define PM_RESC_BOD2_WAKE_OFF           PM_RESC_OFF(kResc_Bod2Wake)     /*!< Disable BOD2 wake-up */

/*! @brief RTC wake-up constraints */
#define PM_RESC_RTC_WAKE_ON             PM_RESC_FULL_ON(kResc_RtcWake)  /*!< Enable RTC wake-up */
#define PM_RESC_RTC_WAKE_OFF            PM_RESC_OFF(kResc_RtcWake)      /*!< Disable RTC wake-up */

/*! @brief Deep sleep wake-up constraints */
#define PM_RESC_WAKE_DSLP_WAKE_ON       PM_RESC_FULL_ON(kResc_WakeDslpWake) /*!< Enable deep sleep wake-up */
#define PM_RESC_WAKE_DSLP_WAKE_OFF      PM_RESC_OFF(kResc_WakeDslpWake)     /*!< Disable deep sleep wake-up */

/*! @brief Pin interrupt 4 wake-up constraints */
#define PM_RESC_PIN_INT4_WAKE_ON        PM_RESC_FULL_ON(kResc_PinInt4Wake) /*!< Enable pin interrupt 4 wake-up */
#define PM_RESC_PIN_INT4_WAKE_OFF       PM_RESC_OFF(kResc_PinInt4Wake)     /*!< Disable pin interrupt 4 wake-up */

/*! @brief Pin interrupt 5 wake-up constraints */
#define PM_RESC_PIN_INT5_WAKE_ON        PM_RESC_FULL_ON(kResc_PinInt5Wake) /*!< Enable pin interrupt 5 wake-up */
#define PM_RESC_PIN_INT5_WAKE_OFF       PM_RESC_OFF(kResc_PinInt5Wake)     /*!< Disable pin interrupt 5 wake-up */

/*! @brief Pin interrupt 6 wake-up constraints */
#define PM_RESC_PIN_INT6_WAKE_ON        PM_RESC_FULL_ON(kResc_PinInt6Wake) /*!< Enable pin interrupt 6 wake-up */
#define PM_RESC_PIN_INT6_WAKE_OFF       PM_RESC_OFF(kResc_PinInt6Wake)     /*!< Disable pin interrupt 6 wake-up */

/*! @brief Pin interrupt 7 wake-up constraints */
#define PM_RESC_PIN_INT7_WAKE_ON        PM_RESC_FULL_ON(kResc_PinInt7Wake) /*!< Enable pin interrupt 7 wake-up */
#define PM_RESC_PIN_INT7_WAKE_OFF       PM_RESC_OFF(kResc_PinInt7Wake)     /*!< Disable pin interrupt 7 wake-up */

/*! @brief Counter/timer 2 wake-up constraints */
#define PM_RESC_CTIMER2_WAKE_ON         PM_RESC_FULL_ON(kResc_Ctimer2Wake) /*!< Enable CTIMER2 wake-up */
#define PM_RESC_CTIMER2_WAKE_OFF        PM_RESC_OFF(kResc_Ctimer2Wake)     /*!< Disable CTIMER2 wake-up */

/*! @brief Counter/timer 4 wake-up constraints */
#define PM_RESC_CTIMER4_WAKE_ON         PM_RESC_FULL_ON(kResc_Ctimer4Wake) /*!< Enable CTIMER4 wake-up */
#define PM_RESC_CTIMER4_WAKE_OFF        PM_RESC_OFF(kResc_Ctimer4Wake)     /*!< Disable CTIMER4 wake-up */

/*! @brief OS event wake-up constraints */
#define PM_RESC_OSEVENT_WAKE_ON         PM_RESC_FULL_ON(kResc_OsEventWake) /*!< Enable OS event wake-up */
#define PM_RESC_OSEVENT_WAKE_OFF        PM_RESC_OFF(kResc_OsEventWake)     /*!< Disable OS event wake-up */

/*! @brief SPIFI wake-up constraints */
#define PM_RESC_SPIFI_WAKE_ON           PM_RESC_FULL_ON(kResc_SpifiWake)   /*!< Enable SPIFI wake-up */
#define PM_RESC_SPIFI_WAKE_OFF          PM_RESC_OFF(kResc_SpifiWake)       /*!< Disable SPIFI wake-up */

/*! @brief Secure GPIO interrupt 0 wake-up constraints */
#define PM_RESC_SEC_GPIO_INT0_WAKE_ON   PM_RESC_FULL_ON(kResc_SecGpioInt0Irq0Wake) /*!< Enable secure GPIO INT0 wake-up */
#define PM_RESC_SEC_GPIO_INT0_WAKE_OFF  PM_RESC_OFF(kResc_SecGpioInt0Irq0Wake)     /*!< Disable secure GPIO INT0 wake-up */

/*! @brief PLU wake-up constraints */
#define PM_RESC_PLU_WAKE_ON             PM_RESC_FULL_ON(kResc_PluWake)     /*!< Enable PLU wake-up */
#define PM_RESC_PLU_WAKE_OFF            PM_RESC_OFF(kResc_PluWake)         /*!< Disable PLU wake-up */

/*! @brief Security violation wake-up constraints */
#define PM_RESC_SEC_VIO_WAKE_ON         PM_RESC_FULL_ON(kResc_SecVioWake)  /*!< Enable security violation wake-up */
#define PM_RESC_SEC_VIO_WAKE_OFF        PM_RESC_OFF(kResc_SecVioWake)      /*!< Disable security violation wake-up */

/*! @brief TRNG wake-up constraints */
#define PM_RESC_TRNG_WAKE_ON            PM_RESC_FULL_ON(kResc_TrngWake)    /*!< Enable TRNG wake-up */
#define PM_RESC_TRNG_WAKE_OFF           PM_RESC_OFF(kResc_TrngWake)        /*!< Disable TRNG wake-up */

/*! @brief DMA1 wake-up constraints */
#define PM_RESC_DMA1_WAKE_ON            PM_RESC_FULL_ON(kResc_Dma1Wake)    /*!< Enable DMA1 wake-up */
#define PM_RESC_DMA1_WAKE_OFF           PM_RESC_OFF(kResc_Dma1Wake)        /*!< Disable DMA1 wake-up */

/*! @brief Wake pad wake-up constraints */
#define PM_RESC_WAKE_PAD_WAKE_ON        PM_RESC_FULL_ON(kResc_WakePadWake) /*!< Enable wake pad wake-up */
#define PM_RESC_WAKE_PAD_WAKE_OFF       PM_RESC_OFF(kResc_WakePadWake)     /*!< Disable wake pad wake-up */
/*@}*/

/*! @name System resource constraints */
/*@{*/
/*! @brief CPU clock constraints */
#define PM_RESC_CPU_CLK_ON              PM_RESC_FULL_ON(kResc_CpuClk)      /*!< Enable CPU clock */
#define PM_RESC_CPU_CLK_OFF             PM_RESC_OFF(kResc_CpuClk)          /*!< Disable CPU clock */

/*! @brief DC-DC converter constraints */
#define PM_RESC_DCDC_ON                 PM_RESC_FULL_ON(kResc_Dcdc)        /*!< Enable DC-DC converter */
#define PM_RESC_DCDC_OFF                PM_RESC_OFF(kResc_Dcdc)            /*!< Disable DC-DC converter */

/*! @brief Brown-out detector 1 constraints */
#define PM_RESC_BOD1_ON                 PM_RESC_FULL_ON(kResc_Bod1)        /*!< Enable BOD1 */
#define PM_RESC_BOD1_OFF                PM_RESC_OFF(kResc_Bod1)            /*!< Disable BOD1 */

/*! @brief Brown-out detector 2 constraints */
#define PM_RESC_BOD2_ON                 PM_RESC_FULL_ON(kResc_Bod2)        /*!< Enable BOD2 */
#define PM_RESC_BOD2_OFF                PM_RESC_OFF(kResc_Bod2)            /*!< Disable BOD2 */

/*! @brief 192MHz FRO constraints */
#define PM_RESC_FRO_192M_ON             PM_RESC_FULL_ON(kResc_Fro192M)     /*!< Enable 192MHz FRO */
#define PM_RESC_FRO_192M_OFF            PM_RESC_OFF(kResc_Fro192M)         /*!< Disable 192MHz FRO */

/*! @brief 32kHz FRO constraints */
#define PM_RESC_FRO_32K_ON              PM_RESC_FULL_ON(kResc_Fro32k)      /*!< Enable 32kHz FRO */
#define PM_RESC_FRO_32K_OFF             PM_RESC_OFF(kResc_Fro32k)          /*!< Disable 32kHz FRO */

/*! @brief 32kHz crystal oscillator constraints */
#define PM_RESC_XTAL_32K_ON             PM_RESC_FULL_ON(kResc_Xtal32k)     /*!< Enable 32kHz crystal oscillator */
#define PM_RESC_XTAL_32K_OFF            PM_RESC_OFF(kResc_Xtal32k)         /*!< Disable 32kHz crystal oscillator */

/*! @brief 1MHz FRO constraints */
#define PM_RESC_FRO_1M_ON               PM_RESC_FULL_ON(kResc_Fro1M)       /*!< Enable 1MHz FRO */
#define PM_RESC_FRO_1M_OFF              PM_RESC_OFF(kResc_Fro1M)           /*!< Disable 1MHz FRO */

/*! @brief TRNG constraints */
#define PM_RESC_TRNG_ON                 PM_RESC_FULL_ON(kResc_Trng)        /*!< Enable TRNG */
#define PM_RESC_TRNG_OFF                PM_RESC_OFF(kResc_Trng)            /*!< Disable TRNG */

/*! @brief DC-DC bypass mode constraints */
#define PM_RESC_DCDC_BYPASS_ON          PM_RESC_FULL_ON(kResc_DcdcBypass)  /*!< Enable DC-DC bypass mode */
#define PM_RESC_DCDC_BYPASS_OFF         PM_RESC_OFF(kResc_DcdcBypass)      /*!< Disable DC-DC bypass mode */

/*! @brief BLE wake-up constraints */
#define PM_RESC_BLE_WUP_ON              PM_RESC_FULL_ON(kResc_BleWup)      /*!< Enable BLE wake-up */
#define PM_RESC_BLE_WUP_OFF             PM_RESC_OFF(kResc_BleWup)          /*!< Disable BLE wake-up */

/*! @brief High frequency delta-sigma modulator constraints */
#define PM_RESC_HFDSM_ON                PM_RESC_FULL_ON(kResc_HFDSM)       /*!< Enable HFDSM */
#define PM_RESC_HFDSM_OFF               PM_RESC_OFF(kResc_HFDSM)           /*!< Disable HFDSM */

#define PM_RESC_FLASH_ON                PM_RESC_FULL_ON(kResc_Flash)       /*!< Keep Flash enabled. */
#define PM_RESC_FLASH_OFF               PM_RESC_OFF(kResc_Flash)           /*!< Disable Flash. */
/*@}*/




#endif /* _FSL_PM_DEVICE_H */
