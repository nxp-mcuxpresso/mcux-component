# Copyright 2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.gen_hal)
    mcux_add_macro(
        TARGETS flash_debug flash_release
        CC  "CONFIG_FLASH_MCUX_FLEXSPI_XIP=1 CONFIG_XIP=1"
    )

    mcux_add_include(
        INCLUDES ./
    )

    mcux_add_source(
        SOURCES
            ./devicetree_generated.h
    )
endif()
