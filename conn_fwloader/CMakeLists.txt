# Copyright 2024-2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.conn_fwloader)
    mcux_add_source(
        SOURCES include/api_tree_root.h
                include/fsl_loader.h
                include/fsl_loader_utils.h
                include/fusemap.h
                include/life_cycle.h
                include/nboot_rom_api_table.h
                fsl_loader.c
                fsl_loader_utils.c
                nboot_hal.c
                life_cycle.c
                readme.txt
    )
    mcux_add_include(
        INCLUDES include
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.conn_fwloader.fw_bin)
    mcux_add_source(
        SOURCES fw_bin/rw61x/rw61x_sb_wifi_a2.bin
                fw_bin/rw61x/rw61x_sb_ble_a2.bin
                fw_bin/rw61x/rw61x_sb_ble_15d4_combo_a2.bin
                fw_bin/rw61x/rw61x_sb_wifi_a2_compressed.bin
                fw_bin/rw61x/rw61x_sb_ble_a2_compressed.bin
                fw_bin/rw61x/rw61x_sb_ble_15d4_combo_a2_compressed.bin
                fw_bin/script/wlan_gen_fw_incs.py
                fw_bin/inc/rw61x/*.inc
                fw_bin/rw61x/rw61x_cpu1.c
                fw_bin/rw61x/rw61x_cpu2.c
                fw_bin/inc/iw610x/*.inc
                fw_bin/iw610x/sduart_iw610_se.h
                fw_bin/iw610x/sd_iw610_se.h
                fw_bin/iw610x/uart_iw610_se.h
                fw_bin/iw610x/iw610_cpu12_se.c
                fw_bin/iw610x/iw610_cpu2_se.c
                fw_bin/iw610x/iw610_cpu1_se.c
                fw_bin/inc/8987/*.inc
                fw_bin/8987/sduart8987_wlan_bt.h
                fw_bin/8987/sd8987_wlan.h
                fw_bin/8987/uart8987_bt.h
                fw_bin/8987/8987_cpu12.c
                fw_bin/8987/8987_cpu1.c
                fw_bin/8987/8987_cpu2.c
                fw_bin/inc/IW416/*.inc
                fw_bin/IW416/sduartIW416_wlan_bt.h
                fw_bin/IW416/sdIW416_wlan.h
                fw_bin/IW416/uartIW416_bt.h
                fw_bin/IW416/IW416_cpu12.c
                fw_bin/IW416/IW416_cpu1.c
                fw_bin/IW416/IW416_cpu2.c
                fw_bin/inc/nw61x/*.inc
                fw_bin/nw61x/sduart_nw61x_se.h
                fw_bin/nw61x/sd_nw61x_se.h
                fw_bin/nw61x/uart_nw61x_se.h
                fw_bin/nw61x/sduart_nw61x_zb_dual_pan_se.h
                fw_bin/nw61x/uart_nw61x_zb_dual_pan_se.h
                fw_bin/nw61x/nw61x_cpu12_se.c
                fw_bin/nw61x/nw61x_cpu1_se.c
                fw_bin/nw61x/nw61x_cpu2_se.c
    )
    mcux_add_include(
        INCLUDES fw_bin
                 fw_bin/inc/rw61x
                 fw_bin/rw61x
                 fw_bin/inc/iw610x
                 fw_bin/iw610x
                 fw_bin/inc/8987
                 fw_bin/8987
                 fw_bin/inc/IW416
                 fw_bin/IW416
                 fw_bin/inc/nw61x
                 fw_bin/nw61x
                 fw_bin/script
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.conn_fwloader_ns)
    mcux_add_source(
        SOURCES include/fsl_loader.h
    )
    mcux_add_include(
        INCLUDES include
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.conn_fwloader.monolithic)
    mcux_add_macro(
        CC  "-DgPlatformMonolithicApp_d=1"
        CX  "-DgPlatformMonolithicApp_d=1"
    )

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/examples/_boards/${board}
        PREINCLUDE TRUE
        SOURCES monolithic_config.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.wifi.libcsi_cm33)
    mcux_add_library(
        LIBS cm33/armgcc/libcsi.a
        TOOLCHAINS armgcc
        CORES cm33
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi
    )

    mcux_add_library(
        LIBS cm33/iar/libcsi.a
        TOOLCHAINS iar
        CORES cm33
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi
    )

    mcux_add_library(
        LIBS cm33/mdk/libcsi.lib
        TOOLCHAINS mdk
        CORES cm33
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi
    )

    mcux_add_library(
        LIBS cm33/mcux/libcsi.a
        TOOLCHAINS mcux
        CORES cm33
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.wifi.libcsi_cm7.cm7f)
    mcux_add_library(
        LIBS cm7f/armgcc/libcsi.a
        TOOLCHAINS armgcc
        CORES cm7f
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi/cm7
    )

    mcux_add_library(
        LIBS cm7f/iar/libcsi.a
        TOOLCHAINS iar
        CORES cm7f
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi/cm7
    )

    mcux_add_library(
        LIBS cm7f/mdk/libcsi.lib
        TOOLCHAINS mdk
        CORES cm7f
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi/cm7
    )

    mcux_add_library(
        LIBS cm7f/mcux/libcsi.a
        TOOLCHAINS mcux
        CORES cm7f
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi/cm7
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.wifi.libcsi_cm7.cm7f_sta_20M_only)
    mcux_add_library(
        LIBS cm7f_sta_20M_only/armgcc/libcsi.a
        TOOLCHAINS armgcc
        CORES cm7f
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi/cm7
    )

    mcux_add_library(
        LIBS cm7f_sta_20M_only/iar/libcsi.a
        TOOLCHAINS iar
        CORES cm7f
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi/cm7
    )

    mcux_add_library(
        LIBS cm7f_sta_20M_only/mdk/libcsi.lib
        TOOLCHAINS mdk
        CORES cm7f
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi/cm7
    )

    mcux_add_library(
        LIBS cm7f_sta_20M_only/mcux/libcsi.a
        TOOLCHAINS mcux
        CORES cm7f
        BASE_PATH ${SdkRootDirPath}/components/conn_fwloader/fw_bin/lib/libcsi/cm7
    )

endif()
