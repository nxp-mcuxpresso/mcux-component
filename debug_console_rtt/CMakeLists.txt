# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_utility.debug_console_rtt)
    mcux_add_source(
        SOURCES fsl_debug_console.c
                # TODO please change to relative dir
                fsl_debug_console.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
