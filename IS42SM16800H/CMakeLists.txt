# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.IS42SM16800H)
    mcux_add_source(
        SOURCES fsl_sdram.c
                fsl_sdram.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
