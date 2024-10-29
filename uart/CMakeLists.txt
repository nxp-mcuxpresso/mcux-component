# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.uart_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_uart.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.iuart_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_iuart.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.lpsci_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_lpsci.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.lpuart_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_lpuart.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.lpuart_dma_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_lpuart.c
    )

    mcux_add_include(
        INCLUDES .
    )
	
    mcux_add_configuration(
        AS "-DHAL_UART_DMA_ENABLE=0"
        CC "-DHAL_UART_DMA_ENABLE=0"
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.miniusart_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_miniusart.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.usart_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_usart.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.usart_dma_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_usart.c
    )

    mcux_add_include(
        INCLUDES .
    )
	
    mcux_add_configuration(
        AS "-DHAL_UART_DMA_ENABLE=0"
        CC "-DHAL_UART_DMA_ENABLE=0"
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.vusart_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_vusart.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.qsci_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_uart.h
                fsl_adapter_qsci.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()
