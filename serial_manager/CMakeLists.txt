# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.serial_manager)
    mcux_add_include(
        INCLUDES .
    )

    mcux_add_source(
        SOURCES
            fsl_component_serial_manager.h
            fsl_component_serial_port_internal.h
            fsl_component_serial_manager.c
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.serial_manager_uart)
    mcux_add_include(
        INCLUDES .
    )

    mcux_add_source(
        SOURCES
            fsl_component_serial_port_uart.h
            fsl_component_serial_port_uart.c
    )
    
    mcux_add_configuration(
        AS "-DSERIAL_PORT_TYPE_UART=1"
        CC "-DSERIAL_PORT_TYPE_UART=1"
        CX "-DSERIAL_PORT_TYPE_UART=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.serial_manager_spi)
    mcux_add_include(
        INCLUDES .
    )

    mcux_add_source(
        SOURCES
            fsl_component_serial_port_spi.h
            fsl_component_serial_port_spi.c
    )
    
    mcux_add_configuration(
        AS "-DSERIAL_PORT_TYPE_SPI=1 -DSERIAL_PORT_TYPE_SPI_MASTER=1 -DSERIAL_PORT_TYPE_SPI_SLAVE=1 -DSERIAL_MANAGER_NON_BLOCKING_MODE=1"
        CC "-DSERIAL_PORT_TYPE_SPI=1 -DSERIAL_PORT_TYPE_SPI_MASTER=1 -DSERIAL_PORT_TYPE_SPI_SLAVE=1 -DSERIAL_MANAGER_NON_BLOCKING_MODE=1"
        CX "-DSERIAL_PORT_TYPE_SPI=1 -DSERIAL_PORT_TYPE_SPI_MASTER=1 -DSERIAL_PORT_TYPE_SPI_SLAVE=1 -DSERIAL_MANAGER_NON_BLOCKING_MODE=1"
    )
        
endif()

if (CONFIG_MCUX_COMPONENT_component.serial_manager_usb_cdc)
    mcux_add_include(
        INCLUDES .
        usb_cdc_adapter
    )

    mcux_add_source(
        SOURCES
            fsl_component_serial_port_usb.h
            fsl_component_serial_port_usb.c
            usb_cdc_adapter/usb_device_descriptor.h
            usb_cdc_adapter/usb_device_descriptor.c
            usb_cdc_adapter/usb_device_config.h
    )
    
    mcux_add_configuration(
        AS "-DSERIAL_PORT_TYPE_USBCDC=1 -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING -DUSB_DEVICE_CONFIG_CDC_ACM=1"
        CC "-DSERIAL_PORT_TYPE_USBCDC=1 -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING -DUSB_DEVICE_CONFIG_CDC_ACM=1"
        CX "-DSERIAL_PORT_TYPE_USBCDC=1 -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING -DUSB_DEVICE_CONFIG_CDC_ACM=1"
    )
        
endif()

if (CONFIG_MCUX_COMPONENT_component.serial_manager_swo)
    mcux_add_include(
        INCLUDES .
    )

    mcux_add_source(
        SOURCES
            fsl_component_serial_port_swo.h
            fsl_component_serial_port_swo.c
    )

    mcux_add_configuration(
        CC "-DSERIAL_PORT_TYPE_SWO=1"
        CX "-DSERIAL_PORT_TYPE_SWO=1"
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.serial_manager_virtual)
    mcux_add_include(
        INCLUDES .
    )

    mcux_add_source(
        SOURCES
            fsl_component_serial_port_virtual.h
            fsl_component_serial_port_virtual.c
    )

    mcux_add_configuration(
        CC "-DSSERIAL_PORT_TYPE_VIRTUAL=1 -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING=1"
        CX "-DSSERIAL_PORT_TYPE_VIRTUAL=1 -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING=1"
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.serial_manager_rpmsg)
    mcux_add_include(
        INCLUDES .
    )

    mcux_add_source(
        SOURCES
            fsl_component_serial_port_rpmsg.h
            fsl_component_serial_port_rpmsg.c
    )

    mcux_add_configuration(
        CC "-DSERIAL_PORT_TYPE_RPMSG=1"
        CX "-DSERIAL_PORT_TYPE_RPMSG=1"
    )

endif()