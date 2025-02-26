# Copyright 2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.lce)

    mcux_add_source(
        SOURCES fsl_component_lce.h
                fsl_component_lce.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()