# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.mem_manager)
    mcux_add_source(
        SOURCES fsl_component_mem_manager.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
if (CONFIG_MCUX_COMPONENT_component.mem_manager_legacy)
    mcux_add_source(
        SOURCES fsl_component_mem_manager.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.mem_manager_light)
    mcux_add_source(
        SOURCES fsl_component_mem_manager_light.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.mem_manager_freertos)
    mcux_add_source(
        SOURCES fsl_component_mem_manager_freertos.c
		
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()