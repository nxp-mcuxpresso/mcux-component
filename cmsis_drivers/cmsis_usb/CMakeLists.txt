if(CONFIG_MCUX_COMPONENT_driver.cmsis_usb)
    mcux_component_version(1.0.0)

    mcux_add_source(SOURCES fsl_usbd_ehci_cmsis.h fsl_usbd_ehci_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
