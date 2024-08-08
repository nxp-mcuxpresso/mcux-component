/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HAL_CLOCK_PLATFORM_H_
#define HAL_CLOCK_PLATFORM_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define HAL_CLOCK_PLATFORM_SOURCE_NUM 46
/* hal_clock_invalid */
#define HAL_CLOCK_PLATFORM_MAX_ID     (HAL_CLOCK_PLATFORM_SOURCE_NUM + 128)
#define HAL_CLOCK_PLATFORM_MUX_MAX_ID 4U

/*******************************************************************************
 * HAL Clock Identifier
 ******************************************************************************/
/*!
 * @brief hal clock identifier
 */
typedef enum
{
    /* unify clock id for clock sources and ips */
    /* clock id for clock sources */
    hal_clock_ext = 0, /* clock id for EXT_CLK */
    hal_clock_osc32k = 1, /* clock id for OSC_32K_CLK */
    hal_clock_osc24m = 2, /* clock id for OSC_24M_CLK */
    hal_clock_fro = 3, /* clock id for FRO_CLK */
    hal_clock_syspll1ctl = 4, /* clock id for SYS_PLL1_CTL */
    hal_clock_syspll1dfs0ctl = 5, /* clock id for SYS_PLL1_DFS0_CTL */
    hal_clock_syspll1dfs0 = 6, /* clock id for SYS_PLL1_DFS0_CLK */
    hal_clock_syspll1dfs0div2 = 7, /* clock id for SYS_PLL1_DFS0_DIV2_CLK */
    hal_clock_syspll1dfs1ctl = 8, /* clock id for SYS_PLL1_DFS1_CTL */
    hal_clock_syspll1dfs1 = 9, /* clock id for SYS_PLL1_DFS1_CLK */
    hal_clock_syspll1dfs1div2 = 10, /* clock id for SYS_PLL1_DFS1_DIV2_CLK */
    hal_clock_syspll1dfs2ctl = 11, /* clock id for SYS_PLL1_DFS2_CTL */
    hal_clock_syspll1dfs2 = 12, /* clock id for SYS_PLL1_DFS2_CLK */
    hal_clock_syspll1dfs2div2 = 13, /* clock id for SYS_PLL1_DFS2_DIV2_CLK */
    hal_clock_audiopll1ctl = 14, /* clock id for AUDIO_PLL1_CTL */
    hal_clock_audiopll1 = 15, /* clock id for AUDIO_PLL1_CLK */
    hal_clock_audiopll2ctl = 16, /* clock id for AUDIO_PLL2_CTL */
    hal_clock_audiopll2 = 17, /* clock id for AUDIO_PLL2_CLK */
    hal_clock_videopll1ctl = 18, /* clock id for VIDEO_PLL1_CTL(The clock source dose not exist) */
    hal_clock_videopll1 = 19, /* clock id for VIDEO_PLL1_CLK(The clock source dose not exist) */
    hal_clock_videopll2ctl = 20, /* clock id for VIDEO_PLL2_CTL(The clock source dose not exist) */
    hal_clock_videopll2 = 21, /* clock id for VIDEO_PLL2_CLK(The clock source dose not exist) */
    hal_clock_videopll3ctl = 22, /* clock id for VIDEO_PLL3_CTL(The clock source dose not exist) */
    hal_clock_videopll3 = 23, /* clock id for VIDEO_PLL3_CLK(The clock source dose not exist) */
    hal_clock_encoderpllctl = 24, /* clock id for ENCODER_PLL_CTL */
    hal_clock_encoderplldfs0ctl = 25, /* clock id for ENCODER_PLL_DFS0_CTL */
    hal_clock_encoderplldfs0 = 26, /* clock id for ENCODER_PLL_DFS0_CLK */
    hal_clock_encoderplldfs1ctl = 27, /* clock id for ENCODER_PLL_DFS1_CTL */
    hal_clock_encoderplldfs1 = 28, /* clock id for ENCODER_PLL_DFS1_CLK */
    hal_clock_armpllctl = 29, /* clock id for ARM_PLL_CTL */
    hal_clock_armplldfs0ctl = 30, /* clock id for ARM_PLL_DFS0_CTL */
    hal_clock_armplldfs0 = 31, /* clock id for ARM_PLL_DFS0_CLK */
    hal_clock_armplldfs1ctl = 32, /* clock id for ARM_PLL_DFS1_CTL */
    hal_clock_armplldfs1 = 33, /* clock id for ARM_PLL_DFS1_CLK */
    hal_clock_armplldfs2ctl = 34, /* clock id for ARM_PLL_DFS2_CTL */
    hal_clock_armplldfs2 = 35, /* clock id for ARM_PLL_DFS2_CLK */
    hal_clock_armplldfs3ctl = 36, /* clock id for ARM_PLL_DFS3_CTL */
    hal_clock_armplldfs3 = 37, /* clock id for ARM_PLL_DFS3_CLK */
    hal_clock_drampllctl = 38, /* clock id for DRAM_PLL_CTL */
    hal_clock_drampll = 39, /* clock id for DRAM_PLL_CLK */
    hal_clock_hsiopllctl = 40, /* clock id for HSIO_PLL_CTL */
    hal_clock_hsiopll = 41, /* clock id for HSIO_PLL_CLK */
    hal_clock_ldbpllctl = 42, /* clock id for LDB_PLL_CTL */
    hal_clock_ldbpll = 43, /* clock id for LDB_PLL_CLK */
    hal_clock_extl   = 44, /* clock id for ext1(from components/imx_sm/devices/MIMX943/drivers/fsl_clock.h) */
    hal_clock_ext2   = 45, /* clock id for ext2(from components/imx_sm/devices/MIMX943/drivers/fsl_clock.h) */

    /* clock id for ips */
    hal_clock_adc = HAL_CLOCK_PLATFORM_SOURCE_NUM + 0, /* clock id for ADC */
    hal_clock_busaon = HAL_CLOCK_PLATFORM_SOURCE_NUM + 1, /* clock id for BUS AON */
    hal_clock_can1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 2, /* clock id for CAN1 */
    hal_clock_glitchfilter = HAL_CLOCK_PLATFORM_SOURCE_NUM + 3, /* clock id for GLITCHFILTER */
    hal_clock_gpt1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 4, /* clock id for GPT1 */
    hal_clock_i3c1slow = HAL_CLOCK_PLATFORM_SOURCE_NUM + 5, /* clock id for I3C1 SLOW */
    hal_clock_lpi2c1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 6, /* clock id for LPI2C1 */
    hal_clock_lpi2c2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 7, /* clock id for LPI2C2 */
    hal_clock_lpspi1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 8, /* clock id for LPSPI1 */
    hal_clock_lpspi2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 9, /* clock id for LPSPI2 */
    hal_clock_lptmr1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 10, /* clock id for LPTMR1 */
    hal_clock_lpuart1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 11, /* clock id for LPUART1 */
    hal_clock_lpuart2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 12, /* clock id for LPUART2 */
    hal_clock_m33 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 13, /* clock id for M33 */
    hal_clock_m33systick = HAL_CLOCK_PLATFORM_SOURCE_NUM + 14, /* clock id for M33 SYSTICK */
    hal_clock_pdm = HAL_CLOCK_PLATFORM_SOURCE_NUM + 15, /* clock id for PDM */
    hal_clock_sai1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 16, /* clock id for SAI1 */
    hal_clock_tpm2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 17, /* clock id for TPM2 */
    hal_clock_arma55 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 18, /* clock id for ARM A55 */
    hal_clock_arma55mtrbus  = HAL_CLOCK_PLATFORM_SOURCE_NUM + 19, /* clock id for ARM A55 MTR BUS  */
    hal_clock_arma55periph = HAL_CLOCK_PLATFORM_SOURCE_NUM + 20, /* clock id for ARM A55 PERIPH */
    hal_clock_dramalt = HAL_CLOCK_PLATFORM_SOURCE_NUM + 21, /* clock id for DRAM ALT */
    hal_clock_dramapb = HAL_CLOCK_PLATFORM_SOURCE_NUM + 22, /* clock id for DRAM APB */
    hal_clock_dispapb = HAL_CLOCK_PLATFORM_SOURCE_NUM + 23, /* clock id for DISP APB */
    hal_clock_dispaxi = HAL_CLOCK_PLATFORM_SOURCE_NUM + 24, /* clock id for DISP AXI */
    hal_clock_disppix = HAL_CLOCK_PLATFORM_SOURCE_NUM + 25, /* clock id for DISP PIX */
    hal_clock_hsioacscan480m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 26, /* clock id for HSIO ACSCAN 480M */
    hal_clock_hsioacscan80m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 27, /* clock id for HSIO ACSCAN 80M */
    hal_clock_hsio = HAL_CLOCK_PLATFORM_SOURCE_NUM + 28, /* clock id for HSIO */
    hal_clock_hsiopcieaux = HAL_CLOCK_PLATFORM_SOURCE_NUM + 29, /* clock id for HSIO PCIE AUX */
    hal_clock_hsiopcietest160m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 30, /* clock id for HSIO PCIE TEST 160M */
    hal_clock_hsiopcietest400m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 31, /* clock id for HSIO PCIE TEST 400M */
    hal_clock_hsiopcietest500m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 32, /* clock id for HSIO PCIE TEST 500M */
    hal_clock_hsiousbtest50m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 33, /* clock id for HSIO USB TEST 50M */
    hal_clock_hsiousbtest60m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 34, /* clock id for HSIO USB TEST 60M */
    hal_clock_busm70 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 35, /* clock id for BUS M7 0 */
    hal_clock_m70 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 36, /* clock id for M7 0 */
    hal_clock_m70systick = HAL_CLOCK_PLATFORM_SOURCE_NUM + 37, /* clock id for M7 0 SYSTICK */
    hal_clock_busm71 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 38, /* clock id for BUS M7 1 */
    hal_clock_m71 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 39, /* clock id for M7 1 */
    hal_clock_m71systick = HAL_CLOCK_PLATFORM_SOURCE_NUM + 40, /* clock id for M7 1 SYSTICK */
    hal_clock_busnetcmix = HAL_CLOCK_PLATFORM_SOURCE_NUM + 41, /* clock id for BUS NETCMIX */
    hal_clock_ecat = HAL_CLOCK_PLATFORM_SOURCE_NUM + 42, /* clock id for ECAT */
    hal_clock_enet = HAL_CLOCK_PLATFORM_SOURCE_NUM + 43, /* clock id for ENET */
    hal_clock_enetphytest200m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 44, /* clock id for ENET PHY TEST 200M */
    hal_clock_enetphytest500m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 45, /* clock id for ENET PHY TEST 500M */
    hal_clock_enetphytest667m = HAL_CLOCK_PLATFORM_SOURCE_NUM + 46, /* clock id for ENET PHY TEST 667M */
    hal_clock_enetref = HAL_CLOCK_PLATFORM_SOURCE_NUM + 47, /* clock id for ENET REF */
    hal_clock_enettimer1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 48, /* clock id for ENET TIMER1 */
    hal_clock_enettimer2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 49, /* clock id for ENET TIMER2 */
    hal_clock_enettimer3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 50, /* clock id for ENET TIMER3 */
    hal_clock_flexio3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 51, /* clock id for FLEXIO3 */
    hal_clock_flexio4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 52, /* clock id for FLEXIO4 */
    hal_clock_m33sync = HAL_CLOCK_PLATFORM_SOURCE_NUM + 53, /* clock id for M33 SYNC */
    hal_clock_m33syncsystick = HAL_CLOCK_PLATFORM_SOURCE_NUM + 54, /* clock id for M33 SYNC SYSTICK */
    hal_clock_mac0 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 55, /* clock id for MAC0 */
    hal_clock_mac1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 56, /* clock id for MAC1 */
    hal_clock_mac2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 57, /* clock id for MAC2 */
    hal_clock_mac3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 58, /* clock id for MAC3 */
    hal_clock_mac4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 59, /* clock id for MAC4 */
    hal_clock_mac5 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 60, /* clock id for MAC5 */
    hal_clock_nocapb = HAL_CLOCK_PLATFORM_SOURCE_NUM + 61, /* clock id for NOC APB */
    hal_clock_noc = HAL_CLOCK_PLATFORM_SOURCE_NUM + 62, /* clock id for NOC */
    hal_clock_npuapb = HAL_CLOCK_PLATFORM_SOURCE_NUM + 63, /* clock id for NPU APB */
    hal_clock_npu = HAL_CLOCK_PLATFORM_SOURCE_NUM + 64, /* clock id for NPU */
    hal_clock_ccmcko1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 65, /* clock id for CCM CKO1 */
    hal_clock_ccmcko2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 66, /* clock id for CCM CKO2 */
    hal_clock_ccmcko3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 67, /* clock id for CCM CKO3 */
    hal_clock_ccmcko4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 68, /* clock id for CCM CKO4 */
    hal_clock_biss = HAL_CLOCK_PLATFORM_SOURCE_NUM + 69, /* clock id for BISS */
    hal_clock_buswakeup = HAL_CLOCK_PLATFORM_SOURCE_NUM + 70, /* clock id for BUS WAKEUP */
    hal_clock_ewm = HAL_CLOCK_PLATFORM_SOURCE_NUM + 70, /* clock id for EWM */
    hal_clock_can2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 71, /* clock id for CAN2 */
    hal_clock_can3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 72, /* clock id for CAN3 */
    hal_clock_can4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 73, /* clock id for CAN4 */
    hal_clock_can5 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 74, /* clock id for CAN5 */
    hal_clock_endat21 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 75, /* clock id for ENDAT2 1 */
    hal_clock_endat22 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 76, /* clock id for ENDAT2 2 */
    hal_clock_endat31fast = HAL_CLOCK_PLATFORM_SOURCE_NUM + 77, /* clock id for ENDAT3 1 FAST */
    hal_clock_endat31slow = HAL_CLOCK_PLATFORM_SOURCE_NUM + 78, /* clock id for ENDAT3 1 SLOW */
    hal_clock_flexio1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 79, /* clock id for FLEXIO1 */
    hal_clock_flexio2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 80, /* clock id for FLEXIO2 */
    hal_clock_gpt2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 81, /* clock id for GPT2 */
    hal_clock_gpt3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 82, /* clock id for GPT3 */
    hal_clock_gpt4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 83, /* clock id for GPT4 */
    hal_clock_hiperface1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 84, /* clock id for HIPERFACE1 */
    hal_clock_hiperface1sync = HAL_CLOCK_PLATFORM_SOURCE_NUM + 85, /* clock id for HIPERFACE1 SYNC */
    hal_clock_hiperface2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 86, /* clock id for HIPERFACE2 */
    hal_clock_hiperface2sync = HAL_CLOCK_PLATFORM_SOURCE_NUM + 87, /* clock id for HIPERFACE2 SYNC */
    hal_clock_i3c2slow = HAL_CLOCK_PLATFORM_SOURCE_NUM + 88, /* clock id for I3C2 SLOW */
    hal_clock_lpi2c3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 89, /* clock id for LPI2C3 */
    hal_clock_lpi2c4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 90, /* clock id for LPI2C4 */
    hal_clock_lpi2c5 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 91, /* clock id for LPI2C5 */
    hal_clock_lpi2c6 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 92, /* clock id for LPI2C6 */
    hal_clock_lpi2c7 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 93, /* clock id for LPI2C7 */
    hal_clock_lpi2c8 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 94, /* clock id for LPI2C8 */
    hal_clock_lpspi3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 95, /* clock id for LPSPI3 */
    hal_clock_lpspi4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 96, /* clock id for LPSPI4 */
    hal_clock_lpspi5 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 97, /* clock id for LPSPI5 */
    hal_clock_lpspi6 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 98, /* clock id for LPSPI6 */
    hal_clock_lpspi7 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 99, /* clock id for LPSPI7 */
    hal_clock_lpspi8 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 100, /* clock id for LPSPI8 */
    hal_clock_lptmr2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 101, /* clock id for LPTMR2 */
    hal_clock_lpuart10 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 102, /* clock id for LPUART10 */
    hal_clock_lpuart11 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 103, /* clock id for LPUART11 */
    hal_clock_lpuart12 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 104, /* clock id for LPUART12 */
    hal_clock_lpuart3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 105, /* clock id for LPUART3 */
    hal_clock_lpuart4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 106, /* clock id for LPUART4 */
    hal_clock_lpuart5 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 107, /* clock id for LPUART5 */
    hal_clock_lpuart6 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 108, /* clock id for LPUART6 */
    hal_clock_lpuart7 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 109, /* clock id for LPUART7 */
    hal_clock_lpuart8 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 110, /* clock id for LPUART8 */
    hal_clock_lpuart9 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 111, /* clock id for LPUART9 */
    hal_clock_sai2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 112, /* clock id for SAI2 */
    hal_clock_sai3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 113, /* clock id for SAI3 */
    hal_clock_sai4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 114, /* clock id for SAI4 */
    hal_clock_swotrace = HAL_CLOCK_PLATFORM_SOURCE_NUM + 115, /* clock id for SWO TRACE */
    hal_clock_tpm4 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 116, /* clock id for TPM4 */
    hal_clock_tpm5 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 117, /* clock id for TPM5 */
    hal_clock_tpm6 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 118, /* clock id for TPM6 */
    hal_clock_usbphyburunin = HAL_CLOCK_PLATFORM_SOURCE_NUM + 119, /* clock id for USB PHY BURUNIN */
    hal_clock_usdhc1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 120, /* clock id for USDHC1 */
    hal_clock_usdhc2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 121, /* clock id for USDHC2 */
    hal_clock_usdhc3 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 122, /* clock id for USDHC3 */
    hal_clock_v2xpk = HAL_CLOCK_PLATFORM_SOURCE_NUM + 123, /* clock id for V2X PK */
    hal_clock_wakeupaxi = HAL_CLOCK_PLATFORM_SOURCE_NUM + 124, /* clock id for WAKEUP AXI */
    hal_clock_xspislvroot = HAL_CLOCK_PLATFORM_SOURCE_NUM + 125, /* clock id for XSPI SLV ROOT */
    hal_clock_xspi1 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 126, /* clock id for XSPI1 */
    hal_clock_xspi2 = HAL_CLOCK_PLATFORM_SOURCE_NUM + 127, /* clock id for XSPI2 */
    hal_clock_invalid          = HAL_CLOCK_PLATFORM_SOURCE_NUM + 128
} hal_clk_id_e;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /*__cplusplus */
uint32_t HAL_ClockPlatformGetMuxId(hal_clk_id_e clk_id, hal_clk_id_e pclk_id);
#if defined(__cplusplus)
}
#endif /*__cplusplus */

/*! @} */

#endif /* HAL_CLOCK_PLATFORM_H_ */
