if(CONFIG_MCUX_COMPONENT_driver.cmsis_i2c)
    mcux_component_version(2.2.0)

    mcux_add_source(SOURCES fsl_i2c_cmsis.h fsl_i2c_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
