# Copyright 2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.gen_hal)
    mcux_add_include(
        INCLUDES ./
    )

    mcux_add_source(
        SOURCES
            ./pinctrl_defs.h
            ./pinctrl_soc.h
            ./flexspi_clock_setup.c
            ./flexspi_clock_setup.h
            ./soc.h
    )
endif()
