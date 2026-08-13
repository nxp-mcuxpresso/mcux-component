# Copyright 2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

if(CONFIG_MCUX_COMPONENT_component.v2x_base)
    mcux_add_source(
        SOURCES v2x_base.c
                v2x_base.h
                v2x_base_internal.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
