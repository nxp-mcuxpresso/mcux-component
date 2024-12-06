if (CONFIG_MCUX_COMPONENT_component.genHal.memc.flexspi)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/genHal
        SOURCES /memc/memc_mcux_flexspi.c
                /memc/memc_mcux_flexspi.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/genHal
        INCLUDES /memc/.
    )
endif()
