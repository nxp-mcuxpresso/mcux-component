# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.audio_flexcomm_i2s_dma_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_audio.h
                fsl_adapter_flexcomm_i2s.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.audio_sai_dma_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_audio.h
                fsl_adapter_sai.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.audio_sai_edma_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_audio.h
                fsl_adapter_sai.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
