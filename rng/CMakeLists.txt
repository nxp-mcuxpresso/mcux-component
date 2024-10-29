# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.rng_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_rng.h
                fsl_adapter_rng.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.rnga_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_rng.h
                fsl_adapter_rnga.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.trng_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_rng.h
                fsl_adapter_trng.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.JNrng_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_rng.h
                fsl_adapter_jn_rng.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.software_rng_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_rng.h
                fsl_adapter_software_rng.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()