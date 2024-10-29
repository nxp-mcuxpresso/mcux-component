# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.sm_clock)
    mcux_add_source(
        SOURCES clock/sm_clock.h
                clock/sm_clock.c
    )
    mcux_add_include(
        INCLUDES clock
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.sm_pinctrl)
    mcux_add_source(
        SOURCES pinctrl/sm_pinctrl.h
                pinctrl/sm_pinctrl.c
    )
    mcux_add_include(
        INCLUDES pinctrl
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.sm_power)
    mcux_add_source(
        SOURCES power/sm_power.h
                power/sm_power.c
    )
    mcux_add_include(
        INCLUDES power
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.sm.imx95_cm33)
    mcux_add_source(
        SOURCES sm_platform.h
                # TODO please change to relative dir
                sm_platform.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.sm.imx95_cm7)
    mcux_add_source(
        SOURCES porting/platform/imx95/cm7/sm_platform.h
                porting/platform/imx95/cm7/sm_platform.c
    )
    mcux_add_include(
        INCLUDES porting/platform/imx95/cm7
    )
endif()
