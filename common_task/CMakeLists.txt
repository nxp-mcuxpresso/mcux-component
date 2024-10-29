# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.common_task)
    mcux_add_source(
        SOURCES fsl_component_common_task.h
                fsl_component_common_task.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()