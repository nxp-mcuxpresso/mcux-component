# Copyright 2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.coremark)
    mcux_add_source(
        SOURCES core_list_join.c
                core_main.c
                core_matrix.c
                core_state.c
                core_util.c
                coremark.h
                simple/core_portme.c
                simple/core_portme.h
    )
    mcux_add_include(
        INCLUDES ./
        INCLUDES ./simple
    )
endif()