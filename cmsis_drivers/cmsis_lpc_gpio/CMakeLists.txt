if(CONFIG_MCUX_COMPONENT_driver.cmsis_lpc_gpio)
    mcux_component_version(1.0.0)

    mcux_add_source(SOURCES fsl_gpio_cmsis.h fsl_gpio_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
