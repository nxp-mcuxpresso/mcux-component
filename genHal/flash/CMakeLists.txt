if (CONFIG_MCUX_COMPONENT_component.genHal.flash_flexspi_nor)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/genHal
        SOURCES 
                /flash/flash_mcux_flexspi_nor.c
                /flash/jesd216.c
                /flash/jesd216.h
                /flash/spi_nor.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/genHal
        INCLUDES 
                 /flash/.
    )
endif()

