# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.tx_pwr_limits)
    mcux_add_source(
        SOURCES AzureWave/tx_pwr_limits/wlan_txpwrlimit_cfg_WW.h
                AzureWave/tx_pwr_limits/wlan_txpwrlimit_cfg_WW_rw610.h
                Murata/tx_pwr_limits/wlan_txpwrlimit_cfg_murata_1XK_WW.h
                Murata/tx_pwr_limits/wlan_txpwrlimit_cfg_murata_1ZM_WW.h
                Murata/tx_pwr_limits/wlan_txpwrlimit_cfg_murata_2DS_WW.h
                Murata/tx_pwr_limits/wlan_txpwrlimit_cfg_murata_2EL_WW.h
                Murata/tx_pwr_limits/wlan_txpwrlimit_cfg_murata_NH.h
                u-blox/tx_pwr_limits/wlan_txpwrlimit_cfg_jody_w5_WW.h
    )
    mcux_add_include(
        INCLUDES AzureWave/tx_pwr_limits
                 Murata/tx_pwr_limits u-blox/tx_pwr_limits
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.config)
    mcux_add_source(
        SOURCES incl/wifi_bt_module_config.h
    )
    mcux_add_include(
        INCLUDES incl
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_rw61x)
    mcux_add_macro(
        "-DWIFI_BOARD_RW610"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_frdmrw61x)
mcux_add_macro(
        "-DWIFI_BOARD_FRDMRW61X"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_aw_nm191ma)
     mcux_add_macro(
        "-DWIFI_88W8801_BOARD_AW_NM191MA"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_aw_nm191_usd)
     mcux_add_macro(
        "-DWIFI_88W8801_BOARD_AW_NM191_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ds_m2)
     mcux_add_macro(
        "-DWIFI_88W8801_BOARD_MURATA_2DS_M2"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2ds_usd)
     mcux_add_macro(
        "-DWIFI_88W8801_BOARD_MURATA_2DS_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_ubx_lily_w1_usd)
     mcux_add_macro(
        "-DWIFI_88W8801_BOARD_UBX_LILY_W1_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_aw_cm358ma)
     mcux_add_macro(
        "-DWIFI_88W8987_BOARD_AW_CM358MA"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_aw_cm358_usd)
     mcux_add_macro(
        "-DWIFI_88W8987_BOARD_AW_CM358_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_1zm_m2)
     mcux_add_macro(
        "-DWIFI_88W8987_BOARD_MURATA_1ZM_M2"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_1zm_usd)
     mcux_add_macro(
        "-DWIFI_88W8987_BOARD_MURATA_1ZM_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_ubx_jody_w2_usd)
     mcux_add_macro(
        "-DWIFI_88W8987_BOARD_UBX_JODY_W2_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_ubx_jody_w5_m2)
     mcux_add_macro(
        "-DWIFI_AW611_BOARD_UBX_JODY_W5_M2"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_ubx_jody_w5_usd)
     mcux_add_macro(
        "-DWIFI_AW611_BOARD_UBX_JODY_W5_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_aw_am457ma)
     mcux_add_macro(
        "-DWIFI_IW416_BOARD_AW_AM457MA"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_aw_am457_usd)
     mcux_add_macro(
        "-DWIFI_IW416_BOARD_AW_AM457_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_aw_am510ma)
     mcux_add_macro(
        "-DWIFI_IW416_BOARD_AW_AM510MA"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_aw_am510_usd)
     mcux_add_macro(
        "-DWIFI_IW416_BOARD_AW_AM510_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_1xk_m2)
     mcux_add_macro(
        "-DWIFI_IW416_BOARD_MURATA_1XK_M2"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_1xk_usd)
     mcux_add_macro(
        "-DWIFI_IW416_BOARD_MURATA_1XK_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_ubx_maya_w1_usd)
     mcux_add_macro(
        "-DWIFI_IW416_BOARD_UBX_MAYA_W1_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2dl_m2)
     mcux_add_macro(
        "-DWIFI_IW611_BOARD_MURATA_2DL_M2"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2dl_usd)
     mcux_add_macro(
        "-DWIFI_IW611_BOARD_MURATA_2DL_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2el_m2)
     mcux_add_macro(
        "-DWIFI_IW612_BOARD_MURATA_2EL_M2"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_murata_2el_usd)
     mcux_add_macro(
        "-DWIFI_IW612_BOARD_MURATA_2EL_USD"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.wifi_bt_module.board_k32w061_transceiver)
     mcux_add_macro(
        "-DK32W061_TRANSCEIVER"
    )
endif()
