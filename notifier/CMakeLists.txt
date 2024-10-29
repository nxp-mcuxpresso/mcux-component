# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_utility.notifier)
    mcux_add_source(
        SOURCES fsl_notifier.c
                # TODO please change to relative dir
                fsl_notifier.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
