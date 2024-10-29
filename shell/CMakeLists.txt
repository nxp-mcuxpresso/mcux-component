# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_utility.shell)

    mcux_add_source(
        SOURCES fsl_shell.h
                fsl_shell.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()