/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pm_device.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device_config.h"
#include "fsl_cmc.h"
#include "fsl_spc.h"
#include "fsl_wuu.h"
#include "fsl_clock.h"
#include "fsl_vbat.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef struct _resource_recode resource_recode_t;
static uint8_t g_nextPowerMode;

static void EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void CleanExitLowPowerMode(void);

static void SetSystemPeripheralPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode);
static void SetVoltagePeripheralPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode);
static void SetSramPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode);

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
static status_t ManageWakeupSource(pm_wakeup_source_t *ws, bool enable);
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_ALWAYS_ON_DATA_INIT(cmc_power_domain_config_t g_powerDomainConfig) = {
    .clock_mode = kCMC_GateNoneClock,
    .main_domain = kCMC_ActiveOrSleepMode,
    .wake_domain = kCMC_ActiveOrSleepMode,
};

pm_device_option_t g_devicePMOption = {
    .states =
        {
            /* Sleep */
            {
                .exitLatency = 1U,
            },
            /* Deep Sleep */
            {
                .exitLatency = 9U,
            },
            /* Power Down */
            {
                .exitLatency = 10U,
            },
            /* Deep Power Down */
            {
                .exitLatency = 5600U,
            },
        },
    .stateCount = PM_LP_STATE_COUNT,
    .enter      = EnterLowPowerMode,
    .clean      = CleanExitLowPowerMode,

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
    .manageWakeupSource = ManageWakeupSource,
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
};

struct _resource_recode
{
    uint8_t currentMode : 4U;
    void (*resourceConfigFunc)(uint8_t operateMode, resource_recode_t *pResourceRecode);
};

uint8_t rescArray[PM_CONSTRAINT_COUNT][kResc_PowerMode_Max] = {
    /* System modules */
    [kResc_CodeCache] = {KReac_Status_Hold, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_Fro_180M]  = {kResc_Status_On,   kResc_Status_On,   kResc_Status_Off,  kResc_Status_Off},
    [kResc_Fro_12M]   = {kResc_Status_On,   kResc_Status_On,   kResc_Status_On,   kResc_Status_Off},
    [kResc_Fro_16K]   = {kResc_Status_On,   kResc_Status_On,   kResc_Status_On,   kResc_Status_On},
    [kResc_Osc_Rtc]   = {kResc_Status_On,   kResc_Status_On,   kResc_Status_On,   kResc_Status_On},
    [kResc_Osc_Sys]   = {kResc_Status_On,   kResc_Status_On,   kResc_Status_Off,  kResc_Status_Off},
    [kResc_Apll]      = {kResc_Status_On,   kResc_Status_On,   kResc_Status_Off,  kResc_Status_Off},
    [kResc_Spll]      = {kResc_Status_On,   kResc_Status_On,   kResc_Status_Off,  kResc_Status_Off},

    /* Power domain modules */
    [kResc_LdoCore]          = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_Off},
    [kResc_RamRetentionLdo]  = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_On},
    [kResc_Core_Vdd_Ivs]     = {kResc_Status_Off, kResc_Status_On, kResc_Status_On, kResc_Status_Off},
    [kResc_Core_Vdd_Lvd_Act] = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_Off},
    [kResc_Core_Vdd_Hvd_Act] = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_Off},
    [kResc_Sys_Vdd_Lvd_Act]  = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_On},
    [kResc_Sys_Vdd_Hvd_Act]  = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_On},
    [kResc_Io_Vdd_Lvd_Act]   = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_Off},
    [kResc_Io_Vdd_Hvd_Act]   = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_Off},
    [kResc_Core_Vdd_Lvd_Lp]  = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_Off},
    [kResc_Core_Vdd_Hvd_Lp]  = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_Off},
    [kResc_Sys_Vdd_Lvd_Lp]   = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_On},
    [kResc_Sys_Vdd_Hvd_Lp]   = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_On},
    [kResc_Io_Vdd_Lvd_Lp]    = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_Off},
    [kResc_Io_Vdd_Hvd_Lp]    = {kResc_Status_On,  kResc_Status_On, kResc_Status_On, kResc_Status_Off},

    /* SRAM modules */
    [kResc_RamA0] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, KReac_Status_Hold},
    [kResc_RamA1] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, KReac_Status_Hold},
    [kResc_RamA2] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, KReac_Status_Hold},
    [kResc_RamA3] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, KReac_Status_Hold},
    [kResc_RamX0] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_RamX1] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_RamX2] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_RamB0] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_RamC0] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_RamC1] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_RamD0] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_RamD1] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_RamE0] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
    [kResc_RamE1] = {kResc_Status_On, KReac_Status_Hold, KReac_Status_Hold, kResc_Status_Off},
};

AT_ALWAYS_ON_DATA_INIT(resource_recode_t resourceDB[kResc_Max_Num]) = {
    /* System modules */
    [kResc_CodeCache] = {PM_RESOURCE_FULL_ON, SetSystemPeripheralPowerMode},
    [kResc_Fro_180M]  = {PM_RESOURCE_FULL_ON, SetSystemPeripheralPowerMode},
    [kResc_Fro_12M]   = {PM_RESOURCE_FULL_ON, SetSystemPeripheralPowerMode},
    [kResc_Fro_16K]   = {PM_RESOURCE_OFF, SetSystemPeripheralPowerMode},
    [kResc_Osc_Rtc]   = {PM_RESOURCE_OFF, SetSystemPeripheralPowerMode},
    [kResc_Osc_Sys]   = {PM_RESOURCE_OFF, SetSystemPeripheralPowerMode},
    [kResc_Apll]      = {PM_RESOURCE_OFF, SetSystemPeripheralPowerMode},
    [kResc_Spll]      = {PM_RESOURCE_OFF, SetSystemPeripheralPowerMode},

    /* Power domain modules */
    [kResc_LdoCore]          = {PM_RESOURCE_FULL_ON, SetVoltagePeripheralPowerMode},
    [kResc_RamRetentionLdo]  = {PM_RESOURCE_FULL_ON, SetVoltagePeripheralPowerMode},
    [kResc_Core_Vdd_Ivs]     = {PM_RESOURCE_FULL_ON, SetVoltagePeripheralPowerMode},
    [kResc_Core_Vdd_Lvd_Act] = {PM_RESOURCE_FULL_ON, SetVoltagePeripheralPowerMode},
    [kResc_Core_Vdd_Hvd_Act] = {PM_RESOURCE_FULL_ON, SetVoltagePeripheralPowerMode},
    [kResc_Sys_Vdd_Lvd_Act]  = {PM_RESOURCE_FULL_ON, SetVoltagePeripheralPowerMode},
    [kResc_Sys_Vdd_Hvd_Act]  = {PM_RESOURCE_FULL_ON, SetVoltagePeripheralPowerMode},
    [kResc_Io_Vdd_Lvd_Act]   = {PM_RESOURCE_FULL_ON, SetVoltagePeripheralPowerMode},
    [kResc_Io_Vdd_Hvd_Act]   = {PM_RESOURCE_FULL_ON, SetVoltagePeripheralPowerMode},
    [kResc_Core_Vdd_Lvd_Lp]  = {PM_RESOURCE_OFF, SetVoltagePeripheralPowerMode},
    [kResc_Core_Vdd_Hvd_Lp]  = {PM_RESOURCE_OFF, SetVoltagePeripheralPowerMode},
    [kResc_Sys_Vdd_Lvd_Lp]   = {PM_RESOURCE_OFF, SetVoltagePeripheralPowerMode},
    [kResc_Sys_Vdd_Hvd_Lp]   = {PM_RESOURCE_OFF, SetVoltagePeripheralPowerMode},
    [kResc_Io_Vdd_Lvd_Lp]    = {PM_RESOURCE_OFF, SetVoltagePeripheralPowerMode},
    [kResc_Io_Vdd_Hvd_Lp]    = {PM_RESOURCE_OFF, SetVoltagePeripheralPowerMode},

    /* SRAM modules */
    [kResc_RamA0] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamA1] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamA2] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamA3] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamX0] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamX1] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamX2] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamB0] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamC0] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamC1] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamD0] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamD1] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamE0] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
    [kResc_RamE1] = {PM_RESOURCE_FULL_ON, SetSramPowerMode},
};

/*******************************************************************************
 * Code
 ******************************************************************************/
void ConstructResourceInformation(uint8_t rescArray[][kResc_PowerMode_Max])
{
    for (uint8_t resc = 0U; resc < kResc_Max_Num; resc++)
    {
        for (uint8_t mod = 0U; mod < kResc_PowerMode_Max; ++mod)
        {
            g_devicePMOption.states[mod].varConstraintsMask.rescMask[resc / 32U] &= ~(1UL << (resc % 32U));
            g_devicePMOption.states[mod].fixConstraintsMask.rescMask[resc / 32U] |= (1UL << (resc % 32U));

            if (rescArray[resc][mod] != kResc_Status_Off)
            {
                g_devicePMOption.states[mod].varConstraintsMask.rescMask[resc / 32U] |= (1UL << (resc % 32U));
                g_devicePMOption.states[mod].fixConstraintsMask.rescMask[resc / 32U] &= ~(1UL << (resc % 32U));
            }
        }
    }
}

static void ControlResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    assert(pSoftRescMask);
    assert(pSysRescGroup);

    uint8_t opMode = 0U;

    for (uint32_t i = 0UL; i < (uint32_t)PM_CONSTRAINT_COUNT; i++)
    {
        if (resourceDB[i].resourceConfigFunc != NULL)
        {
            opMode = (uint8_t)(uint32_t)(PM_RESC_GROUP(pSysRescGroup, i));
            if (opMode != resourceDB[i].currentMode)
            {
                resourceDB[i].resourceConfigFunc(opMode, &resourceDB[i]);
            }
        }
    }
}

/*! Notes: 
 * 1. If the DCDC_VDD_LVL is not 00b, the CORE_LDO_VDD_LVL must set to the same 
 *      value as DCDC_VDD_LVL, even the LDO_CORE is off.
 * 2. If the DCDC_VDD_LVL is 00b, the CORE_LDO_VDD_LVL can be different with 
 *      DCDC_VDD_LVL, and the VDD_CORE_LVD must disabled.
 * 3. Require BG enabled
 *      1. Set CORELDO_VDD_DS/SYSLDO_VDD_DS/DCDC_VDD_DS to normal.
 *      2. VDD_CORE glitch enabled.
 *      3. Any HVDs/LVDs enabled
 * 4. Set CORELDO_VDD_DS/SYSLDO_VDD_DS to low requires LVDs/HVDs disabled.
 * 5. If the CORE_LDO_VDD_DS is set to the same value in both AC and LP mode,
 *      the CORE_LDO_VDD_LVL must set to the same voltage level.
 * 6. CORE_LDO_VDD_LVL can be changed only when AC mode CORE_LDO_VDD_DS=1
*/
static void EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    assert(pSoftRescMask);
    assert(pSysRescGroup);

    g_nextPowerMode = stateIndex;

    ControlResources(pSoftRescMask, pSysRescGroup);

    switch (stateIndex)
    {
        case PM_LP_STATE_SLEEP:
        {
            g_powerDomainConfig.clock_mode  = kCMC_GateCoreClock;
            g_powerDomainConfig.main_domain = kCMC_ActiveOrSleepMode;
            g_powerDomainConfig.wake_domain = kCMC_ActiveOrSleepMode;

            SPC_SetLowPowerModeBandgapmodeConfig(SPC0, kSPC_BandgapEnabledBufferDisabled);
            spc_lowpower_mode_dcdc_option_t dcdcOpt = {
              .DCDCVoltage       = kSPC_DCDC_MidVoltage,
              .DCDCDriveStrength = kSPC_DCDC_NormalDriveStrength, /* requires BG enabled. */
            };
            SPC_SetLowPowerModeDCDCRegulatorConfig(SPC0, &dcdcOpt);

            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            g_powerDomainConfig.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_powerDomainConfig.main_domain = kCMC_DeepSleepMode;
            g_powerDomainConfig.wake_domain = kCMC_DeepSleepMode;
            CMC_SetPowerModeProtection(CMC0, (uint32_t)kCMC_AllowDeepSleepMode);
            
            spc_lowpower_mode_dcdc_option_t dcdcOpt = {
              .DCDCVoltage       = kSPC_DCDC_MidVoltage,
              .DCDCDriveStrength = kSPC_DCDC_LowDriveStrength,
            };
            SPC_SetLowPowerModeDCDCRegulatorConfig(SPC0, &dcdcOpt);
            
            break;
        }
        case PM_LP_STATE_POWER_DOWN:
        {
            g_powerDomainConfig.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_powerDomainConfig.main_domain = kCMC_PowerDownMode;
            g_powerDomainConfig.wake_domain = kCMC_PowerDownMode;
            CMC_SetPowerModeProtection(CMC0, (uint32_t)kCMC_AllowPowerDownMode);
            
            spc_lowpower_mode_dcdc_option_t dcdcOpt = {
              .DCDCVoltage       = kSPC_DCDC_RetentionVoltage,
              .DCDCDriveStrength = kSPC_DCDC_LowDriveStrength,
            };
            SPC_SetLowPowerModeDCDCRegulatorConfig(SPC0, &dcdcOpt);

            SPC_SetLowPowerWakeUpDelay(SPC0, 0x3BBU);
            
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            g_powerDomainConfig.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
            g_powerDomainConfig.main_domain = kCMC_DeepPowerDown;
            g_powerDomainConfig.wake_domain = kCMC_DeepPowerDown;
            CMC_SetPowerModeProtection(CMC0, (uint32_t)kCMC_AllowDeepPowerDownMode);
            
            spc_lowpower_mode_dcdc_option_t dcdcOpt = {
              .DCDCVoltage       = kSPC_DCDC_MidVoltage,
              .DCDCDriveStrength = kSPC_DCDC_LowDriveStrength,
            };
            SPC_SetLowPowerModeDCDCRegulatorConfig(SPC0, &dcdcOpt);

            SPC_SetLowPowerWakeUpDelay(SPC0, 0x3BBU);
            break;
        }
        default:
        {
            assert(0);
            break;
        }
    }

#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
    PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
    CMC_EnterLowPowerMode(CMC0, &g_powerDomainConfig);
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
    PM_StopAndRecordTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
}

static void CleanExitLowPowerMode(void)
{
    if ((SPC_CheckPowerDomainLowPowerRequest(SPC0, kSPC_PowerDomain0) == true) &&
        (SPC_GetPowerDomainLowPowerMode(SPC0, kSPC_PowerDomain0) >= kSPC_PowerDownWithSysClockOff))
    {
        SPC_ClearPeriphIOIsolationFlag(SPC0);
    }

    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain0);
    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain1);
    SPC_ClearLowPowerRequest(SPC0);

    /* LVDs/HVDs should be enabled in active mode */
    SPC_EnableActiveModeCoreLowVoltageDetect(SPC0, true);
    SPC_EnableActiveModeCoreHighVoltageDetect(SPC0, true);
    SPC_EnableActiveModeSystemLowVoltageDetect(SPC0, true);
    SPC_EnableActiveModeSystemHighVoltageDetect(SPC0, true);
    SPC_EnableActiveModeIOLowVoltageDetect(SPC0, true);
    SPC_EnableActiveModeIOHighVoltageDetect(SPC0, true);

    /* LVDs/HVDs should be disabled in low power mode */
    SPC_EnableLowPowerModeCoreLowVoltageDetect(SPC0, false);
    SPC_EnableLowPowerModeCoreHighVoltageDetect(SPC0, false);
    SPC_EnableLowPowerModeSystemLowVoltageDetect(SPC0, false);
    SPC_EnableLowPowerModeSystemHighVoltageDetect(SPC0, false);
    SPC_EnableLowPowerModeIOLowVoltageDetect(SPC0, false);
    SPC_EnableLowPowerModeIOHighVoltageDetect(SPC0, false);

    /* DCDC vl is mid, ds is low in low power mode */
    spc_lowpower_mode_dcdc_option_t dcdcOpt = {
        .DCDCVoltage       = kSPC_DCDC_MidVoltage,      /* same to the core ldo lvl */
        .DCDCDriveStrength = kSPC_DCDC_LowDriveStrength,        /* requires LVDs/HVDs disabled. */
    };
    SPC_SetLowPowerModeDCDCRegulatorConfig(SPC0, &dcdcOpt);
    
    /* core ldo vl to mid, ds is low in low power mode. */
    spc_lowpower_mode_core_ldo_option_t coreLdoOpt = {
        .CoreLDOVoltage = kSPC_CoreLDO_MidDriveVoltage, /* same to the dcdc lvl */
        .CoreLDODriveStrength = kSPC_CoreLDO_LowDriveStrength, /* requires LVDs/HVDs disabled. */
    };
    SPC_SetLowPowerModeCoreLDORegulatorConfig(SPC0, &coreLdoOpt);

    /* Set sys ldo ds to low in low power mode, this operation requires the LVDs/HVDs disabled. */
    SPC_SetLowPowerModeSystemLDORegulatorDriveStrength(SPC0, kSPC_SysLDO_LowDriveStrength);

    /* Dsiable core_vdd IVS in low power mode. */
    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(SPC0, false);

    /* Bandgap should be disabled in low power mode */
    SPC_SetLowPowerModeBandgapmodeConfig(SPC0, kSPC_BandgapDisabled);
}

static void SetSystemPeripheralPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint8_t peripheralId = (uint8_t)(pResourceRecode - resourceDB);

    switch ((resc_name_t)peripheralId)
    {
        case kResc_CodeCache:
            if (operateMode == PM_RESOURCE_OFF)
            {
                 SYSCON->LPCAC_CTRL |= SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;
            }
            else
            {
                 SYSCON->LPCAC_CTRL &= ~SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;
            }
            break;

        case kResc_Fro_180M:
            if (operateMode == PM_RESOURCE_OFF)
            {
                if ((SCG0->FIRCCSR & SCG_FIRCCSR_LK_MASK) == SCG_FIRCCSR_LK_MASK)
                {
                    SCG0->FIRCCSR &= ~SCG_FIRCCSR_LK_MASK;
                }

                SCG0->FIRCCSR &= ~SCG_FIRCCSR_FIRC_FCLK_PERIPH_EN_MASK;
                SCG0->FIRCCSR &= ~SCG_FIRCCSR_FIRC_SCLK_PERIPH_EN_MASK;
                SCG0->FIRCCSR |= SCG_FIRCCSR_LK_MASK;
                
                SYSCON->CLOCK_CTRL &= ~SYSCON_CLOCK_CTRL_FRO_HF_ENA_MASK;
            }
            else
            {
                if ((SCG0->FIRCCSR & SCG_FIRCCSR_LK_MASK) == SCG_FIRCCSR_LK_MASK)
                {
                    SCG0->FIRCCSR &= ~SCG_FIRCCSR_LK_MASK;
                }
                
                SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO_HF_ENA_MASK;
                SCG0->FIRCCSR |= SCG_FIRCCSR_FIRC_FCLK_PERIPH_EN_MASK;
                SCG0->FIRCCSR |= SCG_FIRCCSR_FIRC_SCLK_PERIPH_EN_MASK;
                
                while ((SCG0->FIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK) == 0U)
                {
                }
                
                SCG0->FIRCCSR |= SCG_FIRCCSR_LK_MASK;
            }
            break;
            
        case kResc_Fro_12M:
            if (operateMode == PM_RESOURCE_OFF)
            {
                if ((SCG0->SIRCCSR & SCG_SIRCCSR_LK_MASK) == SCG_SIRCCSR_LK_MASK)
                {
                    SCG0->SIRCCSR &= ~SCG_SIRCCSR_LK_MASK;
                }

                SCG0->SIRCCSR &= ~SCG_SIRCCSR_SIRC_CLK_PERIPH_EN_MASK;
                SCG0->SIRCCSR |= SCG_SIRCCSR_LK_MASK;
                
                SYSCON->CLOCK_CTRL &= ~SYSCON_CLOCK_CTRL_FRO12MHZ_ENA_MASK;
            }
            else
            {
                if ((SCG0->SIRCCSR & SCG_SIRCCSR_LK_MASK) == SCG_SIRCCSR_LK_MASK)
                {
                    SCG0->SIRCCSR &= ~SCG_SIRCCSR_LK_MASK;
                }
                
                SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO12MHZ_ENA_MASK;
                SCG0->SIRCCSR |= SCG_SIRCCSR_SIRC_CLK_PERIPH_EN_MASK;

                while ((SCG0->SIRCCSR & SCG_SIRCCSR_SIRCVLD_MASK) == 0U)
                {
                }
                
                SCG0->SIRCCSR |= SCG_SIRCCSR_LK_MASK;
            }
            break;
            
        case kResc_Fro_16K:
            if (operateMode == PM_RESOURCE_OFF)
            {
                 VBAT_EnableFRO16k(VBAT0, false);
            }
            else
            {
                 VBAT_EnableFRO16k(VBAT0, true);
            }
            break;

        case kResc_Osc_Rtc:
            if (operateMode == PM_RESOURCE_OFF)
            {
                 VBAT_EnableCrystalOsc32k(VBAT0, false);
            }
            else
            {
                 VBAT_EnableCrystalOsc32k(VBAT0, true);
            }
            break;

        case kResc_Osc_Sys:
            if (operateMode == PM_RESOURCE_OFF)
            {
                if ((SCG0->SOSCCSR & SCG_SOSCCSR_LK_MASK) == SCG_SOSCCSR_LK_MASK)
                {
                    SCG0->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;
                }

                SCG0->SOSCCSR &= ~SCG_SOSCCSR_SOSCEN_MASK;
                SCG0->SOSCCSR |= SCG_SOSCCSR_LK_MASK;
            }
            else
            {
                if ((SCG0->SOSCCSR & SCG_SOSCCSR_LK_MASK) == SCG_SOSCCSR_LK_MASK)
                {
                    SCG0->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;
                }

                SCG0->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;
                SCG0->SOSCCSR |= SCG_SOSCCSR_LK_MASK;
            }
            break; 

        case kResc_Apll:
            if (operateMode == PM_RESOURCE_OFF)
            {
                if ((SCG0->APLLCSR & SCG_APLLCSR_APLL_LOCK_MASK) == SCG_APLLCSR_APLL_LOCK_MASK)
                {
                    SCG0->APLLCSR &= ~SCG_APLLCSR_APLL_LOCK_MASK;
                }
                
                SCG0->APLLCSR &= ~(SCG_APLLCSR_APLLPWREN_MASK | SCG_APLLCSR_APLLCLKEN_MASK);
                SCG0->APLLCSR |= SCG_APLLCSR_APLL_LOCK_MASK;
            }
            else
            {
                if ((SCG0->APLLCSR & SCG_APLLCSR_APLL_LOCK_MASK) == SCG_APLLCSR_APLL_LOCK_MASK)
                {
                    SCG0->APLLCSR &= ~SCG_APLLCSR_APLL_LOCK_MASK;
                }

                SCG0->APLLCSR |= (SCG_APLLCSR_APLLPWREN_MASK | SCG_APLLCSR_APLLCLKEN_MASK);
                SCG0->APLLCSR |= SCG_APLLCSR_APLL_LOCK_MASK;
            }
            break;

        case kResc_Spll:
            if (operateMode == PM_RESOURCE_OFF)
            {
                if ((SCG0->SPLLCSR & SCG_SPLLCSR_SPLL_LOCK_MASK) == SCG_SPLLCSR_SPLL_LOCK_MASK)
                {
                    SCG0->SPLLCSR &= ~SCG_SPLLCSR_SPLL_LOCK_MASK;
                }
                
                SCG0->SPLLCSR &= ~(SCG_SPLLCSR_SPLLPWREN_MASK | SCG_SPLLCSR_SPLLCLKEN_MASK);
                SCG0->SPLLCSR &= ~SCG_SPLLCSR_SPLL_LOCK_MASK;
            }
            else
            {
                if ((SCG0->SPLLCSR & SCG_SPLLCSR_SPLL_LOCK_MASK) == SCG_SPLLCSR_SPLL_LOCK_MASK)
                {
                    SCG0->SPLLCSR &= ~SCG_SPLLCSR_SPLL_LOCK_MASK;
                }
                
                SCG0->SPLLCSR |= (SCG_SPLLCSR_SPLLPWREN_MASK | SCG_SPLLCSR_SPLLCLKEN_MASK);
                SCG0->SPLLCSR &= ~SCG_SPLLCSR_SPLL_LOCK_MASK;
            }
            break;
            
        default:
        {
            assert(false);
            break;
        }
    }

    pResourceRecode->currentMode = operateMode;
}

static void SetVoltagePeripheralPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint8_t peripheralId = (uint8_t)(pResourceRecode - resourceDB);
    
    switch ((resc_name_t)peripheralId)
    {
        case kResc_LdoCore:
            if (operateMode == PM_RESOURCE_OFF)
            {
                 SPC_EnableCoreLDORegulator(SPC0, false);
            }
            else
            {
                 SPC_EnableCoreLDORegulator(SPC0, true);
            }
            break;

        case kResc_RamRetentionLdo:
            if (operateMode == PM_RESOURCE_OFF)
            {
                if (kStatus_Success != VBAT_EnableBackupSRAMRegulator(VBAT0, false))
                {
                    assert(false);
                }
                VBAT_EnableBandgapRefreshMode(VBAT0, false);
                VBAT_EnableBandgap(VBAT0, false);
            }
            else
            {
                VBAT_EnableBandgap(VBAT0, true);
                VBAT_EnableBandgapRefreshMode(VBAT0, true);
                if (kStatus_Success != VBAT_EnableBackupSRAMRegulator(VBAT0, true))
                {
                    assert(false);
                }
            }
            break;

        case kResc_Core_Vdd_Ivs:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(SPC0, false);
            }
            else
            {
                SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(SPC0, true);
            }
            break;

        case kResc_Core_Vdd_Lvd_Act:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableActiveModeCoreLowVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableActiveModeCoreLowVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Core_Vdd_Hvd_Act:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableActiveModeCoreHighVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableActiveModeCoreHighVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Sys_Vdd_Lvd_Act:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableActiveModeSystemLowVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableActiveModeSystemLowVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Sys_Vdd_Hvd_Act:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableActiveModeSystemHighVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableActiveModeSystemHighVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Io_Vdd_Lvd_Act:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableActiveModeIOLowVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableActiveModeIOLowVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Io_Vdd_Hvd_Act:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableActiveModeIOHighVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableActiveModeIOHighVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Core_Vdd_Lvd_Lp:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableLowPowerModeCoreLowVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableLowPowerModeCoreLowVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Core_Vdd_Hvd_Lp:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableLowPowerModeCoreHighVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableLowPowerModeCoreHighVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Sys_Vdd_Lvd_Lp:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableLowPowerModeSystemLowVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableLowPowerModeSystemLowVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Sys_Vdd_Hvd_Lp:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableLowPowerModeSystemHighVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableLowPowerModeSystemHighVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Io_Vdd_Lvd_Lp:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableLowPowerModeIOLowVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableLowPowerModeIOLowVoltageDetect(SPC0, true);
            }
            break;

        case kResc_Io_Vdd_Hvd_Lp:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SPC_EnableLowPowerModeIOHighVoltageDetect(SPC0, false);
            }
            else
            {
                SPC_EnableLowPowerModeIOHighVoltageDetect(SPC0, true);
            }
            break;
            
        default:
        {
            assert(false);
            break;
        }
    }

    pResourceRecode->currentMode = operateMode;
}

static void SetSramPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint32_t sramId =0U;
    uint8_t peripheralId = (uint8_t)(pResourceRecode - resourceDB);

    if (peripheralId >= (uint8_t)kResc_RamX0)
    {
        sramId = (1U << (uint32_t)(peripheralId - kResc_RamX0));
        
        if (operateMode == PM_RESOURCE_OFF)
        {
            CMC_PowerOffSRAMAllMode(CMC0, sramId | CMC0->SRAMDIS[0]);
        }
        else if (operateMode == PM_RESOURCE_PARTABLE_ON2)
        {
            CMC_PowerOnSRAMAllMode(CMC0, sramId | CMC0->SRAMDIS[0]);
            CMC_PowerOnSRAMLowPowerOnly(CMC0, sramId | CMC0->SRAMRET[0]);
        }
        else
        {
            CMC_PowerOnSRAMAllMode(CMC0, sramId | CMC0->SRAMDIS[0]);
        }
    }
    else
    {
        sramId = (1U << (uint32_t)(peripheralId - kResc_RamA0));
        
        if (operateMode == PM_RESOURCE_OFF)
        {
            VBAT_PowerOffSRAMsInLowPowerModes(VBAT0, (uint8_t)sramId);
        }
        else if (operateMode == PM_RESOURCE_PARTABLE_ON2)
        {
            if (g_nextPowerMode == PM_LP_STATE_DEEP_POWER_DOWN)
            {
                VBAT_RetainSRAMsInLowPowerModes(VBAT0, (uint8_t)sramId);
            }
            else
            {
                VBAT_SwitchSRAMPowerBySocSupply(VBAT0); /* SRAM supply follows the chip power modes. */
            }
        }
        else
        {
            VBAT_SwitchSRAMPowerBySocSupply(VBAT0); /* SRAM supply follows the chip power modes. */
        }
    }

    pResourceRecode->currentMode = operateMode;
}

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
static status_t ManageWakeupSource(pm_wakeup_source_t *ws, bool enable)
{
    assert(ws);

    uint32_t inputType;
    uint32_t inputId;
    uint32_t irqn;
    uint32_t misc;

    PM_DECODE_WAKEUP_SOURCE_ID(ws->wsId);

    if (inputType == (uint32_t)kWup_ModInt)
    {
        WUU_SetInternalWakeUpModulesConfig(WUU0, (uint8_t)inputId, kWUU_InternalModuleInterrupt);

        if (enable)
        {
            (void)EnableIRQ((IRQn_Type)irqn);
        }
        else
        {
            (void)DisableIRQ((IRQn_Type)irqn);
        }
    }
    else
    {
        assert(false);
    }
    
    (void)misc;

    return kStatus_Success;
}
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER*/
