# Copyright 2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.eeprom_emulation)

    mcux_add_source(
        SOURCES
            eeprom_emulation.h
            eeprom_emulation.c
            eeprom_emulation_platform.h
            eeprom_emulation_platform_private.h
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.eeprom_emulation_flashapi_k4)

    mcux_add_source(
        SOURCES
            flashapi/eeprom_emulation_flashapi_k4.c
    )

    mcux_add_include(
        INCLUDES ./flashapi
    )

endif()
