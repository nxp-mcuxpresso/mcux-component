# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.eeprom_emulation)

    mcux_add_source(
        SOURCES
            fsl_eeprom_emulation.h
            fsl_eeprom_emulation.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()
