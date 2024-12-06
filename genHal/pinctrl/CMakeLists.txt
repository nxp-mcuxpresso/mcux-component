
if (CONFIG_MCUX_COMPONENT_component.genHal.pinctrl.mci_io_mux)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/
        SOURCES genHal/pinctrl/pinctrl_mci_io_mux.c
                genHal/pinctrl/common.c
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/genHal
        INCLUDES /pinctrl/.
    )
endif()
