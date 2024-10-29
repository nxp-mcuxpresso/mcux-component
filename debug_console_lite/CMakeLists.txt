# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_utility.debug_console_lite)

    mcux_add_source(
        SOURCES fsl_debug_console.h
                fsl_debug_console.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()