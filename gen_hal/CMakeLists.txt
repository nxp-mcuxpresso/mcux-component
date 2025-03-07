# Copyright 2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.gen_hal)
    mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/flash)
    mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/memc)
    mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/pinctrl)
    mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/clock_control)
    mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/devices/${soc_portfolio}/${soc_series} OPTIONAL)
    mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/_boards/${board} OPTIONAL)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        SOURCES init.c
                zephyr/kernel/device.c
                sdk_integ/zephyr_mcux_helper.c
                zephyr/misc/empty_file.c
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
                ./zephyr/misc/generated
                ./zephyr/misc
                ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.device)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        SOURCES
                zephyr/include/zephyr/device.h
                zephyr/include/zephyr/devicetree.h
                zephyr/include/zephyr/pm/state.h
                zephyr/include/zephyr/pm/device.h
                _boards/${board}/devicetree_generated.h
                # zephyr/include/zephyr/irq_multilevel.h
                zephyr/misc/generated/zephyr/syscalls/device.h
                zephyr/include/zephyr/init.h
                zephyr/include/zephyr/sys/device_mmio.h
                # zephyr/include/zephyr/llext/symbol.h
                zephyr/include/zephyr/devicetree/clocks.h
                zephyr/include/zephyr/devicetree/ordinals.h
                zephyr/include/zephyr/devicetree/fixed-partitions.h
                zephyr/include/zephyr/internal/syscall_handler.h
                # zephyr/misc/sys/types.h
                zephyr/include/zephyr/sys/math_extras.h
                zephyr/include/zephyr/sys/math_extras_impl.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
                 ./zephyr/misc/generated
                 ./zephyr/misc
                 ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.toolchain)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        SOURCES
            zephyr/include/zephyr/types.h
            zephyr/include/zephyr/toolchain/armclang.h
            zephyr/include/zephyr/toolchain/common.h
            zephyr/include/zephyr/toolchain/gcc.h
            zephyr/include/zephyr/toolchain/iar.h
            zephyr/include/zephyr/toolchain/llvm.h
            zephyr/include/zephyr/toolchain/mwdt.h
            zephyr/include/zephyr/toolchain/xcc.h
            zephyr/include/zephyr/toolchain/xcc_missing_defs.h
            zephyr/include/zephyr/toolchain/zephyr_stdint.h
            zephyr/include/zephyr/toolchain.h
            sdk_integ/errno.h
            sdk_integ/zephyr_mcux_toolchain.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
                 ./zephyr/misc/generated
                 ./zephyr/misc
                 ./
    )
endif()


if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.types)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/types.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.sys_until)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/sys/util.h
            zephyr/sys/util_internal.h
            zephyr/sys/util_internal_is_eq.h
            zephyr/sys/util_internal_util_dec.h
            zephyr/sys/util_internal_util_inc.h
            zephyr/sys/util_internal_util_x2.h
            zephyr/sys/util_listify.h
            zephyr/sys/util_loops.h
            zephyr/sys/util_macro.h
            zephyr/sys/time_units.h
            # zephyr/sys/timeutil.h
            zephyr/sys/sys_io.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.assert)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/sys/__assert.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.linkers)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/linker/sections.h
            zephyr/linker/section_tags.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.sys_byteorder)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include
        SOURCES
            zephyr/sys/byteorder.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.arch)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/arch/cpu.h
            zephyr/arch/arch_interface.h
            zephyr/irq_offload.h
            zephyr/arch/syscall.h
            zephyr/arch/arm/syscall.h
            zephyr/timing/types.h
            zephyr/arch/arch_inlines.h
            zephyr/arch/arm/arch_inlines.h
            zephyr/arch/arm/thread.h
            zephyr/arch/arm/exception.h
            zephyr/arch/arm/cortex_m/exception.h
            zephyr/arch/arm/cortex_m/nvic.h
            zephyr/arch/arm/irq.h
            zephyr/sw_isr_table.h
            zephyr/arch/arm/error.h
            zephyr/arch/arm/misc.h
            zephyr/arch/arm/arch.h
            zephyr/arch/common/sys_io.h
            zephyr/arch/common/addr_types.h
            zephyr/arch/common/ffs.h
            zephyr/arch/arm/nmi.h
            zephyr/arch/arm/asm_inline.h
            zephyr/arch/arm/asm_inline_gcc.h
            zephyr/arch/common/sys_bitops.h
            zephyr/arch/arm/gdbstub.h
            zephyr/arch/arm/cortex_m/cpu.h
            zephyr/arch/arm/cortex_m/memory_map.h
            zephyr/fatal_types.h
            zephyr/arch/arm/mpu/arm_mpu.h
            zephyr/arch/arm/mpu/arm_mpu_v7m.h
            zephyr/arch/arm/mpu/arm_mpu_v8.h
            zephyr/arch/arm/mpu/nxp_mpu.h
            zephyr/arch/arm/mmu/arm_mmu.h
            zephyr/sys/rb.h
    )

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/misc/
        SOURCES
            arch/arm/asm_inline_other.h
            cmsis_core.h
            alloca.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
                 ./zephyr/misc/generated
                 ./zephyr/misc
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.logging)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/logging/log.h
            # zephyr/logging/log_instance.h
            zephyr/sys/iterable_sections.h
            # zephyr/logging/log_core.h
            # zephyr/logging/log_msg.h
            # zephyr/sys/mpsc_packet.h
            # zephyr/sys/cbprintf.h
            # zephyr/sys/cbprintf_internal.h
            # zephyr/sys/cbprintf_enums.h
            # zephyr/sys/cbprintf_cxx.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
                 ./zephyr/misc/generated
                 ./zephyr/misc
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.atomic)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/sys/atomic.h
            zephyr/sys/atomic_types.h
            # zephyr/sys/atomic_c.h
            zephyr/sys/atomic_builtin.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.kernel)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/kernel.h
            zephyr/kernel_includes.h
            zephyr/sys/dlist.h
            zephyr/sys/slist.h
            zephyr/sys/list_gen.h
            zephyr/sys/sflist.h
            zephyr/kernel/obj_core.h
            zephyr/kernel_structs.h
            zephyr/sys/sys_heap.h
            zephyr/arch/structs.h
            zephyr/kernel/stats.h
            zephyr/kernel/obj_core.h
            zephyr/kernel_version.h
            zephyr/syscall.h
            zephyr/sys/printk.h
            zephyr/sys_clock.h
            zephyr/spinlock.h
            zephyr/fatal.h
            zephyr/fatal_types.h
            zephyr/arch/exception.h
            zephyr/irq.h
            zephyr/kernel/thread_stack.h
            zephyr/app_memory/mem_domain.h
            zephyr/sys/kobject.h
            zephyr/sys/iterable_sections.h
            zephyr/sys/internal/kobject_internal.h
            zephyr/kernel/thread.h
            zephyr/kernel/mm/demand_paging.h
            zephyr/kernel/mm.h
            zephyr/kernel/internal/mm.h
            zephyr/sys/mem_manage.h
            zephyr/kernel/internal/smp.h
            zephyr/tracing/tracing_macros.h
            zephyr/sys/mem_stats.h
            zephyr/tracing/tracing.h
            zephyr/tracing/tracking.h
            zephyr/arch/arm/structs.h
    )
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr
        SOURCES
            misc/generated/zephyr/syscall_list.h
            misc/generated/zephyr/kobj-types-enum.h
            misc/generated/zephyr/syscalls/kobject.h
            misc/generated/zephyr/syscalls/kernel.h
            misc/generated/zephyr/syscalls/demand_paging.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
                 ./zephyr/misc/generated
                 ./zephyr/misc
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.debug_coredump)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/debug/coredump.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.storage_flash_map)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/storage/flash_map.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.zephyr_headers.storage_stream_flash)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/zephyr/include/
        SOURCES
            zephyr/storage/stream_flash.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES ./zephyr/include
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.gen_hal.osa)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/osa
        SOURCES
            thread/fsl_osa_thread.c
            thread/fsl_osa_thread.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal/osa
        INCLUDES 
            ./thread
    )
endif()
