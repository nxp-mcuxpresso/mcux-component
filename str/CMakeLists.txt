# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_utility.str)

    mcux_add_source(
        SOURCES fsl_str.h
                fsl_str.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()