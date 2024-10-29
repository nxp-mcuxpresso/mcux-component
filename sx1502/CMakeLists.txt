# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.sx1502)
    mcux_add_source(
        SOURCES fsl_sx1502.h
                fsl_sx1502.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
