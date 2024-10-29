# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.sm.smt)
    mcux_add_source(
        SOURCES smt.h
                smt.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
