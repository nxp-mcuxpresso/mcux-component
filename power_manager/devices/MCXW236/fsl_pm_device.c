/*
 * Copyright 2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
   
#include "fsl_power.h"

static void PM_EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void PM_CleanExitLowPowerMode(void);

const pm_device_option_t g_devicePMOption = {
    .states= {
          [PM_LP_STATE_SLEEP] = {
              .exitLatency = 16U, /* 16 us */
              .fixConstraintsMask = PM_DEFINE_RESC_MASK(kResc_CpuClk),
              .varConstraintsMask = {.rescMask = {0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL}},
          },
          [PM_LP_STATE_DEEP_SLEEP] = {
              .exitLatency = 400U,  /* 400 us */
              .fixConstraintsMask = PM_DEFINE_RESC_MASK(kResc_CpuClk, kResc_Flash),
              .varConstraintsMask = PM_DEFINE_RESC_MASK(kResc_NMISrcWake, kResc_Dma0Wake, kResc_Gint0Wake, kResc_PinInt0Wake, kResc_PinInt1Wake, kResc_PinInt2Wake, kResc_PinInt3Wake, kResc_UtickWake, kResc_MrtWake, kResc_Ctimer0Wake, kResc_Ctimer1Wake, kResc_SctWake, kResc_Ctimer3Wake, kResc_Flexcomm0Wake, kResc_Flexcomm1Wake, kResc_Flexcomm2Wake, kResc_BleLinkLayerWake, kResc_BleSleepTimerWake, kResc_WdtWake, kResc_Bod1Wake, kResc_Bod2Wake, kResc_RtcWake, kResc_WakeDslpWake, kResc_PinInt4Wake, kResc_PinInt5Wake, kResc_PinInt6Wake, kResc_PinInt7Wake, kResc_Ctimer2Wake, kResc_Ctimer4Wake, kResc_OsEventWake, kResc_SpifiWake, kResc_SecGpioInt0Irq0Wake, kResc_SecGpioInt0Irq1Wake, kResc_PluWake, kResc_SecVioWake, kResc_TrngWake, kResc_Dma1Wake, kResc_WakePadWake, kResc_Dcdc, kResc_Bod1, kResc_Bod2, kResc_Fro192M, kResc_Fro32k, kResc_Xtal32k, kResc_Fro1M, kResc_Trng, kResc_DcdcBypass, kResc_BleWup, kResc_HFDSM),

          },
          [PM_LP_STATE_PD_WITH_CPU_RET] = {
              .exitLatency = 1345U, /* 1345 us */
              .fixConstraintsMask = PM_DEFINE_RESC_MASK(kResc_CpuClk, kResc_Flash, kResc_NMISrcWake, kResc_Dma0Wake, kResc_PinInt0Wake, kResc_PinInt1Wake, kResc_PinInt2Wake, kResc_PinInt3Wake, kResc_UtickWake,
              kResc_MrtWake, kResc_Ctimer0Wake, kResc_Ctimer1Wake, kResc_SctWake, kResc_Ctimer3Wake, kResc_Flexcomm1Wake,
              kResc_Flexcomm2Wake, kResc_BleLinkLayerWake, kResc_WdtWake, kResc_Bod1Wake, kResc_Bod2Wake,
              kResc_WakeDslpWake, kResc_PinInt4Wake, kResc_PinInt5Wake, kResc_PinInt6Wake, kResc_PinInt7Wake,
              kResc_Ctimer2Wake, kResc_Ctimer4Wake, kResc_SpifiWake, kResc_SecGpioInt0Irq0Wake, kResc_SecGpioInt0Irq1Wake,
              kResc_PluWake, kResc_SecVioWake, kResc_TrngWake, kResc_Dma1Wake, kResc_Fro192M, kResc_Trng),
              .varConstraintsMask = PM_DEFINE_RESC_MASK(kResc_Gint0Wake, kResc_Flexcomm0Wake, kResc_BleSleepTimerWake, kResc_RtcWake, kResc_OsEventWake, kResc_Dcdc, kResc_Bod1, kResc_Bod2, kResc_Fro32k, kResc_Xtal32k, kResc_Fro1M, kResc_BleWup, kResc_DcdcBypass),
          },
          [PM_LP_STATE_DPD] = {
              .exitLatency = 15400, /* 15.4 mS */
              .fixConstraintsMask = PM_DEFINE_RESC_MASK(kResc_CpuClk, kResc_Flash, kResc_NMISrcWake, kResc_Dma0Wake, kResc_PinInt0Wake, kResc_PinInt1Wake, kResc_PinInt2Wake, kResc_PinInt3Wake, kResc_UtickWake,
              kResc_MrtWake, kResc_Ctimer0Wake, kResc_Ctimer1Wake, kResc_SctWake, kResc_Ctimer3Wake, kResc_Flexcomm1Wake,
              kResc_Flexcomm2Wake, kResc_BleLinkLayerWake, kResc_WdtWake, kResc_Bod1Wake, kResc_Bod2Wake,
              kResc_WakeDslpWake, kResc_PinInt4Wake, kResc_PinInt5Wake, kResc_PinInt6Wake, kResc_PinInt7Wake,
              kResc_Ctimer2Wake, kResc_Ctimer4Wake, kResc_SpifiWake, kResc_SecGpioInt0Irq0Wake, kResc_SecGpioInt0Irq1Wake,
              kResc_PluWake, kResc_SecVioWake, kResc_TrngWake, kResc_Dma1Wake, kResc_Fro192M, kResc_Trng),
              .varConstraintsMask = PM_DEFINE_RESC_MASK(kResc_RtcWake, kResc_OsEventWake, kResc_WakePadWake, kResc_Dcdc, kResc_Bod1, kResc_Bod2,  kResc_Fro32k, kResc_Xtal32k),
          },
          [PM_LP_STATE_POWER_OFF] = {
              .exitLatency = 16000U, /* 16 ms */
              .fixConstraintsMask = PM_DEFINE_RESC_MASK(kResc_CpuClk, kResc_Flash, kResc_Dcdc, kResc_Bod1, kResc_Bod2, kResc_Fro192M, kResc_Fro32k, kResc_Xtal32k, kResc_Fro1M, kResc_Trng, kResc_DcdcBypass, kResc_BleWup, kResc_HFDSM, kResc_NMISrcWake, kResc_Dma0Wake, kResc_Gint0Wake, kResc_PinInt0Wake, kResc_PinInt1Wake, kResc_PinInt2Wake, kResc_PinInt3Wake, kResc_UtickWake, kResc_MrtWake, kResc_Ctimer0Wake, kResc_Ctimer1Wake, kResc_SctWake, kResc_Ctimer3Wake, kResc_Flexcomm0Wake, kResc_Flexcomm1Wake, kResc_Flexcomm2Wake, kResc_BleLinkLayerWake, kResc_BleSleepTimerWake, kResc_WdtWake, kResc_Bod1Wake, kResc_Bod2Wake, kResc_RtcWake, kResc_WakeDslpWake, kResc_PinInt4Wake, kResc_PinInt5Wake, kResc_PinInt6Wake, kResc_PinInt7Wake, kResc_Ctimer2Wake, kResc_Ctimer4Wake, kResc_OsEventWake, kResc_SpifiWake, kResc_SecGpioInt0Irq0Wake, kResc_SecGpioInt0Irq1Wake, kResc_PluWake, kResc_SecVioWake, kResc_TrngWake, kResc_Dma1Wake),
              .varConstraintsMask = PM_DEFINE_RESC_MASK(kResc_WakePadWake),
          },
     },
    .stateCount = PM_LP_STATE_COUNT,
    .enter = PM_EnterLowPowerMode,
    .clean = PM_CleanExitLowPowerMode,
};


static void PM_EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    uint32_t excludeFromPd = 0UL;
    uint32_t enabledWakeupSource = 0UL;
    status_t status = kStatus_Fail;
    uint32_t wakeupIoCtrl;

    const uint32_t pmcConfigBits[] = {
        kLOWPOWERCFG_DCDC,
        kLOWPOWERCFG_BOD1,
        kLOWPOWERCFG_BOD2,
        kLOWPOWERCFG_FRO192M,
        kLOWPOWERCFG_FRO32K,
        kLOWPOWERCFG_XTAL32K,
        kLOWPOWERCFG_FRO1M,
        kLOWPOWERCFG_TRNG,
        kLOWPOWERCFG_DCDC_BYPASS,
        kLOWPOWERCFG_BLE_WUP,
        kLOWPOWERCFG_HFDSM
    };

    for (uint32_t i = 0UL; i < PM_CONSTRAINT_COUNT; i++)
    {
        if (PM_IS_CONS_SET(pSoftRescMask, i) && (PM_FIND_CONS_OPERATE_MODE(i, pSysRescGroup) == PM_RESOURCE_FULL_ON))
        {
            if (i < (uint32_t)kResc_CpuClk)
            {
                /* Wakeup related constraints */
                enabledWakeupSource |= (1UL << i);
            }
            else if (i > (uint32_t)kResc_Flash)
            {
                excludeFromPd |= pmcConfigBits[(i-(uint32_t)kResc_Dcdc)];
            }
        }
    }

    if ((enabledWakeupSource & kResc_WakePadWake) != 0UL)
    {
        wakeupIoCtrl = kWAKEUP_PIN_ENABLE | kWAKEUP_PIN_PUP_EN | kWAKEUP_PIN_PDN_DIS | kWAKEUP_PIN_WAKEUP_LOW_LVL;
    }
    else
    {
        wakeupIoCtrl = 0UL;
    }
    switch(stateIndex)
    {
        case PM_LP_STATE_SLEEP:
        {
            /* An interrupt will force the power lib to exit below function */
            uint32_t primask                = DisableGlobalIRQ();
            clock_attach_id_t previousClock = CLOCK_GetClockAttachId(kFRO12M_to_MAIN_CLK);
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */

            CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);
            SYSCON->AHBCLKCTRLCLR[0] = SYSCON_AHBCLKCTRL0_FLASH_MASK;
            __WFI();
            SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_FLASH_MASK;
            CLOCK_AttachClk(previousClock);

#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_StopAndRecordTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */

            EnableGlobalIRQ(primask);
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */

            if ((excludeFromPd & kLOWPOWERCFG_BLE_WUP) != 0UL)
            {
                excludeFromPd |= kLOWPOWERCFG_FRO32K | kLOWPOWERCFG_XTAL32K;
            }

            if ((excludeFromPd & kLOWPOWERCFG_FRO192M) != 0UL)
            {
                excludeFromPd |= kLOWPOWERCFG_HFDSM;
            }
            status = POWER_EnterDeepSleep(excludeFromPd, enabledWakeupSource);

#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_StopAndRecordTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */

            assert(status == kStatus_Success);
            break;
        }
        case PM_LP_STATE_PD_WITH_CPU_RET:
        {
            /* SRAM_X2 is used to store cpu state */
            POWER_EnableSRAM(POWER_GetSRAMConfig() | kSRAM_CTRL_RAMX2);

            if ((excludeFromPd & kPOWERCFG_DCDC) != 0UL)
            {
                /* BOD1 is required in power down mode when DCDC is enabled. */
                excludeFromPd |= kPOWERCFG_BOD1;
            }
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */

            status = POWER_EnterPowerDown(excludeFromPd, enabledWakeupSource, 1);

#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_StopAndRecordTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */

            assert(status == kStatus_Success);
            break;
        }
        case PM_LP_STATE_DPD:
        {
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
            status = POWER_EnterDeepPowerDown(excludeFromPd, enabledWakeupSource, wakeupIoCtrl);
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_StopAndRecordTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
            assert(status == kStatus_Success);
            break;
        }
        case PM_LP_STATE_POWER_OFF:
        {
            excludeFromPd = 0UL;
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
            POWER_EnterPowerOff(excludeFromPd, wakeupIoCtrl);
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
            PM_StopAndRecordTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
            /* If enter into power off mode successfully, this line will never hit. */
            assert(false);
        }
    }
    (void)status;
}

static void PM_CleanExitLowPowerMode(void)
{
    
}

