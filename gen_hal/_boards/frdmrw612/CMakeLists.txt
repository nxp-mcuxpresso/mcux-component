mcux_add_include(
    INCLUDES ./
)

mcux_add_source(
    SOURCES
        ./devicetree_generated.h
)

if (CONFIG_MCUX_PRJSEG_module.gen_hal.linker)
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
            BASE_PATH ${SdkRootDirPath}/components/gen_hal/_boards/${board}
            LINKER linker/iar/RW612_ram.icf
    )

    mcux_add_iar_linker_script(
            TARGETS flash_debug flash_release
            BASE_PATH ${SdkRootDirPath}/components/gen_hal/_boards/${board}
            LINKER linker/iar/RW612_flash.icf
    )

    mcux_remove_armgcc_linker_script(
        TARGETS debug release
        BASE_PATH ${SdkRootDirPath}
        LINKER devices/${soc_portfolio}/${soc_series}/${device}/gcc/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_ram.ld
    )

    mcux_remove_armgcc_linker_script(
        TARGETS flash_debug flash_release
        BASE_PATH ${SdkRootDirPath}
        LINKER devices/${soc_portfolio}/${soc_series}/${device}/gcc/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_flash.ld
    )

    mcux_add_armgcc_linker_script(
        TARGETS debug release
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/_boards/${board}
        LINKER linker/gcc/RW612_ram.ld
    )

    mcux_add_armgcc_linker_script(
        TARGETS flash_debug flash_release
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/_boards/${board}
        LINKER linker/gcc/RW612_flash.ld
    )
endif()
