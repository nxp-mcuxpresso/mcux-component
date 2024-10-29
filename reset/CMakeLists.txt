# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.reset_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_reset.h
                fsl_adapter_reset.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
