# Copyright 2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.datetime)
    mcux_add_source(
        SOURCES fsl_datetime.h
                fsl_datetime.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
