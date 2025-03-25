if(CONFIG_MCUX_COMPONENT_driver.cmsis_iuart)
    mcux_component_version(2.1.0)

    mcux_add_source(SOURCES fsl_uart_cmsis.h fsl_uart_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
