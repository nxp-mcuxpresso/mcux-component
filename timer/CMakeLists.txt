# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.ctimer_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_ctimer.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_CTIMER=1"
        CC "-DTIMER_PORT_TYPE_CTIMER=1"
        CX "-DTIMER_PORT_TYPE_CTIMER=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.ftm_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_ftm.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_FTM=1"
        CC "-DTIMER_PORT_TYPE_FTM=1"
        CX "-DTIMER_PORT_TYPE_FTM=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.gpt_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_gpt.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_GPT=1"
        CC "-DTIMER_PORT_TYPE_GPT=1"
        CX "-DTIMER_PORT_TYPE_GPT=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.lpit_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_lpit.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_LPIT=1"
        CC "-DTIMER_PORT_TYPE_LPIT=1"
        CX "-DTIMER_PORT_TYPE_LPIT=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.lptmr_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_lptmr.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_LPTMR=1"
        CC "-DTIMER_PORT_TYPE_LPTMR=1"
        CX "-DTIMER_PORT_TYPE_LPTMR=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.mrt_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_mrt.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_MRT=1"
        CC "-DTIMER_PORT_TYPE_MRT=1"
        CX "-DTIMER_PORT_TYPE_MRT=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.pit_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_pit.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_PIT=1"
        CC "-DTIMER_PORT_TYPE_PIT=1"
        CX "-DTIMER_PORT_TYPE_PIT=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.tpm_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_tpm.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_TPM=1"
        CC "-DTIMER_PORT_TYPE_TPM=1"
        CX "-DTIMER_PORT_TYPE_TPM=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.dsc_pit_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_dsc_pit.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_DSC_PIT=1"
        CC "-DTIMER_PORT_TYPE_DSC_PIT=1"
        CX "-DTIMER_PORT_TYPE_DSC_PIT=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.ostimer_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_timer.h
                fsl_adapter_ostimer.c
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_configuration(
        AS "-DTIMER_PORT_TYPE_OSTIMER=1"
        CC "-DTIMER_PORT_TYPE_OSTIMER=1"
        CX "-DTIMER_PORT_TYPE_OSTIMER=1"
    )
endif()
