mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/include)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/flash)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/memc)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/pinctrl)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/clock_control)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/_boards/${board})
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/devices/${soc_portfolio}/${soc_series}/${device})

mcux_add_source(
    SOURCES
        ./util_mcux_helper.c
)
