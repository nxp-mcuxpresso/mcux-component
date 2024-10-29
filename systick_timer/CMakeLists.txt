# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.systick_timer)
    mcux_add_source(
        SOURCES fsl_systick_timer.h
                # TODO please change to relative dir
                fsl_systick_timer.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
