/*
 * Copyright 2024, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <setjmp.h>

#include "fsl_pm_core.h"
#include "fsl_pm_device.h"

#include "fsl_cmc.h"
#include "fsl_spc.h"
#include "fsl_wuu.h"
#include "fsl_clock.h"
#include "fsl_crc.h"
#include "fsl_vbat.h"

/*
 * $Coverage Justification Reference$
 *
 * $Justification pm_device_c_ref_1$
 * Following lines added to avoid MISRA violations and will never hit.
 *
 * $Justification pm_device_c_ref_2$
 * Set chip in low-power modes will disconnect debugger and it is unable to get coverage data.
 *
 * $Justification pm_device_c_ref_3$
 * Following lines will power off selected RAM block and it is unable to get coverage data.
 */

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define WUU_PIN_FLAG_MASK                                                                                              \
    (WUU_PF_WUF0_MASK | WUU_PF_WUF1_MASK |WUU_PF_WUF2_MASK | WUU_PF_WUF3_MASK | WUU_PF_WUF4_MASK | WUU_PF_WUF5_MASK | \
    WUU_PF_WUF6_MASK | WUU_PF_WUF7_MASK | WUU_PF_WUF8_MASK | WUU_PF_WUF9_MASK | WUU_PF_WUF10_MASK | WUU_PF_WUF11_MASK |\
    WUU_PF_WUF12_MASK | WUU_PF_WUF13_MASK | WUU_PF_WUF14_MASK | WUU_PF_WUF15_MASK | WUU_PF_WUF16_MASK | \
    WUU_PF_WUF17_MASK | WUU_PF_WUF18_MASK | WUU_PF_WUF19_MASK | WUU_PF_WUF20_MASK | WUU_PF_WUF21_MASK | \
    WUU_PF_WUF22_MASK | WUU_PF_WUF23_MASK | WUU_PF_WUF24_MASK | WUU_PF_WUF25_MASK | WUU_PF_WUF26_MASK | \
    WUU_PF_WUF27_MASK | WUU_PF_WUF28_MASK)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef struct _pm_resource_recode pm_resource_recode_t;

static void PM_WakeupFunction(void);
static void PM_EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void PM_CleanExitLowPowerMode(void);
static uint8_t PM_FindOperateMode(uint32_t rescIndex, pm_resc_group_t *pSysRescGroup);
static void PM_SetRAMOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode);
static void PM_SetFro192MOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode);
static void PM_SetFro6MOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode);
static void PM_SetWakePowerDomainOperateMode(uint8_t operateMode, pm_resource_recode_t *pResourceRecode);
static void PM_SaveRuntimeContext(void);
static void PM_RestoreRuntimeContext(void);

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
#define PM_SLEEP_FIX_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CoreClk))
#define PM_SLEEP_VAR_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CTCM0) | PM_RESC_TO_BIT(kResc_CTCM1) | \
                        PM_RESC_TO_BIT(kResc_STCM0) | PM_RESC_TO_BIT(kResc_STCM1) | PM_RESC_TO_BIT(kResc_STCM2) |\
                        PM_RESC_TO_BIT(kResc_STCM3) | PM_RESC_TO_BIT(kResc_STCM4) | PM_RESC_TO_BIT(kResc_STCM5) | \
                        PM_RESC_TO_BIT(kResc_STCM6) | PM_RESC_TO_BIT(kResc_STCM7) | PM_RESC_TO_BIT(kResc_STCM8) | \
                        PM_RESC_TO_BIT(kResc_Fro192M) | PM_RESC_TO_BIT(kResc_Fro6M) | \
                        PM_RESC_TO_BIT(kResc_WakePowerDomainPeri) | PM_RESC_TO_BIT(kResc_BusSysClk))

#define PM_DS_FIX_CONSTR_MASK   (PM_RESC_TO_BIT(kResc_CoreClk) | PM_RESC_TO_BIT(kResc_BusSysClk) |\
                                PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt2) | PM_RESC_TO_BIT(kResc_Fro192M))
#define PM_DS_VAR_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CTCM0) | PM_RESC_TO_BIT(kResc_CTCM1) | \
                        PM_RESC_TO_BIT(kResc_STCM0) | PM_RESC_TO_BIT(kResc_STCM1) | PM_RESC_TO_BIT(kResc_STCM2) |\
                        PM_RESC_TO_BIT(kResc_STCM3) | PM_RESC_TO_BIT(kResc_STCM4) | PM_RESC_TO_BIT(kResc_STCM5) | \
                        PM_RESC_TO_BIT(kResc_STCM6) | PM_RESC_TO_BIT(kResc_STCM7) | PM_RESC_TO_BIT(kResc_STCM8) | \
                        PM_RESC_TO_BIT(kResc_Fro6M) | PM_RESC_TO_BIT(kResc_WakePowerDomainPeri))

#define PM_PD_FIX_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CoreClk) | PM_RESC_TO_BIT(kResc_BusSysClk) | \
                            PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt1) | \
                            PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt2) | PM_RESC_TO_BIT(kResc_Fro192M))
#define PM_PD_VAR_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CTCM0) | PM_RESC_TO_BIT(kResc_CTCM1) | \
                        PM_RESC_TO_BIT(kResc_STCM0) | PM_RESC_TO_BIT(kResc_STCM1) | PM_RESC_TO_BIT(kResc_STCM2) |\
                        PM_RESC_TO_BIT(kResc_STCM3) | PM_RESC_TO_BIT(kResc_STCM4) | PM_RESC_TO_BIT(kResc_STCM5) | \
                        PM_RESC_TO_BIT(kResc_STCM6) | PM_RESC_TO_BIT(kResc_STCM7) | PM_RESC_TO_BIT(kResc_STCM8) | \
                        PM_RESC_TO_BIT(kResc_Fro6M) | PM_RESC_TO_BIT(kResc_WakePowerDomainPeri))

#define PM_DPD_FIX_CONSTR_MASK (PM_RESC_TO_BIT(kResc_CoreClk) | PM_RESC_TO_BIT(kResc_BusSysClk) | \
                            PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt1) | \
                            PM_RESC_TO_BIT(kResc_MainPowerDomainPeriOpt2) | PM_RESC_TO_BIT(kResc_Fro192M) | \
                            PM_RESC_TO_BIT(kResc_CTCM0) | PM_RESC_TO_BIT(kResc_CTCM1) | PM_RESC_TO_BIT(kResc_STCM0) | \
                            PM_RESC_TO_BIT(kResc_STCM1) | PM_RESC_TO_BIT(kResc_STCM2) | PM_RESC_TO_BIT(kResc_STCM3) | \
                            PM_RESC_TO_BIT(kResc_STCM4) | PM_RESC_TO_BIT(kResc_STCM5) | PM_RESC_TO_BIT(kResc_STCM6) | \
                            PM_RESC_TO_BIT(kResc_STCM7) | PM_RESC_TO_BIT(kResc_Fro6M))
#define PM_DPD_VAR_CONSTR_MASK (PM_RESC_TO_BIT(kResc_WakePowerDomainPeri) | PM_RESC_TO_BIT(kResc_STCM8))

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
                .exitLatency = 140U, /* 140 us */
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
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* STCM4 */
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* STCM5 */
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* STCM6 */
    {PM_RESOURCE_OFF, 0U, PM_SetRAMOperateMode},    /* STCM7 */
    {PM_RESOURCE_FULL_ON, 0U, PM_SetRAMOperateMode},    /* STCM8 */
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

        /*
         * $Line Coverage Justification$
         * $ref pm_device_c_ref_1$.
         */
        default:
        {
            /* This branch should never be hit. */
            break;
        }
    }
    PM_EnableBasicResources(pSoftRescMask, pSysRescGroup);
    CMC_SetPowerModeProtection(CMC0, (uint32_t)kCMC_AllowAllLowPowerModes);

    if ((stateIndex == PM_LP_STATE_SLEEP) || (stateIndex == PM_LP_STATE_DEEP_SLEEP))
    {
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
    }
    else
    {
        if (g_mainWakePDConfig.wake_domain <= g_mainWakePDConfig.main_domain)
        {
            /* The first word from wakeup address must be SP */
            g_wakeupEntry[0] = ((uint32_t)&m_warmboot_stack_end);

            /* The second word from wakeup address must be PC. */
            g_wakeupEntry[1] = (uint32_t)PM_WakeupFunction;

            for (uint32_t i = 2UL; i < 12UL; i++)
            {
                g_wakeupEntry[i] = i;
            }
            /* Set wake up address. */
            SPC_SetWakeUpValue(SPC0, (uint32_t)g_wakeupEntry);

            /* Don't spend time for crc calculation if crc value is already done, fields are unchanged */
            if (crcResult == 0UL)
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

            PM_SaveRuntimeContext();
            if (setjmp(g_coreContext) == 0)
            {
                CMC_EnterLowPowerMode(CMC0, &g_mainWakePDConfig);
            }
            PM_RestoreRuntimeContext();
        }
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
    if ((SPC_CheckPowerDomainLowPowerRequest(SPC0, kSPC_PowerDomain0) == true) &&
        (SPC_GetPowerDomainLowPowerMode(SPC0, kSPC_PowerDomain0) >= kSPC_PowerDownWithSysClockOff))
    {
        /*
         * $Line Coverage Justification$
         * $ref pm_device_c_ref_2$.
         */
        /* We need to release IO isolation when exiting from Power Down mode
         * This is done here after all peripherals have been reinitialized, to
         * avoid any glitch on IOs */
        SPC_ClearPeriphIOIsolationFlag(SPC0);
    }

    /* Clear SPC LP request status for next low power phase
     * The clean up API has been added because the modules registered to notification
     * could need to read some status registers (like the following one for KW47)
     * so we don't want a specific module to handle the clear of those status
     * as it could lead to some modules missing the status information
     * (depending on the notification order).
     * So this API is meant to clear everything needed for the platform once
     * every module got notified. */
    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain0);
    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain1);
    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain2);
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

    sramId = pResourceRecode - resourceDB;

    if (sramId == 10U)
    {
        if (tmp8 == PM_RESOURCE_OFF)
        {
            /*
            * $Line Coverage Justification$
            * $ref pm_device_c_ref_3$.
            */
            VBAT_EnableSRAMArrayRetained(VBAT0, false);
        }
        else
        {
            VBAT_EnableSRAMArrayRetained(VBAT0, true);
        }
    }
    else
    {
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

            /*
            * $Line Coverage Justification$
            * $ref pm_device_c_ref_3$.
            */
            case PM_RESOURCE_OFF:
            {
                CMC_PowerOnSRAMLowPowerOnly(CMC0, 1U << sramId);
                CMC_PowerOffSRAMAllMode(CMC0, 1U << sramId);
                break;
            }

            /*
            * $Line Coverage Justification$
            * $ref pm_device_c_ref_1$.
            */
            default:
            {
                /* This branch will never be hit. */
                break;
            }
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
    /*
     * $Branch Coverage Justification$
     * KW47 does not provided the software method to power off WAKE power domain.
     */
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

static void PM_WakeupFunction(void)
{
    /*
     * $Line Coverage Justification$
     * $ref pm_device_c_ref_2$
     */
    WDOG0->CS    = (uint32_t)((WDOG0->CS) & ~WDOG_CS_EN_MASK);
    WDOG0->TOVAL = 0xFFFF;
#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));    /* set CP10, CP11 Full Access in Secure mode */
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    SCB_NS->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10, CP11 Full Access in Non-secure mode */
#endif                                                    /* (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U) */
#endif                                                    /* ((__FPU_PRESENT == 1) && (__FPU_USED == 1)) */

    /* Restore the CONTROL register. */
    __set_CONTROL(g_cpuControl);
    longjmp(g_coreContext, (int32_t) true);
}

static void PM_SaveRuntimeContext(void)
{
    uint32_t i = 0UL;

    g_scbVtor  = SCB->VTOR;
    g_scbIcsr  = SCB->ICSR;
    g_scbAircr = ((SCB->AIRCR) & ~SCB_AIRCR_VECTKEY_Msk) | (0x5FAUL << (uint32_t)SCB_AIRCR_VECTKEY_Pos);
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

static void PM_RestoreRuntimeContext(void)
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
        pinConfig.mode  = kWUU_ExternalPinActiveDSPD;

        WUU_SetExternalWakeUpPinsConfig(WUU0, (uint8_t)inputId, &pinConfig);

        (void)irqn;
        if (enable)
        {
            (void)EnableIRQ(WUU0_IRQn);
        }
        else
        {
            if ((WUU0->PE1 == 0UL) && (WUU0->PE2 == 0UL))
            {
                disableWuuIrq = true;
            }

            if (disableWuuIrq == true)
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
            if (disableWuuIrq)
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
    uint32_t wuuPf;
    uint32_t mask;
    bool ret = false;

    assert(ws != NULL);

    PM_DECODE_WAKEUP_SOURCE_ID(ws->wsId);

    /* Wakeup source is external pin. */
    if (inputType == 0UL)
    {
        wuuPf = WUU_GetExternalWakeUpPinsFlag(WUU0);
        mask  = (1UL << inputId) & WUU_PIN_FLAG_MASK;

        if ((wuuPf & mask) != 0UL)
        {
            /*
             * $Line Coverage Justification$
             * $ref pm_device_c_ref_2$.
             */
            /* This wake up source triggered the last wake up */
            ret = true;
        }
    }

    (void)irqn;
    (void)misc;

    return ret;
}
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
