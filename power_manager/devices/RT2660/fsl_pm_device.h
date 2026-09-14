/*
 * Copyright 2026 NXP
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

/* Power Mode Index. States are ordered shallowest-to-deepest (decreasing power consumption),
 * matching the pm_device_option_t.states[] array contract. */
#define PM_LP_STATE_WFI             (0U) /*!< Plain CPU __WFI(): no fsl_power call, no CMC state machine. */
#define PM_LP_STATE_SLEEP           (1U) /*!< POWER_EnterSleep(). */
#define PM_LP_STATE_DEEP_SLEEP      (2U) /*!< POWER_EnterDeepSleep(). */
#define PM_LP_STATE_POWER_DOWN      (3U) /*!< POWER_EnterPowerDown(). */
#define PM_LP_STATE_DEEP_POWER_DOWN (4U) /*!< POWER_EnterDeepPowerDown(). */
#define PM_LP_STATE_NO_CONSTRAINT   (0xFFU)

/*! @} */

/*!
 * @name System basic resource constraints definitions.
 * @{
 */

/*!
 * @brief The enumeration of power manager resource names.
 *
 * Sized to what the policy algorithm (PM_findDeepestState) needs to correctly accept/reject
 * states -- not a mirror of every fsl_power config-struct field. Covers:
 *   - the 6 power_domain_t values (including WAKE/MAIN, so an application can hold e.g.
 *     "WAKE domain must stay on" and have the policy correctly exclude Deep Power Down);
 *   - the 15 memory blocks that have a set*Mode field in power_deep_sleep_config_t;
 *   - the 8 root clocks (power_root_clock_t) and 9 clock sources (power_clock_source_t)
 *     that have an enable field in power_sleep_config_t/power_deep_sleep_config_t;
 *   - 4 pure policy-level busy/idle gates with no fsl_power register binding
 *     (kResc_Cm85Active/NpuActive/MainDmaActive/WakeDmaActive);
 *   - kResc_VbatSram, the one configurable resource in Deep Power Down
 *     (power_deep_power_down_config_t.retainVbatSram).
 * The 8 PMU analog/voltage fields (PmuMode, DcdcMode, CoreLvl, Ldo0V8Mode, Ldo1V8Mode,
 * LdoVdda1V8Mode, Hqref, Sensors) are intentionally NOT modeled here -- they are global
 * voltage/analog settings, not individually app-requestable on/off/retain resources. They
 * get fixed per-state literal defaults hardcoded in fsl_pm_device.c instead.
 */
enum _pm_resc_name
{
    /* Power domains (power_domain_t)
     * PD_CPU and PD_NPU use processor-centric naming (Cm85/Npu) to align with the
     * Active/ClockGate/PowerGate hierarchy exposed in the constraint macros below. */
    kResc_WakeDomain = 0U,  /*!< PD_WAKE. Off only in Deep Power Down. */
    kResc_MainDomain,       /*!< PD_MAIN. Off from Power Down onward. */
    kResc_Cm85Domain,       /*!< PD_CPU domain powered: CM85 domain stays on, core may be
                                 clock-gated by CMC. Configurable in Sleep/Deep Sleep; forced
                                 off from Power Down onward. Corresponds to CM85 in Clock-Gate
                                 state (Active is tracked separately via kResc_Cm85Active). */
    kResc_NpuDomain,        /*!< PD_NPU domain powered: NPU domain stays on, NPU may be
                                 clock-gated. Same semantics as kResc_Cm85Domain for the NPU. */
    kResc_MediaDomain,      /*!< PD_MEDIA. Configurable in Sleep/Deep Sleep; forced off from Power Down. */
    kResc_CommDomain,       /*!< PD_COMM. Configurable in Sleep/Deep Sleep; forced off from Power Down. */

    /* Memory blocks (power_memory_t, at power_deep_sleep_config_t field granularity) */
    kResc_WakeSram,         /*!< WAKE_SS SRAM 8KB. The only block besides ComputeSram01 with a
                                 field in power_down_config_t. */
    kResc_ComputeSram01,    /*!< OCRAM0 Memory 0/1. The only block that can retain across Power Down. */
    kResc_Ocram0M23,        /*!< OCRAM0 Memory 2/3. */
    kResc_Ocram0M4,         /*!< OCRAM0 Memory 4. */
    kResc_Ocram0M5,         /*!< OCRAM0 Memory 5. */
    kResc_Ocram1M0,         /*!< OCRAM1 Memory 0. */
    kResc_Ocram1M1,         /*!< OCRAM1 Memory 1. */
    kResc_Ocram2M0,         /*!< OCRAM2 Memory 0. */
    kResc_Ocram2M1,         /*!< OCRAM2 Memory 1. */
    kResc_LlcCache,         /*!< LLC cache buffer. */
    kResc_Audio8Kb,         /*!< Audio SRAM 8KB. */
    kResc_M85Tcm,           /*!< M85 CPU TCM + cache. */
    kResc_NpuTcm,           /*!< NPU TCM. */
    kResc_CommTcm,          /*!< COMM SRAM. */
    kResc_MediaTcm,         /*!< Media SRAM. */

    /* Root clocks (power_root_clock_t) */
    kResc_RcgCompute,       /*!< COMPUTE_SS root clock. */
    kResc_RcgMain,          /*!< MAIN_SS root clock. */
    kResc_RcgWake,          /*!< WAKE_SS root clock. Stays on through Power Down. */
    kResc_RcgComm,          /*!< COMM_SS root clock. */
    kResc_RcgMedia,         /*!< MEDIA_SS root clock. */
    kResc_RcgAudio,         /*!< AUDIO_SS root clock. */
    kResc_RcgWake1M,        /*!< WAKE_1M root clock. Stays on through Power Down (wakeup detection). */
    kResc_RcgWake2M,        /*!< WAKE_2M root clock. Stays on through Power Down (wakeup detection). */

    /* Clock sources (power_clock_source_t) */
    kResc_ClkLdoa0V8,       /*!< LDOA 0.8V supply. */
    kResc_ClkFro192M,       /*!< Internal 192 MHz FRO. */
    kResc_ClkFro12M,        /*!< Internal 12 MHz FRO. */
    kResc_ClkMainPll,       /*!< Main PLL. */
    kResc_ClkCorePll,       /*!< Core PLL. */
    kResc_ClkSysPll,        /*!< System PLL. */
    kResc_ClkLdoq0V8,       /*!< LDOQ 0.8V supply. */
    kResc_ClkSxosc,         /*!< External crystal oscillator. */
    kResc_ClkFro12MLp,      /*!< Low-power 12 MHz FRO (wake). */

    /* Processor busy/execution state gates -- no fsl_power register binding.
     * These capture whether the processor core is ACTIVELY EXECUTING instructions.
     * Combined with kResc_Cm85Domain / kResc_NpuDomain (domain power), they model
     * the full three-level state hierarchy per processor:
     *   Active     : core executing (kResc_Cm85Active FULL_ON)   -- blocks Sleep+
     *   Clock Gate : core idle, domain ON (kResc_Cm85Domain FULL_ON) -- blocks Power Down+
     *   Power Gate : core idle, domain OFF (no constraint held)  -- deepest
     * See constraint macros PM_RESC_CM85_* / PM_RESC_NPU_* for the unified API. */
    kResc_Cm85Active,       /*!< CM85 core actively executing. Fix in every LP state (all require
                                 the core to stop). Combine with PM_RESC_CM85_CLOCK_GATE to
                                 express the CM85 state precisely. */
    kResc_NpuActive,        /*!< NPU actively computing. Fix from Sleep onward. */
    kResc_MainDmaActive,    /*!< MAIN-domain DMA (eDMA3/eDMA5) transfer in progress. Fix from Deep Sleep onward. */
    kResc_WakeDmaActive,    /*!< WAKE-domain DMA (eDMA3) transfer in progress. Fix from Deep Sleep onward. */

    /* Deep Power Down retention choice (power_deep_power_down_config_t.retainVbatSram) */
    kResc_VbatSram,         /*!< VBAT domain retention SRAM. Held = DPD1 (retain); not held = DPD2. */

    kResc_MaxNum
};

#define PM_CONFIGURABLE_RESOURCE_COUNT ((uint32_t)kResc_MaxNum)

/*!
 * @name CM85 processor state constraints.
 *
 * The CM85 state is modeled as a three-level hierarchy, implemented via two resources:
 *   kResc_Cm85Active (execution state) and kResc_Cm85Domain (domain power state).
 *
 *   Level           | kResc_Cm85Active | kResc_Cm85Domain | State excluded
 *   Active          | FULL_ON          | --               | Sleep + all deeper
 *   Clock Gate      | --               | FULL_ON          | Power Down + deeper (domain must stay on)
 *   Power Gate      | --               | --               | Nothing (domain off -- deepest)
 *
 * In Sleep mode: hardware always clock-gates the CM85 (CMC sequence). Holding
 * PM_RESC_CM85_CLOCK_GATE explicitly keeps PD_CPU powered so it can be clock-gated
 * (prevents domain from being fully powered off if Deep Sleep is selected instead).
 * In Deep Sleep: both clock gate (domain on) and power gate (domain off) are configurable
 * via PM_RESC_CM85_CLOCK_GATE / PM_RESC_CM85_POWER_GATE.
 * @{
 */
#define PM_RESC_CM85_ACTIVE      PM_RESC_FULL_ON(kResc_Cm85Active)  /*!< CM85 actively executing; blocks all LP states. */
#define PM_RESC_CM85_IDLE        PM_RESC_OFF(kResc_Cm85Active)      /*!< CM85 idle (no execution state constraint). */
#define PM_RESC_CM85_CLOCK_GATE  PM_RESC_FULL_ON(kResc_Cm85Domain)  /*!< CM85 clock-gated; PD_CPU powered. Blocks Power Down and deeper. */
#define PM_RESC_CM85_POWER_GATE  PM_RESC_OFF(kResc_Cm85Domain)      /*!< CM85 power-gated; PD_CPU off. No state excluded. */
/*! @} */

/*!
 * @name NPU processor state constraints.
 *
 * Same three-level model as CM85, using kResc_NpuActive and kResc_NpuDomain:
 *   Active      : PM_RESC_NPU_ACTIVE      -- blocks Sleep and deeper.
 *   Clock Gate  : PM_RESC_NPU_CLOCK_GATE  -- domain on; blocks Power Down and deeper.
 *   Power Gate  : PM_RESC_NPU_POWER_GATE  -- domain off; nothing excluded.
 * @{
 */
#define PM_RESC_NPU_ACTIVE       PM_RESC_FULL_ON(kResc_NpuActive)   /*!< NPU actively computing; blocks Sleep and deeper. */
#define PM_RESC_NPU_IDLE         PM_RESC_OFF(kResc_NpuActive)       /*!< NPU idle. */
#define PM_RESC_NPU_CLOCK_GATE   PM_RESC_FULL_ON(kResc_NpuDomain)   /*!< NPU clock-gated; PD_NPU powered. Blocks Power Down and deeper. */
#define PM_RESC_NPU_POWER_GATE   PM_RESC_OFF(kResc_NpuDomain)       /*!< NPU power-gated; PD_NPU off. No state excluded. */
/*! @} */

/*!
 * @name WAKE / MAIN / MEDIA / COMM domain constraints.
 * @{
 */
#define PM_RESC_WAKE_DOMAIN_ON   PM_RESC_FULL_ON(kResc_WakeDomain)  /*!< Keep PD_WAKE on. Excludes Deep Power Down. */
#define PM_RESC_WAKE_DOMAIN_OFF  PM_RESC_OFF(kResc_WakeDomain)      /*!< Allow PD_WAKE to be powered off. */

#define PM_RESC_MAIN_DOMAIN_ON   PM_RESC_FULL_ON(kResc_MainDomain)  /*!< Keep PD_MAIN on. Excludes Power Down and deeper. */
#define PM_RESC_MAIN_DOMAIN_OFF  PM_RESC_OFF(kResc_MainDomain)      /*!< Allow PD_MAIN to be powered off. */

#define PM_RESC_MEDIA_DOMAIN_ON  PM_RESC_FULL_ON(kResc_MediaDomain) /*!< Keep PD_MEDIA on. Excludes Power Down and deeper. */
#define PM_RESC_MEDIA_DOMAIN_OFF PM_RESC_OFF(kResc_MediaDomain)     /*!< Power off PD_MEDIA in the selected state. */

#define PM_RESC_COMM_DOMAIN_ON   PM_RESC_FULL_ON(kResc_CommDomain)  /*!< Keep PD_COMM on. Excludes Power Down and deeper. */
#define PM_RESC_COMM_DOMAIN_OFF  PM_RESC_OFF(kResc_CommDomain)      /*!< Power off PD_COMM in the selected state. */
/*! @} */

/*!
 * @name Memory retention constraints.
 *
 * Three modes are available for each memory block in Deep Sleep:
 *   _ACTIVE     (PM_RESOURCE_FULL_ON)    : block fully powered (kMEMCON_PowerModeActive).
 *   _RETENTION  (PM_RESOURCE_PARTABLE_ON1): block in retention; data preserved at lower power
 *                                          (kMEMCON_PowerModeRetention).
 *   _POWER_DOWN (PM_RESOURCE_OFF)        : block powered down; data lost
 *                                          (kMEMCON_PowerModePowerDown).
 *
 * For kResc_WakeSram and kResc_ComputeSram01 only, _RETENTION also applies in Power Down
 * (the only two blocks whose retention supply survives Power Down). All other blocks are
 * unconditionally lost in Power Down regardless of any constraint held.
 *
 * WARNING: The default when NO constraint is held is PowerDown (data lost). Applications
 *          must explicitly hold at least _RETENTION for any block whose data must survive
 *          the low-power transition.
 * @{
 */
#define PM_RESC_WAKE_SRAM_ACTIVE          PM_RESC_FULL_ON(kResc_WakeSram)         /*!< WAKE SRAM fully active. */
#define PM_RESC_WAKE_SRAM_RETENTION       PM_RESC_PART_ON1(kResc_WakeSram)        /*!< WAKE SRAM data retained. */
#define PM_RESC_WAKE_SRAM_POWER_DOWN      PM_RESC_OFF(kResc_WakeSram)             /*!< WAKE SRAM powered down (data lost). */

#define PM_RESC_COMPUTE_SRAM01_ACTIVE     PM_RESC_FULL_ON(kResc_ComputeSram01)    /*!< OCRAM0 M0/1 fully active. */
#define PM_RESC_COMPUTE_SRAM01_RETENTION  PM_RESC_PART_ON1(kResc_ComputeSram01)   /*!< OCRAM0 M0/1 data retained. Survives Power Down. */
#define PM_RESC_COMPUTE_SRAM01_POWER_DOWN PM_RESC_OFF(kResc_ComputeSram01)        /*!< OCRAM0 M0/1 powered down (data lost). */

#define PM_RESC_OCRAM0_M23_ACTIVE         PM_RESC_FULL_ON(kResc_Ocram0M23)        /*!< OCRAM0 M2/3 fully active. */
#define PM_RESC_OCRAM0_M23_RETENTION      PM_RESC_PART_ON1(kResc_Ocram0M23)       /*!< OCRAM0 M2/3 data retained. */
#define PM_RESC_OCRAM0_M23_POWER_DOWN     PM_RESC_OFF(kResc_Ocram0M23)            /*!< OCRAM0 M2/3 powered down (data lost). */

#define PM_RESC_OCRAM0_M4_ACTIVE          PM_RESC_FULL_ON(kResc_Ocram0M4)         /*!< OCRAM0 M4 fully active. */
#define PM_RESC_OCRAM0_M4_RETENTION       PM_RESC_PART_ON1(kResc_Ocram0M4)        /*!< OCRAM0 M4 data retained. */
#define PM_RESC_OCRAM0_M4_POWER_DOWN      PM_RESC_OFF(kResc_Ocram0M4)             /*!< OCRAM0 M4 powered down (data lost). */

#define PM_RESC_OCRAM0_M5_ACTIVE          PM_RESC_FULL_ON(kResc_Ocram0M5)         /*!< OCRAM0 M5 fully active. */
#define PM_RESC_OCRAM0_M5_RETENTION       PM_RESC_PART_ON1(kResc_Ocram0M5)        /*!< OCRAM0 M5 data retained. */
#define PM_RESC_OCRAM0_M5_POWER_DOWN      PM_RESC_OFF(kResc_Ocram0M5)             /*!< OCRAM0 M5 powered down (data lost). */

#define PM_RESC_OCRAM1_M0_ACTIVE          PM_RESC_FULL_ON(kResc_Ocram1M0)         /*!< OCRAM1 M0 fully active. */
#define PM_RESC_OCRAM1_M0_RETENTION       PM_RESC_PART_ON1(kResc_Ocram1M0)        /*!< OCRAM1 M0 data retained. */
#define PM_RESC_OCRAM1_M0_POWER_DOWN      PM_RESC_OFF(kResc_Ocram1M0)             /*!< OCRAM1 M0 powered down (data lost). */

#define PM_RESC_OCRAM1_M1_ACTIVE          PM_RESC_FULL_ON(kResc_Ocram1M1)         /*!< OCRAM1 M1 fully active. */
#define PM_RESC_OCRAM1_M1_RETENTION       PM_RESC_PART_ON1(kResc_Ocram1M1)        /*!< OCRAM1 M1 data retained. */
#define PM_RESC_OCRAM1_M1_POWER_DOWN      PM_RESC_OFF(kResc_Ocram1M1)             /*!< OCRAM1 M1 powered down (data lost). */

#define PM_RESC_OCRAM2_M0_ACTIVE          PM_RESC_FULL_ON(kResc_Ocram2M0)         /*!< OCRAM2 M0 fully active. */
#define PM_RESC_OCRAM2_M0_RETENTION       PM_RESC_PART_ON1(kResc_Ocram2M0)        /*!< OCRAM2 M0 data retained. */
#define PM_RESC_OCRAM2_M0_POWER_DOWN      PM_RESC_OFF(kResc_Ocram2M0)             /*!< OCRAM2 M0 powered down (data lost). */

#define PM_RESC_OCRAM2_M1_ACTIVE          PM_RESC_FULL_ON(kResc_Ocram2M1)         /*!< OCRAM2 M1 fully active. */
#define PM_RESC_OCRAM2_M1_RETENTION       PM_RESC_PART_ON1(kResc_Ocram2M1)        /*!< OCRAM2 M1 data retained. */
#define PM_RESC_OCRAM2_M1_POWER_DOWN      PM_RESC_OFF(kResc_Ocram2M1)             /*!< OCRAM2 M1 powered down (data lost). */

#define PM_RESC_LLC_CACHE_ACTIVE          PM_RESC_FULL_ON(kResc_LlcCache)         /*!< LLC cache fully active. */
#define PM_RESC_LLC_CACHE_RETENTION       PM_RESC_PART_ON1(kResc_LlcCache)        /*!< LLC cache data retained. */
#define PM_RESC_LLC_CACHE_POWER_DOWN      PM_RESC_OFF(kResc_LlcCache)             /*!< LLC cache powered down (data lost). */

#define PM_RESC_AUDIO_8KB_ACTIVE          PM_RESC_FULL_ON(kResc_Audio8Kb)         /*!< Audio SRAM 8KB fully active. */
#define PM_RESC_AUDIO_8KB_RETENTION       PM_RESC_PART_ON1(kResc_Audio8Kb)        /*!< Audio SRAM 8KB data retained. */
#define PM_RESC_AUDIO_8KB_POWER_DOWN      PM_RESC_OFF(kResc_Audio8Kb)             /*!< Audio SRAM 8KB powered down (data lost). */

#define PM_RESC_M85_TCM_ACTIVE            PM_RESC_FULL_ON(kResc_M85Tcm)           /*!< M85 TCM+cache fully active. */
#define PM_RESC_M85_TCM_RETENTION         PM_RESC_PART_ON1(kResc_M85Tcm)          /*!< M85 TCM+cache data retained. */
#define PM_RESC_M85_TCM_POWER_DOWN        PM_RESC_OFF(kResc_M85Tcm)               /*!< M85 TCM+cache powered down (data lost). */

#define PM_RESC_NPU_TCM_ACTIVE            PM_RESC_FULL_ON(kResc_NpuTcm)           /*!< NPU TCM fully active. */
#define PM_RESC_NPU_TCM_RETENTION         PM_RESC_PART_ON1(kResc_NpuTcm)          /*!< NPU TCM data retained. */
#define PM_RESC_NPU_TCM_POWER_DOWN        PM_RESC_OFF(kResc_NpuTcm)               /*!< NPU TCM powered down (data lost). */

#define PM_RESC_COMM_TCM_ACTIVE           PM_RESC_FULL_ON(kResc_CommTcm)          /*!< COMM SRAM fully active. */
#define PM_RESC_COMM_TCM_RETENTION        PM_RESC_PART_ON1(kResc_CommTcm)         /*!< COMM SRAM data retained. */
#define PM_RESC_COMM_TCM_POWER_DOWN       PM_RESC_OFF(kResc_CommTcm)              /*!< COMM SRAM powered down (data lost). */

#define PM_RESC_MEDIA_TCM_ACTIVE          PM_RESC_FULL_ON(kResc_MediaTcm)         /*!< Media SRAM fully active. */
#define PM_RESC_MEDIA_TCM_RETENTION       PM_RESC_PART_ON1(kResc_MediaTcm)        /*!< Media SRAM data retained. */
#define PM_RESC_MEDIA_TCM_POWER_DOWN      PM_RESC_OFF(kResc_MediaTcm)             /*!< Media SRAM powered down (data lost). */
/*! @} */

/*!
 * @name Root clock constraints.
 *
 * Holding _ON forces the root clock to remain enabled when entering Sleep or Deep Sleep.
 * Holding _OFF allows the root clock to be gated in the selected state.
 *
 * Sleep baseline: all 8 root clocks are ON; Deep Sleep (DS1) baseline gates all except
 * WAKE/WAKE_1M/WAKE_2M. WAKE/WAKE_1M/WAKE_2M stay on unconditionally through Power Down
 * (required for wakeup-detection logic) -- they have no _OFF variant.
 * @{
 */
#define PM_RESC_RCG_COMPUTE_ON  PM_RESC_FULL_ON(kResc_RcgCompute)  /*!< Keep COMPUTE_SS root clock on. */
#define PM_RESC_RCG_COMPUTE_OFF PM_RESC_OFF(kResc_RcgCompute)      /*!< Allow COMPUTE_SS root clock to be gated. */

#define PM_RESC_RCG_MAIN_ON     PM_RESC_FULL_ON(kResc_RcgMain)     /*!< Keep MAIN_SS root clock on. */
#define PM_RESC_RCG_MAIN_OFF    PM_RESC_OFF(kResc_RcgMain)         /*!< Allow MAIN_SS root clock to be gated. */

#define PM_RESC_RCG_WAKE_ON     PM_RESC_FULL_ON(kResc_RcgWake)     /*!< Keep WAKE_SS root clock on. */
#define PM_RESC_RCG_WAKE_OFF    PM_RESC_OFF(kResc_RcgWake)         /*!< Allow WAKE_SS root clock to be gated (e.g. in Deep Power Down). */

#define PM_RESC_RCG_COMM_ON     PM_RESC_FULL_ON(kResc_RcgComm)     /*!< Keep COMM_SS root clock on. */
#define PM_RESC_RCG_COMM_OFF    PM_RESC_OFF(kResc_RcgComm)         /*!< Allow COMM_SS root clock to be gated. */

#define PM_RESC_RCG_MEDIA_ON    PM_RESC_FULL_ON(kResc_RcgMedia)    /*!< Keep MEDIA_SS root clock on. */
#define PM_RESC_RCG_MEDIA_OFF   PM_RESC_OFF(kResc_RcgMedia)        /*!< Allow MEDIA_SS root clock to be gated. */

#define PM_RESC_RCG_AUDIO_ON    PM_RESC_FULL_ON(kResc_RcgAudio)    /*!< Keep AUDIO_SS root clock on. */
#define PM_RESC_RCG_AUDIO_OFF   PM_RESC_OFF(kResc_RcgAudio)        /*!< Allow AUDIO_SS root clock to be gated. */

#define PM_RESC_RCG_WAKE_1M_ON  PM_RESC_FULL_ON(kResc_RcgWake1M)   /*!< Keep WAKE_1M root clock on. */
#define PM_RESC_RCG_WAKE_1M_OFF PM_RESC_OFF(kResc_RcgWake1M)       /*!< Allow WAKE_1M root clock to be gated (e.g. in Deep Power Down). */
#define PM_RESC_RCG_WAKE_2M_ON  PM_RESC_FULL_ON(kResc_RcgWake2M)   /*!< Keep WAKE_2M root clock on. */
#define PM_RESC_RCG_WAKE_2M_OFF PM_RESC_OFF(kResc_RcgWake2M)       /*!< Allow WAKE_2M root clock to be gated (e.g. in Deep Power Down). */
/*! @} */

/*!
 * @name Clock source constraints.
 *
 * Holding _ON forces the clock source to remain enabled; holding _OFF allows it to be gated.
 *
 * Sleep baseline: all 9 sources ON. Deep Sleep (DS1) baseline: LDOA_0V8/FRO_192M/FRO_12M/
 * FRO_12M_LP ON; MAIN_PLL/CORE_PLL/SYS_PLL/LDOQ_0V8/SXOSC OFF.
 *
 * All 9 clock sources are unavailable in Power Down and deeper -- holding any _ON constraint
 * will cause the policy engine to exclude Power Down (the resource appears in its
 * fixConstraintsMask).
 * @{
 */
#define PM_RESC_CLK_LDOA_0V8_ON    PM_RESC_FULL_ON(kResc_ClkLdoa0V8)  /*!< Keep LDOA 0.8V on. */
#define PM_RESC_CLK_LDOA_0V8_OFF   PM_RESC_OFF(kResc_ClkLdoa0V8)      /*!< Allow LDOA 0.8V to be gated. */

#define PM_RESC_CLK_FRO_192M_ON    PM_RESC_FULL_ON(kResc_ClkFro192M)  /*!< Keep FRO 192 MHz on. */
#define PM_RESC_CLK_FRO_192M_OFF   PM_RESC_OFF(kResc_ClkFro192M)      /*!< Allow FRO 192 MHz to be gated. */

#define PM_RESC_CLK_FRO_12M_ON     PM_RESC_FULL_ON(kResc_ClkFro12M)   /*!< Keep FRO 12 MHz on. */
#define PM_RESC_CLK_FRO_12M_OFF    PM_RESC_OFF(kResc_ClkFro12M)       /*!< Allow FRO 12 MHz to be gated. */

#define PM_RESC_CLK_MAIN_PLL_ON    PM_RESC_FULL_ON(kResc_ClkMainPll)  /*!< Keep MAIN PLL on (excludes Power Down and deeper). */
#define PM_RESC_CLK_MAIN_PLL_OFF   PM_RESC_OFF(kResc_ClkMainPll)      /*!< Allow MAIN PLL to be gated. */

#define PM_RESC_CLK_CORE_PLL_ON    PM_RESC_FULL_ON(kResc_ClkCorePll)  /*!< Keep CORE PLL on (excludes Power Down and deeper). */
#define PM_RESC_CLK_CORE_PLL_OFF   PM_RESC_OFF(kResc_ClkCorePll)      /*!< Allow CORE PLL to be gated. */

#define PM_RESC_CLK_SYSPLL_ON      PM_RESC_FULL_ON(kResc_ClkSysPll)   /*!< Keep SYS PLL on (excludes Power Down and deeper). */
#define PM_RESC_CLK_SYSPLL_OFF     PM_RESC_OFF(kResc_ClkSysPll)       /*!< Allow SYS PLL to be gated. */

#define PM_RESC_CLK_LDOQ_0V8_ON    PM_RESC_FULL_ON(kResc_ClkLdoq0V8) /*!< Keep LDOQ 0.8V on. */
#define PM_RESC_CLK_LDOQ_0V8_OFF   PM_RESC_OFF(kResc_ClkLdoq0V8)     /*!< Allow LDOQ 0.8V to be gated. */

#define PM_RESC_CLK_SXOSC_ON       PM_RESC_FULL_ON(kResc_ClkSxosc)   /*!< Keep SXOSC on. */
#define PM_RESC_CLK_SXOSC_OFF      PM_RESC_OFF(kResc_ClkSxosc)       /*!< Allow SXOSC to be gated. */

#define PM_RESC_CLK_FRO_12M_LP_ON  PM_RESC_FULL_ON(kResc_ClkFro12MLp) /*!< Keep FRO 12 MHz LP on. */
#define PM_RESC_CLK_FRO_12M_LP_OFF PM_RESC_OFF(kResc_ClkFro12MLp)    /*!< Allow FRO 12 MHz LP to be gated. */
/*! @} */

/*!
 * @name DMA busy/idle constraints.
 *
 * Hold while a DMA transfer is in progress; release when done. Fix from Deep Sleep onward:
 * if MAIN/WAKE DMA is in progress, Deep Sleep and deeper are blocked.
 *
 * See PM_RESC_CM85_ACTIVE / PM_RESC_NPU_ACTIVE in the processor sections above for the
 * equivalent execution-state gates for CM85 and NPU.
 * @{
 */
#define PM_RESC_MAIN_DMA_ACTIVE  PM_RESC_FULL_ON(kResc_MainDmaActive) /*!< MAIN DMA transfer in progress; block Deep Sleep and deeper. */
#define PM_RESC_MAIN_DMA_IDLE    PM_RESC_OFF(kResc_MainDmaActive)     /*!< MAIN DMA idle; Deep Sleep and deeper are permitted. */
#define PM_RESC_WAKE_DMA_ACTIVE  PM_RESC_FULL_ON(kResc_WakeDmaActive) /*!< WAKE DMA transfer in progress; block Deep Sleep and deeper. */
#define PM_RESC_WAKE_DMA_IDLE    PM_RESC_OFF(kResc_WakeDmaActive)     /*!< WAKE DMA idle; Deep Sleep and deeper are permitted. */
/*! @} */

/*!
 * @name VBAT SRAM retention constraint.
 *
 * @{
 */
#define PM_RESC_VBAT_SRAM_RETAIN PM_RESC_FULL_ON(kResc_VbatSram) /*!< DPD1: retain VBAT SRAM (2 KB). */
/*! @} */

/*! @} */

#endif /* _FSL_PM_DEVICE_H_ */
