if(CONFIG_MCUX_COMPONENT_driver.cmsis_lpuart)
    mcux_component_version(2.7.0)

    mcux_add_source(SOURCES fsl_lpuart_cmsis.h fsl_lpuart_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
