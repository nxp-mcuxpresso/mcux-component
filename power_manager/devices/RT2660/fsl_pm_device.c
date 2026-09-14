/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pm_core.h"
#include "fsl_pm_device.h"

#include "fsl_power.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void PM_DEV_EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void PM_DEV_CleanExitLowPowerMode(void);
static void PM_TimerStart(void);
static void PM_TimerStop(void);
static void PM_BuildSleepConfig(power_sleep_config_t *cfg, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void PM_BuildDeepSleepConfig(power_deep_sleep_config_t *cfg,
                                     pm_resc_mask_t *pSoftRescMask,
                                     pm_resc_group_t *pSysRescGroup);
static void PM_BuildPowerDownConfig(power_down_config_t *cfg, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup);
static void PM_BuildDeepPowerDownConfig(power_deep_power_down_config_t *cfg,
                                        pm_resc_mask_t *pSoftRescMask,
                                        pm_resc_group_t *pSysRescGroup);

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
static status_t PM_DEV_ManageWakeupSource(pm_wakeup_source_t *ws, bool enable);
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Per-state resource-constraint masks, built with the framework's PM_DEFINE_RESC_MASK
 * helper -- matches components/power_manager/devices/MCXW236/fsl_pm_device.c's pattern
 * (a real shipped multi-word port). See design.md "Resource-Constraint Mask Definitions"
 * for the fix/var table this is sourced from. */
#define PM_WFI_FIX_CONSTR_MASK   PM_DEFINE_RESC_MASK(kResc_Cm85Active)
#define PM_WFI_VAR_CONSTR_MASK   ((pm_resc_mask_t){.rescMask = {0}})

#define PM_SLEEP_FIX_CONSTR_MASK PM_DEFINE_RESC_MASK(kResc_Cm85Active, kResc_NpuActive)
#define PM_SLEEP_VAR_CONSTR_MASK PM_DEFINE_RESC_MASK( \
    kResc_Cm85Domain, kResc_NpuDomain, kResc_MediaDomain, kResc_CommDomain, \
    kResc_RcgCompute, kResc_RcgMain, kResc_RcgComm, kResc_RcgMedia, kResc_RcgAudio, \
    kResc_ClkLdoa0V8, kResc_ClkFro192M, kResc_ClkFro12M, kResc_ClkMainPll, kResc_ClkCorePll, \
    kResc_ClkSysPll, kResc_ClkLdoq0V8, kResc_ClkSxosc, kResc_ClkFro12MLp)

#define PM_DS_FIX_CONSTR_MASK PM_DEFINE_RESC_MASK(kResc_Cm85Active, kResc_NpuActive, kResc_MainDmaActive, kResc_WakeDmaActive)
#define PM_DS_VAR_CONSTR_MASK PM_DEFINE_RESC_MASK( \
    kResc_Cm85Domain, kResc_NpuDomain, kResc_MediaDomain, kResc_CommDomain, \
    kResc_WakeSram, kResc_ComputeSram01, kResc_Ocram0M23, kResc_Ocram0M4, kResc_Ocram0M5, \
    kResc_Ocram1M0, kResc_Ocram1M1, kResc_Ocram2M0, kResc_Ocram2M1, kResc_LlcCache, kResc_Audio8Kb, \
    kResc_M85Tcm, kResc_NpuTcm, kResc_CommTcm, kResc_MediaTcm, \
    kResc_RcgCompute, kResc_RcgMain, kResc_RcgComm, kResc_RcgMedia, kResc_RcgAudio, \
    kResc_ClkLdoa0V8, kResc_ClkFro192M, kResc_ClkFro12M, kResc_ClkMainPll, kResc_ClkCorePll, \
    kResc_ClkSysPll, kResc_ClkLdoq0V8, kResc_ClkSxosc, kResc_ClkFro12MLp)

/* Power Down: resources that are ALWAYS OFF (fix) vs ALWAYS ON (unconditional) vs
 * CONFIGURABLE (var).  All values verified against POWER_EnterPowerDown() in fsl_power.c.
 *
 * Always OFF in PD (fix -- holding ON constraint rejects PD):
 *   MainDomain, Cm85Domain, NpuDomain, MediaDomain, CommDomain
 *     -> POWER_ConfigureAllDomainSleepEvents(PowerOff × 5, 5th arg = MainDomain)
 *   13 SRAM blocks (no power_down_config_t field -- supply removed, contents lost)
 *   RcgCompute, RcgComm, RcgMedia, RcgAudio
 *     -> not in POWER_RCGCFG_STBY_POWERDOWN_REQUIRED (hardcoded off)
 *   Cm85Active / NpuActive / MainDmaActive / WakeDmaActive (busy gates)
 *
 * Always ON in PD (unconditional -- NOT in fix, NOT in var, hardware guarantees):
 *   WakeDomain: not configured by POWER_ConfigureAllDomainSleepEvents (Wake domain
 *               stays powered for wakeup detection; hardware default)
 *   RcgMain / RcgWake / RcgWake1M / RcgWake2M
 *     -> POWER_RCGCFG_STBY_POWERDOWN_REQUIRED = Wake|Wake1M|Wake2M|Main
 *   All 9 clock sources
 *     -> socCfg.csrccfgStby = POWER_CSRCCFG_STBY_ALL_ON (hardcoded, power_down_config_t
 *        has no clock-source fields -- driver keeps all ON to minimize wakeup latency)
 *   VbatSram: lives in always-on VBAT domain, separate from MEMCON
 *
 * Configurable in PD (var):
 *   WakeSram and ComputeSram01 (the only two fields in power_down_config_t). */
#define PM_PD_FIX_CONSTR_MASK PM_DEFINE_RESC_MASK( \
    kResc_MainDomain, kResc_Cm85Domain, kResc_NpuDomain, kResc_MediaDomain, kResc_CommDomain, \
    kResc_Ocram0M23, kResc_Ocram0M4, kResc_Ocram0M5, kResc_Ocram1M0, kResc_Ocram1M1, \
    kResc_Ocram2M0, kResc_Ocram2M1, kResc_LlcCache, kResc_Audio8Kb, kResc_M85Tcm, kResc_NpuTcm, \
    kResc_CommTcm, kResc_MediaTcm, \
    kResc_RcgCompute, kResc_RcgComm, kResc_RcgMedia, kResc_RcgAudio, \
    kResc_Cm85Active, kResc_NpuActive, kResc_MainDmaActive, kResc_WakeDmaActive)
#define PM_PD_VAR_CONSTR_MASK PM_DEFINE_RESC_MASK(kResc_WakeSram, kResc_ComputeSram01)

#define PM_DPD_FIX_CONSTR_MASK PM_DEFINE_RESC_MASK( \
    kResc_WakeDomain, kResc_MainDomain, kResc_Cm85Domain, kResc_NpuDomain, kResc_MediaDomain, kResc_CommDomain, \
    kResc_WakeSram, kResc_ComputeSram01, kResc_Ocram0M23, kResc_Ocram0M4, kResc_Ocram0M5, \
    kResc_Ocram1M0, kResc_Ocram1M1, kResc_Ocram2M0, kResc_Ocram2M1, kResc_LlcCache, kResc_Audio8Kb, \
    kResc_M85Tcm, kResc_NpuTcm, kResc_CommTcm, kResc_MediaTcm, \
    kResc_RcgCompute, kResc_RcgMain, kResc_RcgWake, kResc_RcgComm, kResc_RcgMedia, kResc_RcgAudio, \
    kResc_RcgWake1M, kResc_RcgWake2M, \
    kResc_ClkLdoa0V8, kResc_ClkFro192M, kResc_ClkFro12M, kResc_ClkMainPll, kResc_ClkCorePll, \
    kResc_ClkSysPll, kResc_ClkLdoq0V8, kResc_ClkSxosc, kResc_ClkFro12MLp, \
    kResc_Cm85Active, kResc_NpuActive, kResc_MainDmaActive, kResc_WakeDmaActive)
#define PM_DPD_VAR_CONSTR_MASK PM_DEFINE_RESC_MASK(kResc_VbatSram)

extern const pm_device_option_t g_devicePMOption; /* MISRA Rule 8.4: compatible declaration before definition */
const pm_device_option_t g_devicePMOption = {
    .states =
        {
            [PM_LP_STATE_WFI] =
                {
                    .exitLatency        = 0U, /* negligible: a few CPU clock cycles */
                    .fixConstraintsMask = PM_WFI_FIX_CONSTR_MASK,
                    .varConstraintsMask = PM_WFI_VAR_CONSTR_MASK,
                },
            [PM_LP_STATE_SLEEP] =
                {
                    .exitLatency        = 2U, /* measured ~1.3 us, rounded up */
                    .fixConstraintsMask = PM_SLEEP_FIX_CONSTR_MASK,
                    .varConstraintsMask = PM_SLEEP_VAR_CONSTR_MASK,
                },
            [PM_LP_STATE_DEEP_SLEEP] =
                {
                    .exitLatency        = 149U, /* measured */
                    .fixConstraintsMask = PM_DS_FIX_CONSTR_MASK,
                    .varConstraintsMask = PM_DS_VAR_CONSTR_MASK,
                },
            [PM_LP_STATE_POWER_DOWN] =
                {
                    .exitLatency        = 381U, /* measured */
                    .fixConstraintsMask = PM_PD_FIX_CONSTR_MASK,
                    .varConstraintsMask = PM_PD_VAR_CONSTR_MASK,
                },
            [PM_LP_STATE_DEEP_POWER_DOWN] =
                {
                    .exitLatency        = 18700U, /* measured, 18.7 ms */
                    .fixConstraintsMask = PM_DPD_FIX_CONSTR_MASK,
                    .varConstraintsMask = PM_DPD_VAR_CONSTR_MASK,
                },
        },
    .stateCount = PM_LP_STATE_COUNT,
    .enter      = PM_DEV_EnterLowPowerMode,
    .clean      = PM_DEV_CleanExitLowPowerMode,

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
    .manageWakeupSource = PM_DEV_ManageWakeupSource,
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Domain-event field helper: held FULL_ON -> Active; unconstrained -> PowerOff
 *        (deepest/most power-saving default -- matches REQ-002/REQ-003's "unconstrained
 *        means the policy is free to power the resource off" contract).
 */
static pdcon_event_t PM_ResolveDomainEvent(uint32_t resc, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    pdcon_event_t event = kPDCON_EventPowerOff;

    if (PM_IS_CONS_SET(pSoftRescMask, resc) && (PM_FIND_CONS_OPERATE_MODE(resc, pSysRescGroup) == PM_RESOURCE_FULL_ON))
    {
        event = kPDCON_EventNoneOrActive;
    }
    return event;
}

/*!
 * @brief Memory-block field helper: held FULL_ON -> Active; held PARTABLE_ON1 -> Retention;
 *        unconstrained -> PowerDown (deepest/most power-saving default, data lost -- matches
 *        REQ-003's "no memory constraint set entering Power Down defaults to PowerDown").
 */
static memcon_power_mode_t PM_ResolveMemoryMode(uint32_t resc, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    memcon_power_mode_t mode = kMEMCON_PowerModePowerDown;

    if (PM_IS_CONS_SET(pSoftRescMask, resc))
    {
        uint8_t opMode = PM_FIND_CONS_OPERATE_MODE(resc, pSysRescGroup);
        if (opMode == PM_RESOURCE_FULL_ON)
        {
            mode = kMEMCON_PowerModeActive;
        }
        else if (opMode == PM_RESOURCE_PARTABLE_ON1)
        {
            mode = kMEMCON_PowerModeRetention;
        }
        else
        {
            /* MISRA C-2012 15.7 */
        }
    }
    return mode;
}

/*!
 * @brief Clock-source / root-clock bool field helper: held FULL_ON forces the source on
 *        beyond its per-resource, per-state baseline; otherwise the baseline applies
 *        unchanged. Unlike domains/memory, absence of a constraint here does NOT mean
 *        "off" -- the baseline (power_mode_switch_rt2660's g_powerModeTable -- the
 *        [0]Sleep column for PM_BuildSleepConfig(), the [1]DS1 column for
 *        PM_BuildDeepSleepConfig()) already encodes which clock sources/root clocks are
 *        required to keep running in that state, and the framework has no mechanism to
 *        force one of those off via a constraint.
 */
static bool PM_ResolveClockEnable(uint32_t resc, bool baseline, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    bool enabled = baseline;

    if (PM_IS_CONS_SET(pSoftRescMask, resc) && (PM_FIND_CONS_OPERATE_MODE(resc, pSysRescGroup) == PM_RESOURCE_FULL_ON))
    {
        enabled = true;
    }
    return enabled;
}

static void PM_BuildSleepConfig(power_sleep_config_t *cfg, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    (void)memset(cfg, 0, sizeof(*cfg));

    /* CM85 / NPU domain events: kResc_Cm85Domain / kResc_NpuDomain variable in Sleep.
     * Held FULL_ON → domain event Active (clock gate); unconstrained → PowerOff (power gate). */
    cfg->setCpuDomainEvent   = PM_ResolveDomainEvent((uint32_t)kResc_Cm85Domain, pSoftRescMask, pSysRescGroup);
    cfg->setNpuDomainEvent   = PM_ResolveDomainEvent((uint32_t)kResc_NpuDomain, pSoftRescMask, pSysRescGroup);
    cfg->setCommDomainEvent  = PM_ResolveDomainEvent((uint32_t)kResc_CommDomain, pSoftRescMask, pSysRescGroup);
    cfg->setMediaDomainEvent = PM_ResolveDomainEvent((uint32_t)kResc_MediaDomain, pSoftRescMask, pSysRescGroup);

    /* PMU analog + clock sources + root clocks: fixed [0]Sleep column, not resource-driven
     * (power_mode_switch_rt2660's g_powerModeTable, power_mode_switch.h:1111-1133/1163-1170). */
    cfg->setPmuMode        = 0U; /* HP (High Performance) -- fsl_power.h: PmuMode bits[22:20], 0=HP */
    cfg->setDcdcMode       = kPMU_DcdcModePWM;
    cfg->setCoreLevel      = 0U;
    cfg->setLdo0V8Mode     = kPMU_LdoModeHP;
    cfg->setLdo1V8Mode     = kPMU_LdoModeHP;
    cfg->setLdoVdda1V8Mode = (uint8_t)kPMU_LdoModeHP;
    cfg->enableHqref       = true;
    cfg->enableSensors     = 0U;

    /* Clock sources: variable, Sleep-column baseline is ON for every one of the 9 -- held
     * constraint has no effect beyond confirming the baseline (matches REQ-002's Sleep table). */
    cfg->enableLdoa0V8ClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkLdoa0V8, true, pSoftRescMask, pSysRescGroup);
    cfg->enableFro192MClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkFro192M, true, pSoftRescMask, pSysRescGroup);
    cfg->enableFro12MClockSource   = PM_ResolveClockEnable((uint32_t)kResc_ClkFro12M, true, pSoftRescMask, pSysRescGroup);
    cfg->enableMainPllClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkMainPll, true, pSoftRescMask, pSysRescGroup);
    cfg->enableCorePllClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkCorePll, true, pSoftRescMask, pSysRescGroup);
    cfg->enableSysPllClockSource   = PM_ResolveClockEnable((uint32_t)kResc_ClkSysPll, true, pSoftRescMask, pSysRescGroup);
    cfg->enableLdoq0V8ClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkLdoq0V8, true, pSoftRescMask, pSysRescGroup);
    cfg->enableSxoscClockSource    = PM_ResolveClockEnable((uint32_t)kResc_ClkSxosc, true, pSoftRescMask, pSysRescGroup);
    cfg->enableFro12MLpClockSource = PM_ResolveClockEnable((uint32_t)kResc_ClkFro12MLp, true, pSoftRescMask, pSysRescGroup);

    /* Root clocks: 5 are variable (Sleep-column baseline ON); WAKE/WAKE_1M/WAKE_2M are
     * unconditionally required for wakeup-detection logic, not resource-driven. */
    cfg->enableComputeRootClock = PM_ResolveClockEnable((uint32_t)kResc_RcgCompute, true, pSoftRescMask, pSysRescGroup);
    cfg->enableMainRootClock    = PM_ResolveClockEnable((uint32_t)kResc_RcgMain, true, pSoftRescMask, pSysRescGroup);
    cfg->enableWakeRootClock    = true;
    cfg->enableCommRootClock    = PM_ResolveClockEnable((uint32_t)kResc_RcgComm, true, pSoftRescMask, pSysRescGroup);
    cfg->enableMediaRootClock   = PM_ResolveClockEnable((uint32_t)kResc_RcgMedia, true, pSoftRescMask, pSysRescGroup);
    cfg->enableAudioRootClock   = PM_ResolveClockEnable((uint32_t)kResc_RcgAudio, true, pSoftRescMask, pSysRescGroup);
    cfg->enableWake1MRootClock  = true;
    cfg->enableWake2MRootClock  = true;
}

static void PM_BuildDeepSleepConfig(power_deep_sleep_config_t *cfg,
                                     pm_resc_mask_t *pSoftRescMask,
                                     pm_resc_group_t *pSysRescGroup)
{
    (void)memset(cfg, 0, sizeof(*cfg));

    /* CM85 / NPU domain events: variable in Deep Sleep.
     * kResc_Cm85Domain / kResc_NpuDomain FULL_ON → keep domain powered (clock gate).
     * Unconstrained → domain off (power gate). */
    cfg->setCpuDomainEvent   = PM_ResolveDomainEvent((uint32_t)kResc_Cm85Domain, pSoftRescMask, pSysRescGroup);
    cfg->setNpuDomainEvent   = PM_ResolveDomainEvent((uint32_t)kResc_NpuDomain, pSoftRescMask, pSysRescGroup);
    cfg->setCommDomainEvent  = PM_ResolveDomainEvent((uint32_t)kResc_CommDomain, pSoftRescMask, pSysRescGroup);
    cfg->setMediaDomainEvent = PM_ResolveDomainEvent((uint32_t)kResc_MediaDomain, pSoftRescMask, pSysRescGroup);

    /* PMU analog: fixed [1]DS1 column, not resource-driven. */
    cfg->setPmuMode        = 2U; /* LP (Low Power) -- fsl_power.h: PmuMode bits[22:20], 2=LP */
    cfg->setDcdcMode       = kPMU_DcdcModePWM;
    cfg->setCoreLevel      = 0x11U;
    cfg->setLdo0V8Mode     = kPMU_LdoModeHP;
    cfg->setLdo1V8Mode     = kPMU_LdoModeHP;
    cfg->setLdoVdda1V8Mode = (uint8_t)kPMU_LdoModeHP;
    cfg->enableHqref       = true;
    cfg->enableSensors     = 0U;

    /* Clock sources: variable, DS1-baseline defaults per resource, held constraint can only
     * force ON (never OFF -- the baseline already reflects what Deep Sleep requires). */
    cfg->enableLdoa0V8ClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkLdoa0V8, true, pSoftRescMask, pSysRescGroup);
    cfg->enableFro192MClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkFro192M, true, pSoftRescMask, pSysRescGroup);
    cfg->enableFro12MClockSource   = PM_ResolveClockEnable((uint32_t)kResc_ClkFro12M, true, pSoftRescMask, pSysRescGroup);
    cfg->enableMainPllClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkMainPll, false, pSoftRescMask, pSysRescGroup);
    cfg->enableCorePllClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkCorePll, false, pSoftRescMask, pSysRescGroup);
    cfg->enableSysPllClockSource   = PM_ResolveClockEnable((uint32_t)kResc_ClkSysPll, false, pSoftRescMask, pSysRescGroup);
    cfg->enableLdoq0V8ClockSource  = PM_ResolveClockEnable((uint32_t)kResc_ClkLdoq0V8, false, pSoftRescMask, pSysRescGroup);
    cfg->enableSxoscClockSource    = PM_ResolveClockEnable((uint32_t)kResc_ClkSxosc, false, pSoftRescMask, pSysRescGroup);
    cfg->enableFro12MLpClockSource = PM_ResolveClockEnable((uint32_t)kResc_ClkFro12MLp, true, pSoftRescMask, pSysRescGroup);

    /* Root clocks: 5 are variable (DS1 baseline OFF); WAKE/WAKE_1M/WAKE_2M are
     * unconditionally required for wakeup-detection logic, not resource-driven. */
    cfg->enableComputeRootClock = PM_ResolveClockEnable((uint32_t)kResc_RcgCompute, false, pSoftRescMask, pSysRescGroup);
    cfg->enableMainRootClock    = PM_ResolveClockEnable((uint32_t)kResc_RcgMain, false, pSoftRescMask, pSysRescGroup);
    cfg->enableWakeRootClock    = true;
    cfg->enableCommRootClock    = PM_ResolveClockEnable((uint32_t)kResc_RcgComm, false, pSoftRescMask, pSysRescGroup);
    cfg->enableMediaRootClock   = PM_ResolveClockEnable((uint32_t)kResc_RcgMedia, false, pSoftRescMask, pSysRescGroup);
    cfg->enableAudioRootClock   = PM_ResolveClockEnable((uint32_t)kResc_RcgAudio, false, pSoftRescMask, pSysRescGroup);
    cfg->enableWake1MRootClock  = true;
    cfg->enableWake2MRootClock  = true;

    /* Memory: variable, unconstrained defaults to PowerDown (deepest). */
    cfg->setWakeSramMode      = PM_ResolveMemoryMode((uint32_t)kResc_WakeSram, pSoftRescMask, pSysRescGroup);
    cfg->setComputeSram01Mode = PM_ResolveMemoryMode((uint32_t)kResc_ComputeSram01, pSoftRescMask, pSysRescGroup);
    cfg->setOcram0M23Mode     = PM_ResolveMemoryMode((uint32_t)kResc_Ocram0M23, pSoftRescMask, pSysRescGroup);
    cfg->setOcram0M4Mode      = PM_ResolveMemoryMode((uint32_t)kResc_Ocram0M4, pSoftRescMask, pSysRescGroup);
    cfg->setOcram0M5Mode      = PM_ResolveMemoryMode((uint32_t)kResc_Ocram0M5, pSoftRescMask, pSysRescGroup);
    cfg->setOcram1M0Mode      = PM_ResolveMemoryMode((uint32_t)kResc_Ocram1M0, pSoftRescMask, pSysRescGroup);
    cfg->setOcram1M1Mode      = PM_ResolveMemoryMode((uint32_t)kResc_Ocram1M1, pSoftRescMask, pSysRescGroup);
    cfg->setOcram2M0Mode      = PM_ResolveMemoryMode((uint32_t)kResc_Ocram2M0, pSoftRescMask, pSysRescGroup);
    cfg->setOcram2M1Mode      = PM_ResolveMemoryMode((uint32_t)kResc_Ocram2M1, pSoftRescMask, pSysRescGroup);
    cfg->setLlcCacheMode      = PM_ResolveMemoryMode((uint32_t)kResc_LlcCache, pSoftRescMask, pSysRescGroup);
    cfg->setAudio8KbMode      = PM_ResolveMemoryMode((uint32_t)kResc_Audio8Kb, pSoftRescMask, pSysRescGroup);
    cfg->setM85TcmMode        = PM_ResolveMemoryMode((uint32_t)kResc_M85Tcm, pSoftRescMask, pSysRescGroup);
    cfg->setNpuTcmMode        = PM_ResolveMemoryMode((uint32_t)kResc_NpuTcm, pSoftRescMask, pSysRescGroup);
    cfg->setCommTcmMode       = PM_ResolveMemoryMode((uint32_t)kResc_CommTcm, pSoftRescMask, pSysRescGroup);
    cfg->setMediaTcmMode      = PM_ResolveMemoryMode((uint32_t)kResc_MediaTcm, pSoftRescMask, pSysRescGroup);
}

static void PM_BuildPowerDownConfig(power_down_config_t *cfg, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    (void)memset(cfg, 0, sizeof(*cfg));

    cfg->setWakeSramMode      = PM_ResolveMemoryMode((uint32_t)kResc_WakeSram, pSoftRescMask, pSysRescGroup);
    cfg->setComputeSram01Mode = PM_ResolveMemoryMode((uint32_t)kResc_ComputeSram01, pSoftRescMask, pSysRescGroup);

    /* Retention is the only preserved-content option in Power Down (per driver contract,
     * Active is semantically null here); a FULL_ON request degrades to Retention. */
    if (cfg->setWakeSramMode == kMEMCON_PowerModeActive)
    {
        cfg->setWakeSramMode = kMEMCON_PowerModeRetention;
    }
    if (cfg->setComputeSram01Mode == kMEMCON_PowerModeActive)
    {
        cfg->setComputeSram01Mode = kMEMCON_PowerModeRetention;
    }
}

static void PM_BuildDeepPowerDownConfig(power_deep_power_down_config_t *cfg,
                                        pm_resc_mask_t *pSoftRescMask,
                                        pm_resc_group_t *pSysRescGroup)
{
    (void)memset(cfg, 0, sizeof(*cfg));

    cfg->retainVbatSram = PM_IS_CONS_SET(pSoftRescMask, (uint32_t)kResc_VbatSram) &&
                          (PM_FIND_CONS_OPERATE_MODE((uint32_t)kResc_VbatSram, pSysRescGroup) == PM_RESOURCE_FULL_ON);
}

/*!
 * @brief Start the LP-timer-controller's duration measurement, if enabled. Wraps the
 *        FSL_PM_SUPPORT_LP_TIMER_CONTROLLER guard once instead of at each of the 4
 *        non-WFI states' entry points.
 */
static void PM_TimerStart(void)
{
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
    PM_RecordAndStartTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
}

/*!
 * @brief Stop the LP-timer-controller's duration measurement, if enabled. See PM_TimerStart().
 */
static void PM_TimerStop(void)
{
#if (defined(FSL_PM_SUPPORT_LP_TIMER_CONTROLLER) && FSL_PM_SUPPORT_LP_TIMER_CONTROLLER)
    PM_StopAndRecordTimer();
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */
}

static void PM_DEV_EnterLowPowerMode(uint8_t stateIndex, pm_resc_mask_t *pSoftRescMask, pm_resc_group_t *pSysRescGroup)
{
    switch (stateIndex)
    {
        case PM_LP_STATE_WFI:
        {
            /* Plain WFI: no fsl_power call -- any unmasked interrupt wakes the core.
             * Still arm the LP-timer-controller's wakeup source (REQ-004: VBAT LPTMR is
             * valid through every state) -- without it, WFI has no guaranteed wakeup
             * path when the application has no other interrupt already enabled. */
            PM_TimerStart();
            __DSB();
            __WFI();
            PM_TimerStop();
            break;
        }
        case PM_LP_STATE_SLEEP:
        {
            power_sleep_config_t cfg;
            PM_BuildSleepConfig(&cfg, pSoftRescMask, pSysRescGroup);
            PM_TimerStart();
            (void)POWER_EnterSleep(&cfg);
            PM_TimerStop();
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            power_deep_sleep_config_t cfg;
            PM_BuildDeepSleepConfig(&cfg, pSoftRescMask, pSysRescGroup);
            PM_TimerStart();
            (void)POWER_EnterDeepSleep(&cfg);
            PM_TimerStop();
            break;
        }
        case PM_LP_STATE_POWER_DOWN:
        {
            power_down_config_t cfg;
            PM_BuildPowerDownConfig(&cfg, pSoftRescMask, pSysRescGroup);
            PM_TimerStart();
            /* Does not return on success -- wakeup is a PoR reboot. A denied CSSI handshake
             * (kStatus_Fail, only possible with the bypass disabled) is silently ignored,
             * consistent with every other existing power_manager device port. */
            (void)POWER_EnterPowerDown(&cfg);
            PM_TimerStop();
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            power_deep_power_down_config_t cfg;
            PM_BuildDeepPowerDownConfig(&cfg, pSoftRescMask, pSysRescGroup);
            PM_TimerStart();
            (void)POWER_EnterDeepPowerDown(&cfg);
            PM_TimerStop();
            break;
        }
        default:
        {
            /* This branch will never be hit -- stateIndex is always one of the 5 defined states. */
            break;
        }
    }
}

static void PM_DEV_CleanExitLowPowerMode(void)
{
    /* No cleanup needed: fsl_power restores PLL DIV4 outputs and CMC state internally
     * on wakeup already. */
}

#if (defined(FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER) && FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER)
/*!
 * @brief Generic wakeup-source management: identity passthrough to POWER_Enable/
 *        DisableWakeupSource(), plus the matching NVIC IRQ for IRQ-type sources.
 *
 * fsl_power's power_wakeup_source_t already fully encodes path (IRQ/MAIN-DMA/WAKE-DMA)
 * and index in one uint32_t -- no PM_ENCODE/DECODE_WAKEUP_SOURCE_ID remap is needed.
 * IRQ-type sources (wsId < kPOWER_WakeupMainDma_Adc0) are already a plain
 * IRQn_Type value (e.g. kPOWER_WakeupIrq_MainTrdc == MAIN_TRDC_IRQn), so the matching
 * NVIC interrupt is also toggled: POWER_EnableWakeupSource() only arms the CMC0
 * IRQ_WAKEUP_MASK bit that lets the hardware wake the core, it does not touch NVIC,
 * so a peripheral's ISR would not run after wake without this (matches MIMXRT685S's
 * EnableDeepSleepIRQ and KW47B42ZB7's EnableIRQ() in their own manageWakeupSource).
 * MAIN-DMA/WAKE-DMA tagged sources (0x1x/0x2x prefix) have no IRQn_Type and skip the
 * NVIC step.
 *
 * NVIC_ClearPendingIRQ() before EnableIRQ() (matches RW612's own manageWakeupSource):
 * DisableIRQ() only clears the NVIC enable bit, not the pending bit. Without clearing
 * pending first, a source whose ISR fired on the previous low-power entry re-fires the
 * instant it is re-enabled here -- before the caller's POWER_Enter*() (or, for WFI, the
 * __WFI() below) even runs -- silently disarming the wakeup source it was meant to arm.
 */
static status_t PM_DEV_ManageWakeupSource(pm_wakeup_source_t *ws, bool enable)
{
    assert(ws != NULL);

    if (enable)
    {
        POWER_EnableWakeupSource((power_wakeup_source_t)ws->wsId);
        if (ws->wsId < kPOWER_WakeupMainDma_Adc0)
        {
            NVIC_ClearPendingIRQ((IRQn_Type)ws->wsId);
            (void)EnableIRQ((IRQn_Type)ws->wsId);
        }
    }
    else
    {
        POWER_DisableWakeupSource((power_wakeup_source_t)ws->wsId);
        if (ws->wsId < kPOWER_WakeupMainDma_Adc0)
        {
            (void)DisableIRQ((IRQn_Type)ws->wsId);
        }
    }
    return kStatus_Success;
}
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */
