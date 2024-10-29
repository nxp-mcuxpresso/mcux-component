# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.hal_clock)
    mcux_add_source(
        SOURCES hal_clock.h
                # TODO please change to relative dir
                hal_clock.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.hal_clock.imx95)
    mcux_add_source(
        SOURCES porting/platform/imx95/hal_clock_platform.h
                porting/platform/imx95/hal_clock_platform.c
    )
    mcux_add_include(
        INCLUDES porting/platform/imx95
    )
endif()
