# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.gpio_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_gpio.h
                fsl_adapter_gpio.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.lpc_gpio_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_gpio.h
                fsl_adapter_lpc_gpio.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.igpio_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_gpio.h
                fsl_adapter_igpio.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.gint_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_gpio.h
                fsl_adapter_gint.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.rt_gpio_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_gpio.h
                fsl_adapter_rt_gpio.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.dsc_gpio_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_gpio.h
                fsl_adapter_dsc_gpio.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.rgpio_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_gpio.h
                fsl_adapter_rgpio.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()