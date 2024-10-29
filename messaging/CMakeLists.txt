# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.messaging)
    mcux_add_source(
        SOURCES fsl_component_messaging.h
                fsl_component_messaging.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()