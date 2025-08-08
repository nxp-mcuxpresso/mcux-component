/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <setjmp.h>

#include "fsl_pm_core.h"
#include "fsl_pm_board.h"

#include "fsl_clock.h"
#include "fsl_power.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef struct _pm_board_resource_recode pm_board_resource_recode_t;

// static void PM_BOARD_WakeupFunction(void);
static void PM_BOARD_EnterLowPowerMode(uint8_t stateIndex,
                                       pm_resc_mask_t *pSoftRescMask,
                                       pm_resc_group_t *pSysRescGroup);
static void PM_BOARD_CleanExitLowPowerMode(void);
static uint8_t PM_BOARD_FindOperateMode(uint32_t rescIndex, pm_resc_group_t *pSysRescGroup);
static void PM_BOARD_SetRAMOperateMode(uint8_t operateMode, pm_board_resource_recode_t *pResourceRecode);
static void PM_BOARD_SetFro192MOperateMode(uint8_t operateMode, pm_board_resource_recode_t *pResourceRecode);
static void PM_BOARD_SetFro6MOperateMode(uint8_t operateMode, pm_board_resource_recode_t *pResourceRecode);
static void PM_BOARD_SetWakePowerDomainOperateMode(uint8_t operateMode, pm_board_resource_recode_t *pResourceRecode);
// static void PM_BOARD_SaveRuntimeContext(void);
// static void PM_BOARD_RestoreRuntimeContext(void);

static void PM_BOARD_EnableBasicResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
static status_t PM_BOARD_ManageWakeupSource(pm_wakeup_source_t *ws, bool enable);
static bool PM_BOARD_IsWakeupSource(pm_wakeup_source_t *ws);
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/** Holds the resources that are controlled by the PMC and are set as required resource */
// static uint32_t s_pm_board_EnabledResources     = 0;
/** Holds the wake up sources that are controlled by the PMC */
static uint64_t s_pm_board_EnabledWakeupSources = 0;

/*!
 *  |  Module  |    Sleep     | Deep Sleep | Power Down | Deep Power Down |
 *  |  CM33    |    Static    | Static     |    OFF     |       OFF       |
 *  | FRO-192M |   Optional   |    OFF     |    OFF     |       OFF       |
 *  | FRO-6M   |   Optional   |  Optional  |   Optional |       OFF       |
 *  | FRO-32K  |      ON      |     ON     |   Optional |     Optional    |
 *  | OSC-RTC  |      ON      |     ON     |   Optional |     Optional    |
 *  | OSC-RF   |      ON      |  Optional  |   Optional |       OFF       |
 *  |   SCG    |      ON      |   Static   |    Static  |       OFF       |
 *  |   PCC    |      ON      |   Static   |    OFF     |       OFF       |
 *  |  Flash   |  Optional LP |    OFF/LP  |    OFF     |       OFF       |
 *  |  SRAMx   |   Optional   |   Optional |   Optional |       OFF       |
 *  |  DCDC    |  Optional LP |  ON, LP or OFF | ON, LP or OFF | OFF |
 *  |  LDO-CORE | Optional LP |  ON, LP or OFF | ON, LP or OFF | OFF |
 *  |  LDO-SYS  | Optional LP |  ON, LP | ON, LP | ON, LP |
 *  |  VDD_CORE HVD | Optional | Optional | Optional | OFF |
 *  |  VDD_CORE LVD | Optional | Optional | Optional | OFF |
 *  |  VDD_IO_ABC HVD | Optional | Optional | Optional | OFF |
 *  |  VDD_IO_ABC LVD | Optional | Optional | Optional | OFF |
 *  |  VDD_SYS HVD    | Optional | Optional | Optional | OFF |
 *  |  VDD_SYS LVD    | Optional | Optional | Optional | OFF |
 */
const pm_device_option_t g_devicePMOption = {
    .states =
        {
            /* WFI or PM_LP_STATE_SLEEP */
            {
                .exitLatency = 0U, /* no latency - Always allowed*/
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = 0x0000UL, // 0x2000UL,
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = 0xFFFFUL, // 0x13FFUL,
                    },
            },
            /* Deep Sleep */
            {
                .exitLatency = 3000U, /* Does not allow Deep sleep if low power duration is below this time */
                .fixConstraintsMask =
                    {
                        .rescMask[0] =
                            0x0000UL, //  for now, all available in deep sleep, give no restriction (was 0x3880UL),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0] = 0xFFFFUL, // 0x33FUL,
                    },
            },
            /* Power Down */
            {
                .exitLatency = 3000U, /* */
                .fixConstraintsMask =
                    {
                        .rescMask[0] = 0x0000UL,
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0] = 0xFFFFUL,
                    },
            },
            /* Deep Power Down */
            {
                .exitLatency = 5000000U, /* 7 us */
                .fixConstraintsMask =
                    {
                        .rescMask[0] = 0x0000UL, //  for now, all available in deep sleep, give no restriction
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0] = 0xFFFFUL,
                    },
            },

        },
    .stateCount = 4U,
    .enter      = PM_BOARD_EnterLowPowerMode,
    .clean      = PM_BOARD_CleanExitLowPowerMode,

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
    .manageWakeupSource = PM_BOARD_ManageWakeupSource,
    .isWakeupSource     = PM_BOARD_IsWakeupSource,
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
};

struct _pm_board_resource_recode
{
    uint8_t defaultOperateMode : 4U;
    uint8_t currentOperateMode : 4U;
    void (*resourceConfigFunc)(uint8_t operateMode, pm_board_resource_recode_t *pResourceRecode);
};

#define PM_RESOURCE_DB_COUNT (10U)

AT_ALWAYS_ON_DATA_INIT(pm_board_resource_recode_t resourceDB[PM_RESOURCE_DB_COUNT]) = {
    {PM_RESOURCE_OFF, 0U, PM_BOARD_SetRAMOperateMode},
    {PM_RESOURCE_OFF, 0U, PM_BOARD_SetRAMOperateMode},
    {PM_RESOURCE_OFF, 0U, PM_BOARD_SetRAMOperateMode},
    {PM_RESOURCE_OFF, 0U, PM_BOARD_SetRAMOperateMode},
    {PM_RESOURCE_OFF, 0U, PM_BOARD_SetRAMOperateMode},
    {PM_RESOURCE_OFF, 0U, PM_BOARD_SetRAMOperateMode},
    {PM_RESOURCE_OFF, 0U, PM_BOARD_SetRAMOperateMode},
    {PM_RESOURCE_OFF, 0U, PM_BOARD_SetFro192MOperateMode},
    {PM_RESOURCE_OFF, 0U, PM_BOARD_SetFro6MOperateMode},
    {PM_RESOURCE_FULL_ON, PM_RESOURCE_FULL_ON, PM_BOARD_SetWakePowerDomainOperateMode},
};
#if 0
AT_ALWAYS_ON_DATA_INIT(cmc_power_domain_config_t g_mainWakePDConfig) = {
    .clock_mode = kCMC_GateAllSystemClocksEnterLowPowerMode,
    /* we chose to put the main and wake domain in retention (deep sleep) by default if no constraints are set */
    .main_domain = kCMC_DeepSleepMode,
    .wake_domain = kCMC_DeepSleepMode,
};
#endif

AT_ALWAYS_ON_DATA(jmp_buf g_coreContext);
AT_ALWAYS_ON_DATA(uint32_t g_scbVtor);
AT_ALWAYS_ON_DATA(uint32_t g_scbIcsr);
AT_ALWAYS_ON_DATA(uint32_t g_scbAircr);
AT_ALWAYS_ON_DATA(uint32_t g_scbCCR);
AT_ALWAYS_ON_DATA(uint8_t g_scbShp[12]);
AT_ALWAYS_ON_DATA(uint32_t g_scbShcsr);
AT_ALWAYS_ON_DATA(uint32_t g_nvicIser[16U]);
AT_ALWAYS_ON_DATA(uint8_t g_nvicIp[496U]);
AT_ALWAYS_ON_DATA(uint32_t g_systickCtrl);
AT_ALWAYS_ON_DATA(uint32_t g_systickLoad);
AT_ALWAYS_ON_DATA(uint32_t g_cpuControl);
AT_ALWAYS_ON_DATA(uint32_t g_wakeupEntry[12]);
AT_ALWAYS_ON_DATA(uint32_t crcResult);

extern uint32_t m_warmboot_stack_end;

/*******************************************************************************
 * Code
 ******************************************************************************/
#define EXCLUDE_PD_DEEP_SLEEP (kLOWPOWERCFG_DCDC | kLOWPOWERCFG_XTAL32K | kLOWPOWERCFG_BLE_WUP)
// #define EXCLUDE_PD_DEEP_SLEEP  (kLOWPOWERCFG_DCDC | kLOWPOWERCFG_XTAL32K )  // this does not
// work,kLOWPOWERCFG_BLE_WUP is required #define EXCLUDE_PD_DEEP_SLEEP  ( kLOWPOWERCFG_XTAL32K | kLOWPOWERCFG_BLE_WUP )
// // this improves low power consumption from 36 to 14uA in Deep sleep #define EXCLUDE_PD_DEEP_SLEEP  (
// kLOWPOWERCFG_DCDC | kLOWPOWERCFG_XTAL32K  )   // 14uA but BLE no longer works

static void PM_BOARD_EnterLowPowerMode(uint8_t stateIndex,
                                       pm_resc_mask_t *pSoftRescMask,
                                       pm_resc_group_t *pSysRescGroup)
{
    // uint8_t rescMaskArrayIndex = 0U;
    // uint8_t rescMaskOffset     = 0U;

#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
    PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */

    switch (stateIndex)
    {
        /* WFI */
        case PM_LP_STATE_SLEEP:
        {
            /* reduce power consumption on WFI by switching to 12Mhz clock
                 if a component requires WFI @32Mhz,  need to create a constraint to avoid 12Mhz */
            SYSCON->MAINCLKSELA = 0;

            POWER_EnterSleep();

            /* Exit WFI at 32Mhz */
            SYSCON->MAINCLKSELA = 3;

            break;
        }

        /* Deep Sleep */
        case PM_LP_STATE_DEEP_SLEEP:
        {
            status_t status;

            status = POWER_EnterDeepSleep(
                EXCLUDE_PD_DEEP_SLEEP,
                kWAKEUP_BLE_SLP_TMR | kWAKEUP_RTC); // kWAKEUP_BLE_LL , s_pm_board_EnabledWakeupSources
            assert(status == kStatus_Success);
            (void)status;
            break;
        }

        case PM_LP_STATE_POWER_DOWN:
        {
            // assert(0); // not implemented yet
            // break;
        }

        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            status_t status = POWER_EnterPowerOff(
                0 // | kLOWPOWERCFG_DCDC_BYPASS
                /* BITMASK_ISSET(requiredResources, kResource_WakeupWakePad) ? kLOWPOWERCFG_DCDC_BYPASS : 0) */,
                kWAKEUP_PIN_ENABLE | kWAKEUP_PIN_PUP_EN | kWAKEUP_PIN_WAKEUP_LOW_LVL);
            assert(status == kStatus_Success);
            (void)status;
            break;
        }

        default:
        {
            /* This branch should never be hit. */
            break;
        }
    }
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
    PM_StopAndRecordTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
    PM_BOARD_EnableBasicResources(pSoftRescMask, pSysRescGroup);
#if 0
    CMC_SetPowerModeProtection(CMC0, kCMC_AllowAllLowPowerModes);
    if ((stateIndex == PM_LP_STATE_SLEEP) || (stateIndex == PM_LP_STATE_DEEP_SLEEP))
    {
        CMC_EnterLowPowerMode(CMC0, &g_mainWakePDConfig);
    }
    else
    {
        if (g_mainWakePDConfig.wake_domain <= g_mainWakePDConfig.main_domain)
        {
            /* The first word from wakeup address must be SP */
            g_wakeupEntry[0] = ((uint32_t)&m_warmboot_stack_end);

            /* The second word from wakeup address must be PC. */
            g_wakeupEntry[1] = (uint32_t)PM_BOARD_WakeupFunction;

            for (uint32_t i = 2UL; i < 12UL; i++)
            {
                g_wakeupEntry[i] = i;
            }
            /* Set wake up address. */
            SPC_SetWakeUpValue(SPC0, (uint32_t)g_wakeupEntry);

            /* Don't spend time for crc calculation if crc value is already done, fields are unchanged */
            if (crcResult == 0)
            {
                crc_config_t config;
                config.polynomial         = 0x04C11DB7U;
                config.seed               = 0xFFFFFFFFU;
                config.reflectIn          = false;
                config.reflectOut         = false;
                config.complementChecksum = false;
                config.crcBits            = kCrcBits32;
                config.crcResult          = kCrcFinalChecksum;

                /* Need to disable clock before CRC_init() to prevent hardfault when going to lowpower */
                CLOCK_DisableClock(kCLOCK_Crc0);
                CRC_Init(CRC0, &config);
                CRC_WriteData(CRC0, (const uint8_t *)((uint32_t)g_wakeupEntry), 48);
                crcResult = CRC_Get32bitResult(CRC0);
            }
            REGFILE1->REG[0] = crcResult;

            PM_BOARD_SaveRuntimeContext();
            if (setjmp(g_coreContext) == 0)
            {
                CMC_EnterLowPowerMode(CMC0, &g_mainWakePDConfig);
            }
            PM_BOARD_RestoreRuntimeContext();
        }
    }
#endif
}

static void PM_BOARD_CleanExitLowPowerMode(void)
{
#if 0
    if ((SPC_CheckPowerDomainLowPowerRequest(SPC0, kSPC_PowerDomain0) == true) &&
        (SPC_GetPowerDomainLowPowerMode(SPC0, kSPC_PowerDomain0) >= kSPC_PowerDownWithSysClockOff))
    {
        /* We need to release IO isolation when exiting from Power Down mode
         * This is done here after all peripherals have been reinitialized, to
         * avoid any glitch on IOs */
        SPC_ClearPeriphIOIsolationFlag(SPC0);
    }

    /* Clear SPC LP request status for next low power phase
     * The clean up API has been added because the modules registered to notification
     * could need to read some status registers (like the following one for KW45)
     * so we don't want a specific module to handle the clear of those status
     * as it could lead to some modules missing the status information
     * (depending on the notification order).
     * So this API is meant to clear everything needed for the platform once
     * every module got notified. */
    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain0);
    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain1);
    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain2);
#endif
}

static uint8_t PM_BOARD_FindOperateMode(uint32_t rescIndex, pm_resc_group_t *pSysRescGroup)
{
    assert(pSysRescGroup);
    uint8_t u8Tmp = (pSysRescGroup->groupSlice[rescIndex / 8UL] >> (4UL * (rescIndex % 8UL))) & 0xFUL;

    u8Tmp |= (u8Tmp >> 1U);
    u8Tmp |= (u8Tmp >> 2U);

    return ((u8Tmp + 1U) >> 1U);
}

static void PM_BOARD_EnableBasicResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    assert(pSoftRescMask);
    assert(pSysRescGroup);

    uint32_t i     = 0UL;
    uint8_t opMode = PM_RESOURCE_OFF;

    for (i = 0UL; i < PM_RESOURCE_DB_COUNT; i++)
    {
        if ((pSoftRescMask->rescMask[i / 32UL] & (1UL << (i % 32UL))) != 0UL)
        {
            opMode = PM_BOARD_FindOperateMode(i, pSysRescGroup);
            if (opMode != resourceDB[i].currentOperateMode)
            {
                resourceDB[i].resourceConfigFunc(opMode, &resourceDB[i]);
            }
        }
    }
}

static void PM_BOARD_SetRAMOperateMode(uint8_t operateMode, pm_board_resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);
#if 0
    uint8_t tmp8;
    uint8_t sramId;

    tmp8 = pResourceRecode->defaultOperateMode;
    if (operateMode != tmp8)
    {
        tmp8 = operateMode;
    }

    sramId = pResourceRecode - resourceDB;

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
#endif
}

static void PM_BOARD_SetFro192MOperateMode(uint8_t operateMode, pm_board_resource_recode_t *pResourceRecode)
{
#if 0
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
#endif
}

static void PM_BOARD_SetFro6MOperateMode(uint8_t operateMode, pm_board_resource_recode_t *pResourceRecode)
{
#if 0
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
#endif
}

static void PM_BOARD_SetWakePowerDomainOperateMode(uint8_t operateMode, pm_board_resource_recode_t *pResourceRecode)
{
#if 0
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

    pResourceRecode->currentOperateMode = operateMode;
#endif
}
#if 0
static void PM_BOARD_WakeupFunction(void)
{
#if 0
    WDOG0->CS    = (uint32_t)((WDOG0->CS) & ~WDOG_CS_EN_MASK);
    WDOG0->TOVAL = 0xFFFF;
#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10, CP11 Full Access in Secure mode */
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    SCB_NS->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10, CP11 Full Access in Non-secure mode */
#endif /* (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U) */
#endif /* ((__FPU_PRESENT == 1) && (__FPU_USED == 1)) */

    /* Restore the CONTROL register. */
    __set_CONTROL(g_cpuControl);
    longjmp(g_coreContext, true);
#endif
}

static void PM_BOARD_SaveRuntimeContext(void)
{
    uint32_t i = 0UL;

    g_scbVtor  = SCB->VTOR;
    g_scbIcsr  = SCB->ICSR;
    g_scbAircr = ((SCB->AIRCR) & ~SCB_AIRCR_VECTKEY_Msk) | (0x5FA << SCB_AIRCR_VECTKEY_Pos);
    g_scbCCR   = SCB->CCR;
    for (i = 0UL; i < 12UL; i++)
    {
        g_scbShp[i] = SCB->SHPR[i];
    }
    g_scbShcsr = SCB->SHCSR;
    for (i = 0UL; i < 16U; i++)
    {
        g_nvicIser[i] = NVIC->ISER[i];
    }
    for (i = 0UL; i < 496UL; i++)
    {
        g_nvicIp[i] = NVIC->IPR[i];
    }
    g_cpuControl  = __get_CONTROL();
    g_systickCtrl = SysTick->CTRL;
    g_systickLoad = SysTick->LOAD;
}

static void PM_BOARD_RestoreRuntimeContext(void)
{
    uint32_t i;

    SCB->VTOR  = g_scbVtor;
    SCB->ICSR  = g_scbIcsr;
    SCB->AIRCR = g_scbAircr;
    SCB->CCR   = g_scbCCR;
    for (i = 0UL; i < 12UL; i++)
    {
        SCB->SHPR[i] = g_scbShp[i];
    }
    SCB->SHCSR = g_scbShcsr;
    for (i = 0UL; i < 16UL; i++)
    {
        NVIC->ISER[i] = g_nvicIser[i];
    }
    for (i = 0UL; i < 496UL; i++)
    {
        NVIC->IPR[i] = g_nvicIp[i];
    }
    SysTick->CTRL = g_systickCtrl;
    SysTick->LOAD = g_systickLoad;
}
#endif
#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
static status_t PM_BOARD_ManageWakeupSource(pm_wakeup_source_t *ws, bool enable)
{
    assert(ws);

    uint32_t inputType;
    uint32_t inputId;
    uint32_t irqn;
    uint32_t misc;

    PM_DECODE_WAKEUP_SOURCE_ID(ws->wsId);

    (void)irqn;
    (void)misc;

    if ((inputType == 0UL) || 1)
    {
        if (enable)
        {
            s_pm_board_EnabledWakeupSources |= inputId;
        }
        else
        {
            s_pm_board_EnabledWakeupSources &= ~inputId;
        }
#if 0
        /* Wakeup source is external pin. */
        wuu_external_wakeup_pin_config_t pinConfig;

        pinConfig.edge  = enable ? (wuu_external_pin_edge_detection_t)misc : kWUU_ExternalPinDisable;
        pinConfig.event = kWUU_ExternalPinInterrupt;
        pinConfig.mode  = kWUU_ExternalPinActiveDSPD;

        WUU_SetExternalWakeUpPinsConfig(WUU0, inputId, &pinConfig);

        if (enable)
        {
            EnableIRQ((IRQn_Type)irqn);
            EnableIRQ(WUU0_IRQn);
        }
        else
        {
            DisableIRQ((IRQn_Type)irqn);
            DisableIRQ(WUU0_IRQn);
        }
#endif
    }
#if 0
    else
    {
        if (enable)
        {
            /* Wakeup source is internal module. */
            EnableIRQ(WUU0_IRQn);
            WUU_SetInternalWakeUpModulesConfig(WUU0, inputId, kWUU_InternalModuleInterrupt);
            EnableIRQ((IRQn_Type)irqn);
        }
        else
        {
            WUU0->ME &= ~(1UL << inputId);
            DisableIRQ((IRQn_Type)irqn);
            DisableIRQ(WUU0_IRQn);
        }
    }
#endif

    return kStatus_Success;
}

static bool PM_BOARD_IsWakeupSource(pm_wakeup_source_t *ws)
{
    bool ret = false;
#if 0
    uint32_t inputType;
    uint32_t inputId;
    uint32_t irqn;
    uint32_t misc;
    uint32_t wuuPf;
    uint32_t mask;

    assert(ws != NULL);

    PM_DECODE_WAKEUP_SOURCE_ID(ws->wsId);

    /* Wakeup source is external pin. */
    if (inputType == 0UL)
    {
        wuuPf = WUU_GetExternalWakeUpPinsFlag(WUU0);
        mask  = (1U << inputId) & WUU_PIN_FLAG_MASK;

        if ((wuuPf * mask) != 0U)
        {
            /* This wake up source triggered the last wake up */
            ret = true;
        }
    }

    (void)irqn;
    (void)misc;
#endif
    return ret;
}
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
