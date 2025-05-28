/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pm_device.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device_config.h"
#include "fsl_debug_console.h"
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

static void EnterLowPowerPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void CleanExitLowPowerMode(void);
void EnableResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void SetSysRescPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode);
static void SetMemoryPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode);

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
static status_t ManageWakeupSource(pm_wakeup_source_t *ws, bool enable);
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*******************************************************************************
 * Variables
 ******************************************************************************/
const pm_device_option_t g_devicePMOption = {
    .states =
        {
            /* Sleep */
            {
                .exitLatency = 0U, /* TODO: Need to fill in the wake up time when the value is available in the datasheet. */
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_RESC_MASK_SLEEP0),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_RESC_MASK_SLEEP0,
                    },
            },
            /* Deep Sleep */
            {
                .exitLatency = 0U, /* TODO: Need to fill in the wake up time when the value is available in the datasheet. */
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_RESC_MASK_DEEP_SLEEP0),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_RESC_MASK_DEEP_SLEEP0,
                    },
            },
            /* Power Down */
            {
                .exitLatency = 0U, /* TODO: Need to fill in the wake up time when the value is available in the datasheet. */
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_RESC_MASK_POWER_DOWN0),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_RESC_MASK_POWER_DOWN0,
                    },
            },
            /* Deep Power Down */
            {
                .exitLatency = 0U, /* TODO: Need to fill in the wake up time when the value is available in the datasheet. */
                .fixConstraintsMask =
                    {
                        .rescMask[0U] = ~(PM_RESC_MASK_DEEP_POWER_DOWN0),
                    },
                .varConstraintsMask =
                    {
                        .rescMask[0U] = PM_RESC_MASK_DEEP_POWER_DOWN0,
                    },
            },
        },
    .stateCount = PM_LP_STATE_COUNT,
    .enter      = EnterLowPowerPowerMode,
    .clean      = CleanExitLowPowerMode,

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
    .manageWakeupSource = ManageWakeupSource,
    .isWakeupSource     = NULL,
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
};

struct _resource_recode
{
    uint8_t currentMode;
    void (*resourceConfigFunc)(uint8_t operateMode, resource_recode_t *pResourceRecode);
};

AT_ALWAYS_ON_DATA_INIT(cmc_power_domain_config_t powerConfig) = {
    .clock_mode = kCMC_GateNoneClock,
    .main_domain = kCMC_ActiveOrSleepMode,
};

/* Struct below stored in Always-On memory, to retain in all power modes*/
AT_ALWAYS_ON_DATA_INIT(resource_recode_t resourceDB[PM_CONSTRAINT_COUNT]) = {
    [kResc_Fro_16k]             = {PM_RESOURCE_FULL_ON, SetSysRescPowerMode},
    [kResc_RamRetentionLdo]     = {PM_RESOURCE_FULL_ON, SetSysRescPowerMode},
    [kResc_Core_Vdd_LVd_Act]    = {PM_RESOURCE_FULL_ON, SetSysRescPowerMode},
    [kResc_Core_Vdd_LVd_Lp]     = {PM_RESOURCE_FULL_ON, SetSysRescPowerMode},
    [kResc_Sys_Vdd_LVd]         = {PM_RESOURCE_FULL_ON, SetSysRescPowerMode},
    [kResc_Sys_Vdd_Hvd]         = {PM_RESOURCE_FULL_ON, SetSysRescPowerMode},
    [kResc_Lpcac]               = {PM_RESOURCE_FULL_ON, SetMemoryPowerMode},
    [kResc_FlashArray]          = {PM_RESOURCE_FULL_ON, SetMemoryPowerMode},
    [kResc_SramBlock0]          = {PM_RESOURCE_OFF, SetMemoryPowerMode},
    [kResc_SramBlock1]          = {PM_RESOURCE_OFF, SetMemoryPowerMode},
    [kResc_SramBlock2]          = {PM_RESOURCE_OFF, SetMemoryPowerMode},
    [kResc_SramBlock3]          = {PM_RESOURCE_OFF, SetMemoryPowerMode},
};

/*******************************************************************************
 * Code
 ******************************************************************************/
void ControlResources(pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
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

static void EnterLowPowerPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    assert(pSoftRescMask);
    assert(pSysRescGroup);

    switch (stateIndex)
    {
        case PM_LP_STATE_SLEEP:
        {
            powerConfig.clock_mode  = kCMC_GateCoreClock;
            powerConfig.main_domain = kCMC_ActiveOrSleepMode;
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            powerConfig.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
            powerConfig.main_domain = kCMC_DeepSleepMode;
            break;
        }
        case PM_LP_STATE_POWER_DOWN:
        {
            powerConfig.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
            powerConfig.main_domain = kCMC_PowerDownMode;
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            powerConfig.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
            powerConfig.main_domain = kCMC_DeepPowerDown;
            break;
        }
        default:
        {
            assert(0);
            break;
        }
    }

    ControlResources(pSoftRescMask, pSysRescGroup);
    CMC_SetPowerModeProtection(CMC, (uint32_t)kCMC_AllowAllLowPowerModes);
    CMC_EnterLowPowerMode(CMC, &powerConfig);
}

static void CleanExitLowPowerMode(void)
{
    if ((SPC_GetRequestedLowPowerMode(SPC0) & (kSPC_PowerDownWithSysClockOff | kSPC_DeepPowerDownWithSysClockOff)) != 0UL)
    {
        SPC_ClearPeriphIOIsolationFlag(SPC0);
    }

    SPC_ClearPowerDomainLowPowerRequestFlag(SPC0, kSPC_PowerDomain0);
    SPC_ClearLowPowerRequest(SPC0);
}

static void SetSysRescPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint8_t peripheralId = (uint8_t)(pResourceRecode - resourceDB);

    switch ((resc_name_t)peripheralId)
    {
        case kResc_Fro_16k:
            if (operateMode == PM_RESOURCE_OFF)
            {
                  VBAT_EnableFRO16k(VBAT0, false);
            }
            else
            {
                  VBAT_EnableFRO16k(VBAT0, true);
            }
            break;  
            
        case kResc_RamRetentionLdo:
            if (operateMode == PM_RESOURCE_OFF)
            {
                 SPC_EnableSRAMLdo(SPC0, false);
            }
            else
            {
                 SPC_EnableSRAMLdo(SPC0, true);
            }
            break; 
            
        case kResc_Core_Vdd_LVd_Act:
            if (operateMode == PM_RESOURCE_OFF)
            {
                 SPC0->ACTIVE_CFG &= ~SPC_ACTIVE_CFG_CORE_LVDE_MASK;
            }
            else
            {
                 SPC0->ACTIVE_CFG |= ~SPC_ACTIVE_CFG_CORE_LVDE_MASK;
            }
            break; 

        case kResc_Core_Vdd_LVd_Lp:
            if (operateMode == PM_RESOURCE_OFF)
            {
                 SPC0->LP_CFG &= ~SPC_LP_CFG_CORE_LVDE_MASK;
            }
            else
            {
                 SPC0->LP_CFG |= ~SPC_LP_CFG_CORE_LVDE_MASK;
            }
            break;
            
        case kResc_Sys_Vdd_LVd:
            if (operateMode == PM_RESOURCE_OFF)
            {
                 SPC0->LP_CFG &= ~SPC_LP_CFG_SYS_LVDE_MASK;
            }
            else
            {
                 SPC0->LP_CFG |= ~SPC_LP_CFG_SYS_LVDE_MASK;
            }
            break; 

        case kResc_Sys_Vdd_Hvd:
            if (operateMode == PM_RESOURCE_OFF)
            {
                 SPC0->LP_CFG &= ~SPC_LP_CFG_SYS_HVDE_MASK;
            }
            else
            {
                 SPC0->LP_CFG |= ~SPC_LP_CFG_SYS_HVDE_MASK;
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

static void SetMemoryPowerMode(uint8_t operateMode, resource_recode_t *pResourceRecode)
{
    assert(pResourceRecode);

    uint32_t sramMask =0U;
    uint8_t peripheralId = (uint8_t)(pResourceRecode - resourceDB);
    
    if (peripheralId >= (uint8_t)kResc_SramBlock0)
    {
        sramMask = (1U << (uint32_t)(peripheralId - kResc_SramBlock0));
    }

    switch ((resc_name_t)peripheralId)
    {
        case kResc_Lpcac:
            if (operateMode == PM_RESOURCE_OFF)
            {
                SYSCON->LPCAC_CTRL |= SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;
            }
            else
            {
                SYSCON->LPCAC_CTRL &= ~SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;
            }
            break;  
            
        case kResc_FlashArray:
            if (operateMode == PM_RESOURCE_OFF)
            {
                /* OFF status, Flash memory is disabled when core is sleeping, access will not
                 * cause memory exit low-power state.
                 */
                CMC_ConfigFlashMode(CMC, false, true, false);
            }
            else if (operateMode == PM_RESOURCE_PARTABLE_ON2)
            {
                /* Static status, Flash memory is disabled when core is sleeping, access will
                 * cause memory exit low-power state.
                 */
                CMC_ConfigFlashMode(CMC, true, true, false);
            }
            else
            {
                /* On status, disable=false, doze=false, wake=false. */
                CMC_ConfigFlashMode(CMC, false, false, false);
            }
            break;  
            
        case kResc_SramBlock0:
            if (operateMode == PM_RESOURCE_OFF)
            {
                /* This operation will unretain SRAM block X0/X1, RAMB0/B1. */
                SPC_UnRetainSRAMArray(SPC0, (1U << SPC_SRAMRETLDO_CNTRL_SRAM_RET_EN(sramMask)));
            }
            else
            {
                /* This operation will retain SRAM block X0/X1, RAMB0/B1. */
                SPC_RetainSRAMArray(SPC0, (1U << SPC_SRAMRETLDO_CNTRL_SRAM_RET_EN(sramMask)));
            }
            break;  
            
        case kResc_SramBlock1:
            if (operateMode == PM_RESOURCE_OFF)
            {
                /* This operation will unretain SRAM block A0. */
                SPC_UnRetainSRAMArray(SPC0, (1U << SPC_SRAMRETLDO_CNTRL_SRAM_RET_EN(sramMask)));
            }
            else
            {
                /* This operation will retain SRAM block A0. */
                SPC_RetainSRAMArray(SPC0, (1U << SPC_SRAMRETLDO_CNTRL_SRAM_RET_EN(sramMask)));
            }
            break;  
            
        case kResc_SramBlock2:
            if (operateMode == PM_RESOURCE_OFF)
            {
                /* This operation will unretain SRAM block A1-A4. */
                SPC_UnRetainSRAMArray(SPC0, (1U << SPC_SRAMRETLDO_CNTRL_SRAM_RET_EN(sramMask)));
            }
            else
            {
                 /* This operation will retain SRAM block A1-A4. */
                 SPC_RetainSRAMArray(SPC0, (1U << SPC_SRAMRETLDO_CNTRL_SRAM_RET_EN(sramMask)));
            }
            break;  

        case kResc_SramBlock3:
            if (operateMode == PM_RESOURCE_OFF)
            {
                /* This operation will unretain SRAM block C0-C3. */
                SPC_UnRetainSRAMArray(SPC0, (1U << SPC_SRAMRETLDO_CNTRL_SRAM_RET_EN(sramMask)));
            }
            else
            {
                /* This operation will retain SRAM block C0-C3. */
                SPC_RetainSRAMArray(SPC0, (1U << SPC_SRAMRETLDO_CNTRL_SRAM_RET_EN(sramMask)));
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
