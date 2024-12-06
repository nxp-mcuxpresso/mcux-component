if (CONFIG_MCUX_PRJSEG_module.genHal.linker)
    mcux_add_include(
        INCLUDES ./
    )

    mcux_add_source(
        SOURCES
            ./devicetree_generated.h
    )

    mcux_remove_iar_linker_script(
        TARGETS debug release
        BASE_PATH ${SdkRootDirPath}
        LINKER devices/${soc_portfolio}/${soc_series}/${device}/iar/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_ram.icf
    )

    mcux_remove_iar_linker_script(
        TARGETS flash_debug flash_release
        BASE_PATH ${SdkRootDirPath}
        LINKER devices/${soc_portfolio}/${soc_series}/${device}/iar/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_flash.icf
    )

    mcux_add_iar_linker_script(
            TARGETS debug release
            BASE_PATH ${SdkRootDirPath}/components/genHal/_boards/${board}
            LINKER linker/iar/RW612_ram.icf
    )

    mcux_add_iar_linker_script(
            TARGETS flash_debug flash_release
            BASE_PATH ${SdkRootDirPath}/components/genHal/_boards/${board}
            LINKER linker/iar/RW612_flash.icf
    )
endif()
