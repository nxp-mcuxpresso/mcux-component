if(CONFIG_MCUX_COMPONENT_driver.cmsis_lpspi)
    mcux_component_version(2.13.0)

    mcux_add_source(SOURCES fsl_lpspi_cmsis.h fsl_lpspi_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
