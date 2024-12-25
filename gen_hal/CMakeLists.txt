mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/flash)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/memc)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/pinctrl)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/clock_control)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/_boards/${board})
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/devices/${soc_portfolio}/${soc_series}/${device})

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}/components/gen_hal
    INCLUDES ./zephyr/include
             ./zephyr/misc/generated
             ./sdk_integ
             ./
)

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}/components/gen_hal
    SOURCES init.c
            zephyr/kernel/device.c
            sdk_integ/zephyr_mcux_helper.c
)
