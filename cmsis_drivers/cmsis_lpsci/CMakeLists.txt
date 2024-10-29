if(CONFIG_MCUX_COMPONENT_driver.cmsis_lpsci)
    mcux_component_version(1.0.0)

    mcux_add_source(SOURCES fsl_lpsci_cmsis.h fsl_lpsci_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
