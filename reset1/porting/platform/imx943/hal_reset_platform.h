/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HAL_RESET_PLATFORM_H_
#define HAL_RESET_PLATFORM_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define HAL_RST_PLATFORM_MAX_ID 58U    

/* Individual resets (IRST) */
#define HAL_RST_LINE_A55C0_NCPUPORESET      0U
#define HAL_RST_LINE_A55C0_NCORERESET       1U
#define HAL_RST_LINE_A55C1_NCPUPORESET      2U
#define HAL_RST_LINE_A55C1_NCORERESET       3U
#define HAL_RST_LINE_A55C2_NCPUPORESET      4U
#define HAL_RST_LINE_A55C2_NCORERESET       5U
#define HAL_RST_LINE_A55C3_NCPUPORESET      6U
#define HAL_RST_LINE_A55C3_NCORERESET       7U
#define HAL_RST_LINE_A55P_NPRESET           8U
#define HAL_RST_LINE_A55P_NSPORESET         9U
#define HAL_RST_LINE_A55P_NSRESET           10U
#define HAL_RST_LINE_A55P_NATRESET          11U
#define HAL_RST_LINE_A55P_NGICRESET         12U
#define HAL_RST_LINE_A55P_NPERIPHRESET      13U
#define HAL_RST_LINE_DDRPHY_PRESETN         14U
#define HAL_RST_LINE_DDRPHY_RESETN          15U
#define HAL_RST_LINE_DISP0_RESETN           16U
#define HAL_RST_LINE_DISP1_RESETN           17U
#define HAL_RST_LINE_LVDS_RESETN            18U
#define HAL_RST_LINE_USB1PHY_RESETN         19U
#define HAL_RST_LINE_USB2PHY_RESETN         20U
#define HAL_RST_LINE_PCIE1PHY_RESETN        21U
#define HAL_RST_LINE_PCIE2PHY_RESETN        22U
#define HAL_RST_LINE_ENETPHY_PCS_RESETN     23U
#define HAL_RST_LINE_ECAT_IP_RESETN         24U
#define HAL_RST_LINE_V2X_RESETB             25U
#define HAL_RST_LINE_BISS_RESETN            26U
#define HAL_RST_LINE_ENDAT3_RESETN          27U
#define HAL_RST_LINE_ENDAT2_RESETN          28U
#define HAL_RST_LINE_HIPERFACE_RESETN       29U

/* Regional resets (RSTR) */      
#define HAL_RST_LINE_ANAMIX                 30U
#define HAL_RST_LINE_AONMIX_TOP             31U
#define HAL_RST_LINE_AONMIX_M33             32U
#define HAL_RST_LINE_AONMIX_ELE             33U
#define HAL_RST_LINE_BBSMMIX                34U
#define HAL_RST_LINE_M71MIX                 35U
#define HAL_RST_LINE_CCMSRCGPCMIX           36U
#define HAL_RST_LINE_CORTEXAMIX_CORE0       37U
#define HAL_RST_LINE_CORTEXAMIX_CORE1       38U
#define HAL_RST_LINE_CORTEXAMIX_CORE2       39U
#define HAL_RST_LINE_CORTEXAMIX_CORE3       40U
#define HAL_RST_LINE_CORTEXAMIX_PLATFORM    41U
#define HAL_RST_LINE_CORTEXAMIX_WDOG_3_4    42U
#define HAL_RST_LINE_DDRMIX_TOP             43U
#define HAL_RST_LINE_DDRMIX_PHY             44U
#define HAL_RST_LINE_DISPLAYMIX             45U
#define HAL_RST_LINE_M70MIX                 46U
#define HAL_RST_LINE_HSIOMIX_TOP            47U
#define HAL_RST_LINE_HSIOMIX_WAON           48U
#define HAL_RST_LINE_NETCMIX                49U
#define HAL_RST_LINE_NETCMIX_M33            50U
#define HAL_RST_LINE_NETCMIX_WDOG_7_8       51U
#define HAL_RST_LINE_NOCMIX                 52U
#define HAL_RST_LINE_NPUMIX                 53U
#define HAL_RST_LINE_WAKEUPMIX_TOP          54U
#define HAL_RST_LINE_WAKEUPMIX_JTAG         55U
#define HAL_RST_LINE_WAKEUPMIX_WDOG_5       56U
#define HAL_RST_LINE_WAKEUPMIX_WDOG_6       57U
 

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

#endif /* HAL_RESET_PLATFORM_H_ */
