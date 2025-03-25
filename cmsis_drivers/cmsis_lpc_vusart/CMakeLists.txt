if(CONFIG_MCUX_COMPONENT_driver.cmsis_vusart)
    mcux_component_version(2.0.0)

    mcux_add_source(SOURCES fsl_usart_cmsis.h fsl_usart_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
