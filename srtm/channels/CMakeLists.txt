# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.rpmsg_endpoint)
    mcux_add_source(
        SOURCES srtm_rpmsg_endpoint.h
                srtm_rpmsg_endpoint.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()


if (CONFIG_MCUX_COMPONENT_component.mu_endpoint)
    mcux_add_source(
        SOURCES srtm_mu_endpoint.h
                srtm_mu_endpoint.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.rpmsg_emu_endpoint)
    mcux_add_source(
        SOURCES srtm_rpmsg_emu.h
                srtm_rpmsg_emu.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()
