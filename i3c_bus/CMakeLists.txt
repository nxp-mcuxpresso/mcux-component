# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.i3c_bus)
    mcux_add_source(
        SOURCES fsl_component_i3c.h
                fsl_component_i3c.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.i3c_bus_adapter)
    mcux_add_source(
        SOURCES fsl_component_i3c_adapter.h
                fsl_component_i3c_adapter.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
