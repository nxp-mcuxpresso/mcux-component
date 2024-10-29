# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.smbus)
    mcux_add_source(
        SOURCES fsl_smbus.c
                # TODO please change to relative dir
                fsl_smbus.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.smbus_event)
    mcux_add_source(
        SOURCES event/fsl_smbus_event.h
                # TODO please change to relative dir
                event/fsl_smbus_event.c
    )
    mcux_add_include(
        INCLUDES event
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.smbus.port_pn_i2c)
    mcux_add_source(
        SOURCES port/pn_i2c/fsl_smbus_port.h
                # TODO please change to relative dir
                port/pn_i2c/fsl_smbus_port.c
    )
    mcux_add_include(
        INCLUDES port/pn_i2c
    )
endif()
