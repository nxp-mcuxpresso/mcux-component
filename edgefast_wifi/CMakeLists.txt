# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_middleware.edgefast_wifi_nxp)

    mcux_add_source(
        SOURCES include/wpl.h
                source/wpl_nxp.c
    )

    mcux_add_include(
        INCLUDES include
    )

endif()
