/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pm_core.h"
#include "fsl_pm_device.h"

#include "fsl_cmc.h"
#include "fsl_spc.h"
#include "fsl_wuu.h"
#include "fsl_clock.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef struct _pm_resource_recode pm_resource_recode_t;

static void PM_EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void PM_CleanExitLowPowerMode(void);
static uint8_t PM_FindOperateMode(uint32_t rescIndex, pm_resc_group_t *pSysRescGroup);
static void PM_SetRAMOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode);
static void PM_SetFro192MOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode);
static void PM_SetFro6MOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode);
static void PM_SetWakePowerDomainOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode);
static void PM_ApplyRadioLowPowerMode(uint8_t stateIndex);

static void PM_EnableBasicResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
static status_t PM_ManageWakeupSource(pm_wakeup_source_t *ws, bool enable);
static bool PM_IsWakeupSource(pm_wakeup_source_t *ws);
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*!
 *  |  Module  |    Sleep     | Deep Sleep | Power Down | Deep Power Down |
 *  |  CM33    |    Static    |  Static    |    OFF     |       OFF       |
 *  | FRO-192M |   Optional   |  Optional  |   Optional |       OFF       |
 *  | FRO-6M   |   Optional   |  Optional  |   Optional |       OFF       |
 *  | FRO-32K  |      ON      |     ON     |   Optional |     Optional    |
 *  | OSC-RTC  |      ON      |     ON     |   Optional |     Optional    |
 *  | OSC-RF   |      ON      |  Optional  |   Optional |       OFF       |
 *  |   SCG    |      ON      |   Static   |   Static   |       OFF       |
 *  |   PCC    |      ON      |   Static   |    OFF     |       OFF       |
 *  |  Flash   |  Optional LP |   OFF/LP   |    OFF     |       OFF       |
 *  |  SRAMx   |   Optional   |  Optional  |   Optional |       OFF       |
 *
 *  SRAM partition mapping used by the framework resource table:
 *    CTCM0 -> SRAM0 lower 64 KB
 *    CTCM1 -> SRAM0 upper 64 KB
 *    STCM0 -> SRAM1 64 KB
 *    STCM1 -> SRAM2 32 KB
 *    STCM2 -> SRAM3 lower 16 KB
 *    STCM3 -> SRAM3 upper 16 KB
 */
#define PM_SLEEP_FIX_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CoreClk))
#define PM_SLEEP_VAR_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CTCM0) | PM_RESC_TO_BIT(kResc_CTCM1) | \
                        PM_RESC_TO_BIT(kResc_STCM0) | PM_RESC_TO_BIT(kResc_STCM1) | PM_RESC_TO_BIT(kResc_STCM2) |\
                        PM_RESC_TO_BIT(kResc_STCM3) | PM_RESC_TO_BIT(kResc_Fro192M) | PM_RESC_TO_BIT(kResc_Fro6M) | \
                        PM_RESC_TO_BIT(kResc_WakePowerDomainPeri) | PM_RESC_TO_BIT(kResc_BusSysClk))

#define PM_DS_FIX_CONSTR_MASK   (PM_RESC_TO_BIT(kResc_CoreClk) | PM_RESC_TO_BIT(kResc_BusSysClk) |\
                                PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt2))
#define PM_DS_VAR_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CTCM0) | PM_RESC_TO_BIT(kResc_CTCM1) | \
                        PM_RESC_TO_BIT(kResc_STCM0) | PM_RESC_TO_BIT(kResc_STCM1) | PM_RESC_TO_BIT(kResc_STCM2) |\
                        PM_RESC_TO_BIT(kResc_STCM3) | PM_RESC_TO_BIT(kResc_Fro6M) | \
                        PM_RESC_TO_BIT(kResc_WakePowerDomainPeri))

#define PM_PD_FIX_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CoreClk) | PM_RESC_TO_BIT(kResc_BusSysClk) | \
                            PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt1) | \
                            PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt2))
#define PM_PD_VAR_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CTCM0) | PM_RESC_TO_BIT(kResc_CTCM1) | \
                        PM_RESC_TO_BIT(kResc_STCM0) | PM_RESC_TO_BIT(kResc_STCM1) | PM_RESC_TO_BIT(kResc_STCM2) |\
                        PM_RESC_TO_BIT(kResc_STCM3) | PM_RESC_TO_BIT(kResc_Fro6M) | \
                        PM_RESC_TO_BIT(kResc_WakePowerDomainPeri))

#define PM_DPD_FIX_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CoreClk) | PM_RESC_TO_BIT(kResc_BusSysClk) | \
                            PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt1) | \
                            PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt2) | PM_RESC_TO_BIT(kResc_Fro192M) | \
                            PM_RESC_TO_BIT(kResc_Fro6M) | \
                            PM_RESC_TO_BIT(kResc_CTCM0) | PM_RESC_TO_BIT(kResc_CTCM1) | \
                            PM_RESC_TO_BIT(kResc_STCM0) | PM_RESC_TO_BIT(kResc_STCM1) | \
                            PM_RESC_TO_BIT(kResc_STCM2) | PM_RESC_TO_BIT(kResc_STCM3) | \
                            PM_RESC_TO_BIT(kResc_WakePowerDomainPeri))
#define PM_DPD_VAR_CONSTR_MASK (0UL)

const pm_device_option_t g_devicePMOption = {
    .states =
        {
            /* Sleep */
            {
                .exitLatency = 140U, /* 140 us */
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = PM_SLEEP_FIX_CONSTR_MASK,
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_SLEEP_VAR_CONSTR_MASK,
                    },
            },
            /* Deep Sleep */
            {
                .exitLatency = 180U, /* 180 us */
                .fixConstraintsMask =
                    {
                        .rescMask[0] = PM_DS_FIX_CONSTR_MASK,
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0] = PM_DS_VAR_CONSTR_MASK,
                    },
            },
            /* Power Down */
            {
                .exitLatency = 600U, /* 600 us */
                .fixConstraintsMask =
                    {
                        .rescMask[0] = PM_PD_FIX_CONSTR_MASK,
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0] = PM_PD_VAR_CONSTR_MASK,
                    },
            },
            /* Deep Power Down */
            {
                .fixConstraintsMask =
                    {
                        .rescMask[0] = PM_DPD_FIX_CONSTR_MASK,
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0] = PM_DPD_VAR_CONSTR_MASK,
                    },
            },
        },
    .stateCount = 4U,
    .enter      = PM_EnterLowPowerMode,
    .clean      = PM_CleanExitLowPowerMode,

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
    .manageWakeupSource = PM_ManageWakeupSource,
    .isWakeupSource     = PM_IsWakeupSource,
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
};

struct _pm_resource_recode
{
    uint8_t defaultOperateMode : 4U;
    uint8_t currentOperateMode : 4U;
    void (*resourceConfigFunc)(uint8_t operateMode, pm_resource_recode_t *pResourceRecode);
};

AT_ALWAYS_ON_DATA_INIT(pm_resource_recode_t resourceDB[PM_CONFIGURABLE_RESOURCE_COUNT]) = {
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* CTCM0 */
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* CTCM1 */
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* STCM0 */
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* STCM1 */
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* STCM2 */
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* STCM3 */
    {PM_RESOURCE_OFF, 0U, PM_SetFro192MOperateMode},
    {PM_RESOURCE_OFF, 0U, PM_SetFro6MOperateMode},
    {PM_RESOURCE_FULL_ON, PM_RESOURCE_FULL_ON, PM_SetWakePowerDomainOperateMode},
};

AT_ALWAYS_ON_DATA_INIT(cmc_power_domain_config_t g_mainWakePDConfig) = {
    .clock_mode = kCMC_GateAllSystemClocksEnterLowPowerMode,
    /* we chose to put the main and wake domain in retention (deep sleep) by default if no constraints are set */
    .main_domain = kCMC_DeepSleepMode,
    .wake_domain = kCMC_DeepSleepMode,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
static void PM_EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    uint8_t rescMaskArrayIndex = 0U;
    uint8_t rescMaskOffset     = 0U;

    switch (stateIndex)
    {
        case PM_LP_STATE_SLEEP:
        {
            rescMaskArrayIndex = ((uint8_t)kResc_BusSysClk) / 32U;
            rescMaskOffset     = ((uint8_t)kResc_BusSysClk) % 32U;
            if ((pSoftRescMask->rescMask[rescMaskArrayIndex] & (1UL << rescMaskOffset)) != 0UL)
            {
                g_mainWakePDConfig.clock_mode    = kCMC_GateCoreClock;
                g_mainWakePDConfig.main_domain   = kCMC_ActiveMode;
                g_mainWakePDConfig.wake_domain   = kCMC_ActiveMode;
                resourceDB[(uint8_t)kResc_WakePowerDomainPeri].currentOperateMode = PM_RESOURCE_FULL_ON;
                pSoftRescMask->rescMask[rescMaskArrayIndex] &= ~(1UL << rescMaskOffset);
            }
            else
            {
                g_mainWakePDConfig.clock_mode    = kCMC_GateAllSystemClocksEnterLowPowerMode;
                g_mainWakePDConfig.main_domain   = kCMC_SleepMode;
                g_mainWakePDConfig.wake_domain   = kCMC_SleepMode;
                resourceDB[(uint8_t)kResc_WakePowerDomainPeri].currentOperateMode = PM_RESOURCE_PARTABLE_ON2;
            }
            break;
        }

        case PM_LP_STATE_DEEP_SLEEP:
        {
            g_mainWakePDConfig.clock_mode    = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_mainWakePDConfig.main_domain   = kCMC_DeepSleepMode;
            g_mainWakePDConfig.wake_domain   = kCMC_DeepSleepMode;
            resourceDB[(uint8_t)kResc_WakePowerDomainPeri].currentOperateMode = PM_RESOURCE_PARTABLE_ON1;
            break;
        }

        case PM_LP_STATE_POWER_DOWN:
        {
            g_mainWakePDConfig.clock_mode    = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_mainWakePDConfig.main_domain   = kCMC_PowerDownMode;
            g_mainWakePDConfig.wake_domain   = kCMC_PowerDownMode;
            resourceDB[(uint8_t)kResc_WakePowerDomainPeri].currentOperateMode = PM_RESOURCE_OFF;
            break;
        }

        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            g_mainWakePDConfig.clock_mode    = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_mainWakePDConfig.main_domain   = kCMC_DeepPowerDown;
            g_mainWakePDConfig.wake_domain   = kCMC_DeepPowerDown;
            resourceDB[(uint8_t)kResc_WakePowerDomainPeri].currentOperateMode = PM_RESOURCE_OFF;
            break;
        }

        default:
        {
            /* This branch should never be hit. */
            return;
        }
    }
    PM_EnableBasicResources(pSoftRescMask, pSysRescGroup);
    CMC_SetPowerModeProtection(CMC0, (uint32_t)kCMC_AllowAllLowPowerModes);

    PM_ApplyRadioLowPowerMode(stateIndex);

    if ((stateIndex == PM_LP_STATE_SLEEP) || (stateIndex == PM_LP_STATE_DEEP_SLEEP))
    {
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
        PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
        if (g_mainWakePDConfig.clock_mode == kCMC_GateCoreClock)
        {
            /* This configuration corresponds to WFI only, only the core clock will be gated */
            CMC_SetClockMode(CMC0, g_mainWakePDConfig.clock_mode);
            CMC_SetMAINPowerMode(CMC0, g_mainWakePDConfig.main_domain);
            CMC_SetWAKEPowerMode(CMC0, g_mainWakePDConfig.wake_domain);
            SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
            __DSB();
            __WFI();
            __ISB();
        }
        else
        {
            CMC_EnterLowPowerMode(CMC0, &g_mainWakePDConfig);
        }
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
        PM_StopAndRecordTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
    }
    else
    {
        /*
         * KW43 A0 ROM does not support fast boot for PD/DPD, so context
         * save/restore (setjmp/longjmp) is not used here.  The device returns
         * through a normal boot after wakeup.  The low-power timer is still
         * started so that the LPTMR can generate a WUU wakeup event;
         * PM_StopAndRecordTimer() will not run after the reset, which is
         * expected on this path.
         */
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
        PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
        CMC_EnterLowPowerMode(CMC0, &g_mainWakePDConfig);
    }
    /* Note: Reconfigure CMC and clear SCR SLEEPDEEP bit to
     * avoid soc low power when a WFI instruction is called. */
    CMC_SetClockMode(CMC0, kCMC_GateNoneClock);
    CMC_SetMAINPowerMode(CMC0, kCMC_ActiveMode);
    CMC_SetWAKEPowerMode(CMC0, kCMC_ActiveMode);
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    __DSB();
}

static void PM_CleanExitLowPowerMode(void)
{
    if ((CMC_GetSystemResetStatus(CMC0) & kCMC_WakeUpReset) != 0UL)
    {
        SPC_ClearPeriphIOIsolationFlag(SPC0);
    }

    /* Clear SPC LP request status for next low power phase
     * The clean up API has been added because the modules registered to notification
     * could need to read some status registers so we don't want a specific module to
     * handle the clear of those status as it could lead to some modules missing the
     * status information (depending on the notification order).
     * So this API is meant to clear everything needed for the platform once every
     * module got notified. */
    if (SPC_CheckPowerDomainLowPowerRequest(SPC0, kSPC_PowerDomain0))
    {
        SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain0);
    }
    if (SPC_CheckPowerDomainLowPowerRequest(SPC0, kSPC_PowerDomain1))
    {
        SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain1);
    }
    if (SPC_CheckPowerDomainLowPowerRequest(SPC0, kSPC_PowerDomain2))
    {
        /* Release the radio 2.4 GHz LP request so the domain can re-enter at the next cycle. */
        RFMC->RF2P4GHZ_CTRL &= ~RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;
        RFMC->RF2P4GHZ_CTRL &= ~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK;
        SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain2);
    }
    if (SPC_CheckPowerDomainLowPowerRequest(SPC0, kSPC_PowerDomain3))
    {
        SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain3);
    }
    SPC_ClearLowPowerRequest(SPC0);
}

static uint8_t PM_FindOperateMode(uint32_t rescIndex, pm_resc_group_t *pSysRescGroup)
{
    assert(pSysRescGroup);
    uint32_t u32Tmp = (pSysRescGroup->groupSlice[rescIndex / 8UL] >> (4UL * (rescIndex % 8UL))) & 0xFUL;

    u32Tmp |= (u32Tmp >> 1UL);
    u32Tmp |= (u32Tmp >> 2UL);

    return (uint8_t)(((u32Tmp + 1UL) >> 1UL));
}

static void PM_EnableBasicResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    assert(pSoftRescMask);
    assert(pSysRescGroup);

    uint32_t i     = 0UL;
    uint8_t opMode = PM_RESOURCE_OFF;

    for (i = 0UL; i < PM_CONFIGURABLE_RESOURCE_COUNT; i++)
    {
        if ((pSoftRescMask->rescMask[i / 32UL] & (1UL << (i % 32UL))) != 0UL)
        {
            opMode = PM_FindOperateMode(i, pSysRescGroup);
            resourceDB[i].resourceConfigFunc(opMode, &resourceDB[i]);
        }
    }
}

static void PM_SetRAMOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint8_t tmp8;
    uint8_t sramId;

    tmp8 = pResourceRecode->defaultOperateMode;
    if (operateMode != tmp8)
    {
        tmp8 = operateMode;
    }

    /* The resource index maps directly to the SRAM slice ID for KW43. */
    sramId = (uint8_t)(pResourceRecode - resourceDB);

    switch (tmp8)
    {
        case PM_RESOURCE_FULL_ON:
        {
            CMC_PowerOnSRAMAllMode(CMC0, 1U << sramId);
            CMC_PowerOnSRAMLowPowerOnly(CMC0, 1U << sramId);
            break;
        }

        case PM_RESOURCE_PARTABLE_ON1:
        {
            CMC_PowerOnSRAMAllMode(CMC0, 1U << sramId);
            CMC_PowerOffSRAMLowPowerOnly(CMC0, 1U << sramId);
            break;
        }

        case PM_RESOURCE_OFF:
        {
            CMC_PowerOnSRAMLowPowerOnly(CMC0, 1U << sramId);
            CMC_PowerOffSRAMAllMode(CMC0, 1U << sramId);
            break;
        }

        default:
        {
            /* This branch will never be hit. */
            break;
        }
    }
    pResourceRecode->currentOperateMode = tmp8;
}

static void PM_SetFro192MOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint8_t tmp8;

    tmp8 = pResourceRecode->defaultOperateMode;

    if (operateMode != tmp8)
    {
        tmp8 = operateMode;
    }

    if (tmp8 == PM_RESOURCE_FULL_ON)
    {
        SCG0->FIRCCSR |= SCG_FIRCCSR_FIRCSTEN_MASK;
    }
    else
    {
        SCG0->FIRCCSR &= ~SCG_FIRCCSR_FIRCSTEN_MASK;
    }

    pResourceRecode->currentOperateMode = tmp8;
}

static void PM_SetFro6MOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint8_t tmp8;

    tmp8 = pResourceRecode->defaultOperateMode;

    if (operateMode != tmp8)
    {
        tmp8 = operateMode;
    }

    if (tmp8 == PM_RESOURCE_FULL_ON)
    {
        SCG0->SIRCCSR |= SCG_SIRCCSR_SIRCSTEN_MASK;
    }
    else
    {
        SCG0->SIRCCSR &= ~SCG_SIRCCSR_SIRCSTEN_MASK;
    }

    pResourceRecode->currentOperateMode = tmp8;
}

static void PM_SetWakePowerDomainOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode)
{
    if (operateMode == PM_RESOURCE_FULL_ON)
    {
        g_mainWakePDConfig.wake_domain = kCMC_ActiveMode;
    }
    else if (operateMode == PM_RESOURCE_PARTABLE_ON2)
    {
        if (g_mainWakePDConfig.main_domain >= kCMC_SleepMode)
        {
            g_mainWakePDConfig.wake_domain = kCMC_SleepMode;
        }
    }
    else if (operateMode == PM_RESOURCE_PARTABLE_ON1)
    {
        if (g_mainWakePDConfig.main_domain >= kCMC_DeepSleepMode)
        {
            g_mainWakePDConfig.wake_domain = kCMC_DeepSleepMode;
        }
    }
    else
    {
        /* MISRA C-2012 rule 15.7 */
    }

    pResourceRecode->currentOperateMode = operateMode;
}

/*
 * Program the radio 2.4 GHz low-power mode that matches the host power state.
 * KW43 drives the RF sub-system entry via RFMC.RF2P4GHZ_CTRL rather than through
 * the notification callback path used on KW47.
 */
static void PM_ApplyRadioLowPowerMode(uint8_t stateIndex)
{
    uint32_t rfCtrl;
    uint32_t lpMode;

    switch (stateIndex)
    {
        case PM_LP_STATE_SLEEP:
            lpMode = 0x1U;
            break;
        case PM_LP_STATE_DEEP_SLEEP:
            lpMode = 0x3U;
            break;
        case PM_LP_STATE_POWER_DOWN:
            lpMode = 0x7U;
            break;
        case PM_LP_STATE_DEEP_POWER_DOWN:
            RF_CMC1->RAM_PWR = 0x000004FFU;
            lpMode = 0xFU;
            break;
        default:
            return;
    }

    rfCtrl = RFMC->RF2P4GHZ_CTRL & ~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK;
    RFMC->RF2P4GHZ_CTRL = rfCtrl | RFMC_RF2P4GHZ_CTRL_LP_MODE(lpMode);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;
}

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
static status_t PM_ManageWakeupSource(pm_wakeup_source_t *ws, bool enable)
{
    assert(ws);

    uint32_t inputType;
    uint32_t inputId;
    uint32_t irqn;
    uint32_t misc;
    bool disableWuuIrq = false;

    PM_DECODE_WAKEUP_SOURCE_ID(ws->wsId);

    if (inputType == 0UL)
    {
        /* Wakeup source is external pin. */
        wuu_external_wakeup_pin_config_t pinConfig;

        pinConfig.edge  = enable ? (wuu_external_pin_edge_detection_t)misc : kWUU_ExternalPinDisable;
        pinConfig.event = kWUU_ExternalPinInterrupt;
        pinConfig.mode  = kWUU_ExternalPinActiveAlways;

        WUU_SetExternalWakeUpPinsConfig(WUU0, (uint8_t)inputId, &pinConfig);

        if (enable)
        {
            (void)EnableIRQ((IRQn_Type)irqn);
            (void)EnableIRQ(WUU0_IRQn);
        }
        else
        {
            (void)DisableIRQ((IRQn_Type)irqn);
            if ((WUU0->PE1 == 0UL) && (WUU0->PE2 == 0UL) && (WUU0->ME == 0UL))
            {
                (void)DisableIRQ(WUU0_IRQn);
            }
        }
    }
    else
    {
        if (enable)
        {
            /* Wakeup source is internal module. */
            (void)EnableIRQ(WUU0_IRQn);

            WUU_SetInternalWakeUpModulesConfig(WUU0, (uint8_t)inputId, (wuu_internal_wakeup_module_event_t)misc);
            if (misc == (uint32_t)kWUU_InternalModuleInterrupt)
            {
                (void)EnableIRQ((IRQn_Type)irqn);
            }
        }
        else
        {
            WUU_ClearInternalWakeUpModulesConfig(WUU0, (uint8_t)inputId, (wuu_internal_wakeup_module_event_t)misc);
            if (misc == (uint32_t)kWUU_InternalModuleInterrupt)
            {
                (void)DisableIRQ((IRQn_Type)irqn);
                if (WUU0->ME == 0UL)
                {
                    disableWuuIrq = true;
                }
            }
            else
            {
                if (WUU0->DE == 0UL)
                {
                    disableWuuIrq = true;
                }
            }
            if (disableWuuIrq && (WUU0->PE1 == 0UL) && (WUU0->PE2 == 0UL))
            {
                (void)DisableIRQ(WUU0_IRQn);
            }
        }
    }

    return kStatus_Success;
}

static bool PM_IsWakeupSource(pm_wakeup_source_t *ws)
{
    uint32_t inputType;
    uint32_t inputId;
    uint32_t irqn;
    uint32_t misc;
    bool ret = false;

    assert(ws != NULL);

    PM_DECODE_WAKEUP_SOURCE_ID(ws->wsId);

    /* Wakeup source is external pin. */
    if (inputType == 0UL)
    {
        if ((WUU_GetExternalWakeUpPinsFlag(WUU0) & (1UL << inputId)) != 0UL)
        {
            ret = true;
        }
    }

    (void)irqn;
    (void)misc;

    return ret;
}
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
