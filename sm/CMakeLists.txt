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

if (CONFIG_MCUX_COMPONENT_component.sm_reset)
    mcux_add_source(
        SOURCES reset/sm_reset.h
                reset/sm_reset.c
    )
    mcux_add_include(
        INCLUDES reset
    )
endif()



if (CONFIG_MCUX_COMPONENT_component.sm.imx943_cm33_core0)
    mcux_add_source(
        SOURCES porting/platform/imx943/cm33_core0/sm_platform.h
                porting/platform/imx943/cm33_core0/sm_platform.c
    )
    mcux_add_include(
        INCLUDES porting/platform/imx943/cm33_core0
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.sm.imx943_cm33_core1)
    mcux_add_source(
        SOURCES porting/platform/imx943/cm33_core1/sm_platform.h
                porting/platform/imx943/cm33_core1/sm_platform.c
    )
    mcux_add_include(
        INCLUDES porting/platform/imx943/cm33_core1
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.sm.imx943_cm7_core0)
    mcux_add_source(
        SOURCES porting/platform/imx943/cm7_core0/sm_platform.h
                porting/platform/imx943/cm7_core0/sm_platform.c
    )
    mcux_add_include(
        INCLUDES porting/platform/imx943/cm7_core0
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.sm.imx943_cm7_core1)
    mcux_add_source(
        SOURCES porting/platform/imx943/cm7_core1/sm_platform.h
                porting/platform/imx943/cm7_core1/sm_platform.c
    )
    mcux_add_include(
        INCLUDES porting/platform/imx943/cm7_core1
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.sm.imx95_cm33)
    mcux_add_source(
        SOURCES porting/platform/imx95/cm33/sm_platform.h
                # TODO please change to relative dir
                porting/platform/imx95/cm33/sm_platform.c
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
