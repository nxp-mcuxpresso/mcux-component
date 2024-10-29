# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.pit_time_stamp_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_time_stamp.h
                fsl_adapter_pit_time_stamp.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.lpit_time_stamp_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_time_stamp.h
                fsl_adapter_lpit_time_stamp.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.lptmr_time_stamp_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_time_stamp.h
                fsl_adapter_lptmr_time_stamp.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.ctimer_time_stamp_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_time_stamp.h
                fsl_adapter_ctimer_time_stamp.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.ostimer_time_stamp_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_time_stamp.h
                fsl_adapter_ostimer_time_stamp.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.gpt_time_stamp_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_time_stamp.h
                fsl_adapter_gpt_time_stamp.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
