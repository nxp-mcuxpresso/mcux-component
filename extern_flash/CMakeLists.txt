# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.eeprom_AT45DBxxxx)
    mcux_add_source(
        SOURCES fsl_adapter_eeprom.h
                fsl_adapter_eeprom_AT45DBxxxx.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.eeprom_InternalFlash)
    mcux_add_source(
        SOURCES fsl_adapter_eeprom.h
                fsl_adapter_eeprom_InternalFlash.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.eeprom_AT25Fxxxx)
    mcux_add_source(
        SOURCES fsl_adapter_eeprom.h
                fsl_adapter_eeprom_AT25Fxxxx.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
