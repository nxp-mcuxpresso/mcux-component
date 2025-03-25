if(CONFIG_MCUX_COMPONENT_driver.cmsis_lpi2c)
    mcux_component_version(2.5.0)

    mcux_add_source(SOURCES fsl_lpi2c_cmsis.h fsl_lpi2c_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
