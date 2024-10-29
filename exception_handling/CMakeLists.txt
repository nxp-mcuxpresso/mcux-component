# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.exception_handling_cm7)
    mcux_add_source(
        SOURCES fsl_component_exception_handling.h
                cm7/fsl_component_exception_handling.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()