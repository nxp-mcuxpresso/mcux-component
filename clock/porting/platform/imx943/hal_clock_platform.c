/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "hal_clock_platform.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

const hal_clk_id_e mux_tbl[HAL_CLOCK_PLATFORM_MAX_ID][HAL_CLOCK_PLATFORM_MUX_MAX_ID] = {
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid}, /* ext */
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid},
    {hal_clock_invalid, hal_clock_invalid, hal_clock_invalid, hal_clock_invalid}, /* ext2 */

    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* adc */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* bus_aon */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* can1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* glitchfilter */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* gpt1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* i3c1_slow */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpi2c1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpi2c2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpspi1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpspi2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lptmr1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* m33 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* m33_systick */
    {hal_clock_osc24m, hal_clock_audiopll1, hal_clock_audiopll2, hal_clock_ext}, /* pdm */
    {hal_clock_osc24m, hal_clock_audiopll1, hal_clock_audiopll2, hal_clock_ext}, /* sai1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_audiopll1, hal_clock_ext}, /* tpm2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* arm_a55 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* arm_a55_mtr_bus  */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* arm_a55_periph */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* dram_alt */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* dram_apb */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* disp_apb */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* disp_axi */
    {hal_clock_osc24m, hal_clock_audiopll1, hal_clock_audiopll2, hal_clock_syspll1dfs0}, /* disp_pix */
    {hal_clock_osc24m, hal_clock_audiopll1, hal_clock_encoderplldfs1, hal_clock_syspll1dfs0div2}, /* hsio_acscan_480m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* hsio_acscan_80m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* hsio */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* hsio_pcie_aux */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* hsio_pcie_test_160m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* hsio_pcie_test_400m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* hsio_pcie_test_500m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* hsio_usb_test_50m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* hsio_usb_test_60m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* bus_m7_0 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* m7_0 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* m7_0_systick */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* bus_m7_1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* m7_1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* m7_1_systick */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* bus_netcmix */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* ecat */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* enet */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* enet_phy_test_200m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* enet_phy_test_500m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* enet_phy_test_667m */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* enet_ref */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* enet_timer1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* enet_timer2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* enet_timer3 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* flexio3 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* flexio4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* m33_sync */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* m33_sync_systick */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* mac0 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* mac1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* mac2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* mac3 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* mac4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* mac5 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* noc_apb */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* noc */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* npu_apb */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* npu */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_osc32k, hal_clock_audiopll1}, /* ccm_cko1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_audiopll2}, /* ccm_cko2 */
    {hal_clock_osc24m, hal_clock_osc32k, hal_clock_syspll1dfs1, hal_clock_encoderplldfs0}, /* ccm_cko3 */
    {hal_clock_osc24m, hal_clock_encoderplldfs1, hal_clock_syspll1dfs1, hal_clock_audiopll2}, /* ccm_cko4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* biss */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* bus_wakeup */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* can2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* can3 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* can4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* can5 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_encoderplldfs1}, /* endat2_1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_encoderplldfs1}, /* endat2_2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* endat3_1_fast */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* endat3_1_slow */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* flexio1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* flexio2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* gpt2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* gpt3 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* gpt4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_encoderplldfs0}, /* hiperface1 */
    {hal_clock_osc24m, hal_clock_osc32k, hal_clock_syspll1dfs1div2, hal_clock_encoderplldfs0}, /* hiperface1_sync */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_encoderplldfs0}, /* hiperface2 */
    {hal_clock_osc24m, hal_clock_osc32k, hal_clock_syspll1dfs1div2, hal_clock_encoderplldfs0}, /* hiperface2_sync */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* i3c2_slow */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpi2c3 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpi2c4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpi2c5 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpi2c6 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpi2c7 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpi2c8 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpspi3 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpspi4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpspi5 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpspi6 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpspi7 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lpspi8 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* lptmr2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart10 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart11 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart12 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart3 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart5 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart6 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart7 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart8 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_encoderplldfs0, hal_clock_fro}, /* lpuart9 */
    {hal_clock_osc24m, hal_clock_audiopll1, hal_clock_audiopll2, hal_clock_ext}, /* sai2 */
    {hal_clock_osc24m, hal_clock_audiopll1, hal_clock_audiopll2, hal_clock_ext}, /* sai3 */
    {hal_clock_osc24m, hal_clock_audiopll1, hal_clock_audiopll2, hal_clock_ext}, /* sai4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* swo_trace */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_audiopll1, hal_clock_ext}, /* tpm4 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_audiopll1, hal_clock_ext}, /* tpm5 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_audiopll1, hal_clock_ext}, /* tpm6 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0div2, hal_clock_syspll1dfs1div2, hal_clock_fro}, /* usb_phy_burunin */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* usdhc1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* usdhc2 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* usdhc3 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* v2x_pk */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* wakeup_axi */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* xspi_slv_root */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* xspi1 */
    {hal_clock_osc24m, hal_clock_syspll1dfs0, hal_clock_syspll1dfs1, hal_clock_syspll1dfs2}, /* xspi2 */
};

/*******************************************************************************
 * API
 ******************************************************************************/
uint32_t HAL_ClockPlatformGetMuxId(hal_clk_id_e clk_id, hal_clk_id_e pclk_id)
{
    uint32_t mux_id = 0;
    while (mux_id < HAL_CLOCK_PLATFORM_MUX_MAX_ID)
    {
        if (mux_tbl[clk_id][mux_id] == pclk_id)
        {
            break;
        }
        mux_id++;
    }
    return mux_id;
}
