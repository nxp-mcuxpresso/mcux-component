if(CONFIG_MCUX_COMPONENT_driver.cmsis_spi)
    mcux_component_version(1.0.0)

    mcux_add_source(SOURCES fsl_spi_cmsis.h fsl_spi_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
