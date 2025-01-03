if (CONFIG_MCUX_COMPONENT_component.gen_hal.flash_flexspi_nor)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/drivers
        SOURCES
                flash/flash_mcux_flexspi_nor.c
                flash/flash_page_layout.c
                flash/flash_util.c
                flash/jesd216.c
                flash/jesd216.h
                flash/spi_nor.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/drivers
        INCLUDES
                flash/.
    )
endif()

