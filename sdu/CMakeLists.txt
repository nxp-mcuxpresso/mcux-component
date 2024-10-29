# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.sdu_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_sdu.h
                fsl_adapter_sdu.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
