# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.panic)
    mcux_add_source(
        SOURCES fsl_component_panic.h
                fsl_component_panic.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
