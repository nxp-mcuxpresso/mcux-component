# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.pwm_ctimer_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_pwm.h
                fsl_adapter_pwm_ctimer.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.pwm_ftm_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_pwm.h
                fsl_adapter_pwm_ftm.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.pwm_tpm_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_pwm.h
                fsl_adapter_pwm_tpm.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
