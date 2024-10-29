# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.button)

    mcux_add_source(
        SOURCES fsl_component_button.h
                fsl_component_button.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()