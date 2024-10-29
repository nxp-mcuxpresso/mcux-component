# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.hal_power)
    mcux_add_source(
        SOURCES hal_power.h
                hal_power.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.hal_power.imx95)
    mcux_add_source(
        SOURCES porting/platform/imx95/hal_power_platform.h
    )
    mcux_add_include(
        INCLUDES porting/platform/imx95
    )
endif()
