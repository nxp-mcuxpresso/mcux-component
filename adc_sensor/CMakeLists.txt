# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.cns_adc_sensor_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_adc_sensor.h
                fsl_adapter_cns_adc_sensor.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.lpadc_sensor_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_adc_sensor.h
                fsl_adapter_lpadc_sensor.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
