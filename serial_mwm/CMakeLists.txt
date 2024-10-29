# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.serial_mwm)
    mcux_add_source(
        SOURCES serial_mwm.h
                serial_mwm.c
                serial_mwm_port.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.serial_mwm_usart)
    mcux_add_source(
        SOURCES serial_mwm_usart.c
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.serial_mwm_lpuart)
    mcux_add_source(
        SOURCES serial_mwm_lpuart.c
    )
endif()
