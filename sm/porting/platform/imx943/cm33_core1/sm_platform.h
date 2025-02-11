/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SM_PLATFORM_H_
#define SM_PLATFORM_H_

#include "fsl_common.h"
#include "scmi.h"
#include "scmi_internal.h"
#include "smt.h"
#include "fsl_cpu.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* MU8_MUA (2*(8-1) = 14) */
#define SM_PLATFORM_MU_INST 14
#define SM_PLATFORM_MU_IRQ MU8_A_IRQn

#define SM_PLATFORM_A2P      0 /* SCMI Agent -> SCMI Platform */
#define SM_PLATFORM_NOTIFY   1 /* SCMI Platform -> SCMI Agent */
#define SM_PLATFORM_PRIORITY 2

/* Logical Machine */
#define SM_PLATFORM_LMID_A55 (4U)

/* Pinctrl(from fsl_iomuxc.h) */
#define SM_PLATFORM_PINCTRL_MUX_MODE_MASK  (0x7U)
#define SM_PLATFORM_PINCTRL_MUX_MODE_SHIFT (0U)
#define SM_PLATFORM_PINCTRL_MUX_MODE(x) \
    (((uint32_t)(((uint32_t)(x)) << SM_PLATFORM_PINCTRL_MUX_MODE_SHIFT)) & SM_PLATFORM_PINCTRL_MUX_MODE_MASK)

#define SM_PLATFORM_PINCTRL_SION_MASK  (0x10)
#define SM_PLATFORM_PINCTRL_SION_SHIFT (4U)
#define SM_PLATFORM_PINCTRL_SION(x) \
    (((uint32_t)(((uint32_t)(x)) << SM_PLATFORM_PINCTRL_SION_SHIFT)) & SM_PLATFORM_PINCTRL_SION_MASK)

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
/*
 * SM API interface using non-security address, so need to convert
 * security address to non-security address.
 * bit28 is flag of security address
 */
#define SM_PLATFORM_SECURITY_MASK (0x10000000U)
#define SM_PLATFORM_PINCTRL_BASE         (IOMUXC_BASE & (~SM_PLATFORM_SECURITY_MASK))
#else
#define SM_PLATFORM_PINCTRL_BASE         (IOMUXC_BASE)
#endif

#define SM_PLATFORM_PINCTRL_MUXREG_OFF   (SM_PLATFORM_PINCTRL_BASE)
#define SM_PLATFORM_PINCTRL_CFGREG_OFF   (SM_PLATFORM_PINCTRL_BASE + 0x304) /* 0x443c0304 */
#define SM_PLATFORM_PINCTRL_DAISYREG_OFF (SM_PLATFORM_PINCTRL_BASE + 0x608) /* 0x443c0608 */

/* Doorbell*/
#define SM_PLATFORM_DBIR_A2P      0  /* A2P channel */
#define SM_PLATFORM_DBIR_NOTIFY   1  /* P2A notify */
#define SM_PLATFORM_DBIR_PRIORITY 2U /* P2A priority */

#define SM_PLATFORM_SMA_ADDR 0

/* BBM(RTC) */
#define SM_PLATFORM_RTC_ID 0

/* FUSA */
#define SM_PLATFORM_FAULT_ID_FIRST 6U
#define SM_PLATFORM_FAULT_MASK     0x1
#define SM_PLATFORM_NOTIFY_ENABLE  0x1

/* CPU Id */
#define SM_PLATFORM_AP_ID (CPU_IDX_A55C0)
#define SM_PLATFORM_M33S_ID (CPU_IDX_M33P_S)
#define SM_PLATFORM_M70_ID (CPU_IDX_M7P_0)
#define SM_PLATFORM_M71_ID (CPU_IDX_M7P_1)

/* Domain id(same with components/power/porting/platform/imx943/hal_power_platform.h) */
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_ANA           0U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_AON           1U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_BBSM          2U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_M7_1          3U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_CCMSRCGPC     4U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_A55C0         5U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_A55C1         6U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_A55C2         7U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_A55C3         8U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_A55P          9U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_DDR           10U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_DISPLAY       11U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_M7_0          12U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_HSIO_TOP      13U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_HSIO_WAON     14U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_NETC          15U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_NOC           16U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_NPU           17U
#define SM_POWER_PLATFORM_MIX_SLICE_IDX_WAKEUP        18U

#ifndef SM_PLATFORM_RTC_NOTIFY
#define SM_PLATFORM_RTC_NOTIFY 0
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /*__cplusplus */

/*!
 * @brief Initialize MU interface for SM access.
 */
void SM_Platform_Init(void);

/*!
 * @brief Deinitialize MU interface for SM access.
 */
void SM_Platform_Deinit(void);

/*!
 * @brief SM Platform Set System State
 */
void SM_Platform_SetSystemState(uint32_t systemState);

/*!
 * @brief SM Platform Get System State
 */
uint32_t SM_Platform_GetSystemState(void);

#if defined(__cplusplus)
}
#endif /*__cplusplus */

/*! @} */

#endif /* SM_PLATFORM_H_ */
