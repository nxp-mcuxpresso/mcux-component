# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.slcd_engine)
    mcux_add_source(
        SOURCES ./slcd_engine.c
                ./slcd_engine.h
                ./slcd_panel.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.slcd_lcd_s401m16kr)
    mcux_add_source(
        SOURCES ./LCD-S401M16KR/LCD-S401M16KR.c
                ./LCD-S401M16KR/LCD-S401M16KR.h
    )
    mcux_add_include(
        INCLUDES ./LCD-S401M16KR
    )
    mcux_add_configuration(
        CC "-DSLCD_PANEL_LCD_S401M16KR_H"
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.slcd_gdh_1247wp)
    mcux_add_source(
        SOURCES ./GDH-1247WP/GDH-1247WP.c
                ./GDH-1247WP/GDH-1247WP.h
    )
    mcux_add_include(
        INCLUDES ./GDH-1247WP
    )
    mcux_add_configuration(
        CC "-DSLCD_PANEL_GDH_1247WP_H"
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.slcd_od_6010)
    mcux_add_source(
        SOURCES ./OD-6010/OD-6010.c
                ./OD-6010/OD-6010.h
    )
    mcux_add_include(
        INCLUDES ./OD-6010
    )
    mcux_add_configuration(
        CC "-DSLCD_PANEL_OD_6010_H"
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.slcd_ftp12557)
    mcux_add_source(
        SOURCES ./FTP12557/FTP12557.c
                ./FTP12557/FTP12557.h
    )
    mcux_add_include(
        INCLUDES ./FTP12557
    )
    mcux_add_configuration(
        CC "-DSLCD_PANEL_FTP12557_H"
    )
endif()
