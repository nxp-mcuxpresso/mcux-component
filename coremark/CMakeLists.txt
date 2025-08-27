# Copyright 2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.coremark)
    mcux_add_source(
        SOURCES core_list_join.c
                core_main.c
                core_matrix.c
                core_state.c
                core_util.c
                coremark.h
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_source(
        SOURCES porting/imxrt700/core_portme.c
        SOURCES porting/imxrt700/core_portme.h
        DEVICE_IDS MIMXRT798S
    )
    mcux_add_include(
        INCLUDES ./porting/imxrt700
        DEVICE_IDS MIMXRT798S
    )
    mcux_add_source(
        SOURCES porting/imxrt2660/core_portme.c
        SOURCES porting/imxrt2660/core_portme.h
        DEVICE_IDS MIMXRT2661xxxxx MIMXRT2662xxxxx
    )
    mcux_add_include(
        INCLUDES ./porting/imxrt2660
        DEVICE_IDS MIMXRT2661xxxxx MIMXRT2662xxxxx
    )
endif()
