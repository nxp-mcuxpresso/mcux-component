# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.mx25r_flash)
    mcux_add_source(
        SOURCES mx25r_flash.h
                mx25r_flash.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
