# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.led)

    mcux_add_source(
        SOURCES fsl_component_led.h
                fsl_component_led.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()