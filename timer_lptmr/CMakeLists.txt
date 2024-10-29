# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.timer_lptmr)
    mcux_add_source(
        SOURCES fsl_timer.h
                # TODO please change to relative dir
                fsl_timer.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
