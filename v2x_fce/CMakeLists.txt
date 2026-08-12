# Copyright 2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

if(CONFIG_MCUX_COMPONENT_component.v2x_fce)
    mcux_add_source(
        SOURCES v2x_fce.c
                v2x_fce.h
                v2x_fce_internal.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
