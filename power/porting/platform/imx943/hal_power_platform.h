/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HAL_POWER_PLATFORM_H_
#define HAL_POWER_PLATFORM_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define HAL_POWER_PLATFORM_NUM_MIX_SLICE               19U

#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_ANA           0U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_AON           1U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_BBSM          2U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_M7_1          3U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_CCMSRCGPC     4U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_A55C0         5U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_A55C1         6U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_A55C2         7U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_A55C3         8U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_A55P          9U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_DDR           10U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_DISPLAY       11U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_M7_0          12U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_HSIO_TOP      13U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_HSIO_WAON     14U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_NETC          15U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_NOC           16U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_NPU           17U
#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_WAKEUP        18U

#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_AON           0U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_M7_1          1U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_A55C0         2U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_A55C1         3U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_A55C2         4U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_A55C3         5U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_A55P          6U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_A55L3         7U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_DDR           8U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_DISPLAY       9U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_M7_0          10U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_HSIO          11U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_NETC          12U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_NOC1          13U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_NOC2          14U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_NPU           15U
#define HAL_POWER_PLATFORM_MEM_SLICE_IDX_WAKEUP        16U

#define HAL_POWER_PLATFORM_MIX_SLICE_IDX_A55C_LAST     HAL_POWER_PLATFORM_MIX_SLICE_IDX_A55C3

#define HAL_POWER_PLATFORM_MIX_PSW_STAT_MASK           SRC_XSPR_FUNC_STAT_PSW_STAT_MASK
#define HAL_POWER_PLATFORM_MIX_PSW_STAT_PUP            SRC_XSPR_FUNC_STAT_PSW_STAT(0U)
#define HAL_POWER_PLATFORM_MIX_PSW_STAT_PDN            SRC_XSPR_FUNC_STAT_PSW_STAT(1U)

#define HAL_POWER_PLATFORM_MIX_FUNC_STAT_MASK                  \
    (SRC_XSPR_FUNC_STAT_SYSMAN_STAT_MASK |      \
     SRC_XSPR_FUNC_STAT_MEM_STAT_MASK |         \
     SRC_XSPR_FUNC_STAT_A55_HDSK_STAT_MASK |    \
     SRC_XSPR_FUNC_STAT_SSAR_STAT_MASK |        \
     SRC_XSPR_FUNC_STAT_ISO_STAT_MASK |         \
     SRC_XSPR_FUNC_STAT_RST_STAT_MASK |         \
     SRC_XSPR_FUNC_STAT_PSW_STAT_MASK)

#define HAL_POWER_PLATFORM_MIX_FUNC_STAT_PUP                   \
    (SRC_XSPR_FUNC_STAT_SYSMAN_STAT(0U) |       \
     SRC_XSPR_FUNC_STAT_MEM_STAT(0U) |          \
     SRC_XSPR_FUNC_STAT_A55_HDSK_STAT(0U) |     \
     SRC_XSPR_FUNC_STAT_SSAR_STAT(0U) |         \
     SRC_XSPR_FUNC_STAT_ISO_STAT(0U) |          \
     SRC_XSPR_FUNC_STAT_RST_STAT(1U) |          \
     SRC_XSPR_FUNC_STAT_PSW_STAT(0U))

#define HAL_POWER_PLATFORM_MIX_FUNC_STAT_PDN                   \
    (SRC_XSPR_FUNC_STAT_SYSMAN_STAT(1U) |       \
     SRC_XSPR_FUNC_STAT_MEM_STAT(1U) |          \
     SRC_XSPR_FUNC_STAT_A55_HDSK_STAT(1U) |     \
     SRC_XSPR_FUNC_STAT_SSAR_STAT(1U) |         \
     SRC_XSPR_FUNC_STAT_ISO_STAT(1U) |          \
     SRC_XSPR_FUNC_STAT_RST_STAT(0U) |          \
     SRC_XSPR_FUNC_STAT_PSW_STAT(1U))

/* Macro to convert CPU ID to GPC domain ID
 * Note:  AUTHEN_CTRL.WHITE_LIST assignments use macro
 *        to define LPM voting logic mappings.  Offset
 *        mapping to GPC domain by TRDC ID assigned to M33
 *        to avoid collisions between WHITE_LIST access control
 *        and LPM voting logic.
 */
#define CM33_TRDC_ID                2U
#define CPU2GPC(cpuId)              ((cpuId) + CM33_TRDC_ID)

#define LPMSETTING_MASK(cpuId)          (0x7ULL << ((CPU2GPC(cpuId) << 2U)))
#define LPMSETTING_DOM(cpuId, lpmVal)   (((uint64_t) lpmVal) << ((CPU2GPC(cpuId) << 2U)))
#define LPMSETTING_VAL(cpuId, lpmReg)   ((((uint64_t) lpmReg) & LPMSETTING_MASK(cpuId)) >> ((CPU2GPC(cpuId) << 2U)))

#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_M7_0         0U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_S500         1U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_NPU          2U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_A55          3U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_M33          4U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_AON          5U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_DDR          6U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_WAKEUP       7U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_NOC          8U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_M7_1         9U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_HSIO         10U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_M33_S        11U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_NETC         12U
#define HAL_POWER_PLATFORM_GPC_REQ_MIX_ID_DISPLAY      13U

#define HAL_POWER_PLATFORM_GPC_REQ_STATE_CLK_GATE      1U
#define HAL_POWER_PLATFORM_GPC_REQ_STATE_CLK_CHANGE    2U
#define HAL_POWER_PLATFORM_GPC_REQ_STATE_RESET         3U
#define HAL_POWER_PLATFORM_GPC_REQ_STATE_POWER         4U
#define HAL_POWER_PLATFORM_GPC_REQ_STATE_RETENTION     5U

#define HAL_POWER_PLATFORM_GPC_HS_RST_AON              1U
#define HAL_POWER_PLATFORM_GPC_HS_RST_M33P             2U
#define HAL_POWER_PLATFORM_GPC_HS_RST_ELE              3U
#define HAL_POWER_PLATFORM_GPC_HS_RST_M7_1             5U
#define HAL_POWER_PLATFORM_GPC_HS_RST_A55C0            7U
#define HAL_POWER_PLATFORM_GPC_HS_RST_A55C1            8U
#define HAL_POWER_PLATFORM_GPC_HS_RST_A55C2            9U
#define HAL_POWER_PLATFORM_GPC_HS_RST_A55C3            10U
#define HAL_POWER_PLATFORM_GPC_HS_RST_A55P             11U
#define HAL_POWER_PLATFORM_GPC_HS_RST_DDR              12U
#define HAL_POWER_PLATFORM_GPC_HS_RST_DDRPHY           13U
#define HAL_POWER_PLATFORM_GPC_HS_RST_DISPLAY          14U
#define HAL_POWER_PLATFORM_GPC_HS_RST_M7_0             15U
#define HAL_POWER_PLATFORM_GPC_HS_RST_HSIO             16U
#define HAL_POWER_PLATFORM_GPC_HS_RST_HSIOAON          17U
#define HAL_POWER_PLATFORM_GPC_HS_RST_NETC             18U
#define HAL_POWER_PLATFORM_GPC_HS_RST_M33S             19U
#define HAL_POWER_PLATFORM_GPC_HS_RST_WDOG_7_8         20U
#define HAL_POWER_PLATFORM_GPC_HS_RST_NOC              21U
#define HAL_POWER_PLATFORM_GPC_HS_RST_WDOG_3_4         22U
#define HAL_POWER_PLATFORM_GPC_HS_RST_NPU              23U
#define HAL_POWER_PLATFORM_GPC_HS_RST_WAKEUP           24U
#define HAL_POWER_PLATFORM_GPC_HS_RST_JTAG             25U
#define HAL_POWER_PLATFORM_GPC_HS_RST_WDOG5            26U
#define HAL_POWER_PLATFORM_GPC_HS_RST_WDOG6            27U

/* FIXME, the index should start from 30 */
#define HAL_POWER_PLATFORM_GPC_HS_PWR_M7_1             31U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_A55C0            1U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_A55C1            2U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_A55C2            3U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_A55C3            4U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_A55P             5U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_DDR              6U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_DISPLAY          7U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_M7_0             8U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_HSIO             9U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_HSIOAON          10U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_NETC             11U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_NOC              12U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_NPU              13U
#define HAL_POWER_PLATFORM_GPC_HS_PWR_WAKEUP           14U

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /*__cplusplus */

#if defined(__cplusplus)
}
#endif /*__cplusplus */

/*! @} */

#endif /* HAL_POWER_PLATFORM_H_ */
