# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.sm.crc)
    mcux_add_source(
        SOURCES crc.h
                crc.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
