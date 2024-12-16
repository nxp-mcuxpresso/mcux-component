
if (CONFIG_MCUX_COMPONENT_component.gen_hal.pinctrl.mci_io_mux)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        SOURCES zephyr/drivers/pinctrl/pinctrl_mci_io_mux.c
                zephyr/drivers/pinctrl/common.c
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES zephyr/drivers/pinctrl/.
    )
endif()
