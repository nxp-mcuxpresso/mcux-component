# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.mt48lc2m32b2)
    mcux_add_source(
        SOURCES fsl_sdram.c
                fsl_sdram.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
