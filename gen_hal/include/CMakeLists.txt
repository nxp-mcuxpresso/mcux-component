if (CONFIG_MCUX_COMPONENT_component.zephyr_header.core)
    mcux_add_include(
        INCLUDES ./
    )

    mcux_add_include(
        INCLUDES ./kernel/include
    )

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/zephyr/.
        SOURCES device.h
                devicetree.h
                init.h
                types.h
                toolchain.h
                kernel.h
                irq.h
                irq_multilevel.h
                sys_clock.h
                fatal_types.h
                kernel_structs.h
                kernel_includes.h
                kernel_version.h
                sys/util_internal_is_eq.h
                sys/util_internal_util_dec.h
                sys/util_internal_util_inc.h
                sys/util_internal_util_x2.h
                sys/util_internal.h
                sys/util_listify.h
                sys/util_loops.h
                sys/util_macro.h
                sys/util.h
                sys/__assert.h
                sys/byteorder.h
                sys/errno_private.h
                sys/iterable_sections.h
                sys/slist.h
                sys/device_mmio.h
                sys/time_units.h
                sys/sys_io.h
                sys/dlist.h
                sys/sflist.h
                sys/list_gen.h
                pm/state.h
                pm/device.h
                linker/sections.h
                devicetree/ordinals.h
                logging/log.h
                arch/cpu.h
                arch/arm/thread.h
                arch/arm/exception.h
                arch/common/sys_io.h
    )

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/sys/.
        SOURCES types.h
    )

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/.
        SOURCES util_mcux_helper.h
    )

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/kernel/include/.
        SOURCES kernel_internal.h
                kernel_arch_interface.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_header.drv_flash)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/zephyr/.
        SOURCES
            drivers/flash.h  
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_header.drv_clock_control)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/zephyr/.
        SOURCES
            drivers/clock_control.h
            devicetree/clocks.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_header.drv_memc)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/zephyr/.
        SOURCES
            drivers/memc.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_header.drv_pinctrl)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/zephyr/.
        SOURCES
            drivers/pinctrl.h
            devicetree/pinctrl.h
    )
endif()


if (CONFIG_MCUX_COMPONENT_component.zephyr_header.stream_flash)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/zephyr/.
        SOURCES
            storage/stream_flash.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_header.flash_map)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/zephyr/.
        SOURCES
            storage/flash_map.h
            devicetree/fixed-partitions.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_header.coredump)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/include/zephyr/.
        SOURCES
            debug/coredump.h
    )
endif()
