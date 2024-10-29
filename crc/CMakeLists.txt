# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.crc_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_crc.h
                fsl_adapter_crc.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.lpc_crc_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_crc.h
                fsl_adapter_lpc_crc.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.software_crc_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_crc.h
                fsl_adapter_software_crc.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
