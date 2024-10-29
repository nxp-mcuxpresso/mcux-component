# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.ele_base_api)
    mcux_add_source(
        SOURCES fsl_ele_base_api.c
                fsl_ele_base_api.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
