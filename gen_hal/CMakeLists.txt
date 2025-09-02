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
                zephyr/include/zephyr_headers/device.h
                zephyr/include/zephyr_headers/devicetree.h
                zephyr/include/zephyr_headers/pm/state.h
                zephyr/include/zephyr_headers/pm/device.h
                _boards/${board}/devicetree_generated.h
                # zephyr/include/zephyr_headers/irq_multilevel.h
                zephyr/misc/generated/zephyr/syscalls/device.h
                zephyr/include/zephyr_headers/init.h
                zephyr/include/zephyr_headers/sys/device_mmio.h
                # zephyr/include/zephyr_headers/llext/symbol.h
                zephyr/include/zephyr_headers/devicetree/clocks.h
                zephyr/include/zephyr_headers/devicetree/ordinals.h
                zephyr/include/zephyr_headers/devicetree/fixed-partitions.h
                zephyr/include/zephyr_headers/internal/syscall_handler.h
                # zephyr/misc/sys/types.h
                zephyr/include/zephyr_headers/sys/math_extras.h
                zephyr/include/zephyr_headers/sys/math_extras_impl.h
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
            zephyr/include/zephyr_headers/types.h
            zephyr/include/zephyr_headers/toolchain/armclang.h
            zephyr/include/zephyr_headers/toolchain/common.h
            zephyr/include/zephyr_headers/toolchain/gcc.h
            zephyr/include/zephyr_headers/toolchain/iar.h
            zephyr/include/zephyr_headers/toolchain/llvm.h
            zephyr/include/zephyr_headers/toolchain/mwdt.h
            zephyr/include/zephyr_headers/toolchain/xcc.h
            zephyr/include/zephyr_headers/toolchain/xcc_missing_defs.h
            zephyr/include/zephyr_headers/toolchain/zephyr_stdint.h
            zephyr/include/zephyr_headers/toolchain.h
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
            zephyr_headers/types.h
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
            zephyr_headers/sys/util.h
            zephyr_headers/sys/util_internal.h
            zephyr_headers/sys/util_internal_is_eq.h
            zephyr_headers/sys/util_internal_util_dec.h
            zephyr_headers/sys/util_internal_util_inc.h
            zephyr_headers/sys/util_internal_util_x2.h
            zephyr_headers/sys/util_listify.h
            zephyr_headers/sys/util_loops.h
            zephyr_headers/sys/util_macro.h
            zephyr_headers/sys/time_units.h
            # zephyr_headers/sys/timeutil.h
            zephyr_headers/sys/sys_io.h
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
            zephyr_headers/sys/__assert.h
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
            zephyr_headers/linker/sections.h
            zephyr_headers/linker/section_tags.h
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
            zephyr_headers/sys/byteorder.h
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
            zephyr_headers/arch/cpu.h
            zephyr_headers/arch/arch_interface.h
            zephyr_headers/irq_offload.h
            zephyr_headers/arch/syscall.h
            zephyr_headers/arch/arm/syscall.h
            zephyr_headers/timing/types.h
            zephyr_headers/arch/arch_inlines.h
            zephyr_headers/arch/arm/arch_inlines.h
            zephyr_headers/arch/arm/thread.h
            zephyr_headers/arch/arm/exception.h
            zephyr_headers/arch/arm/cortex_m/exception.h
            zephyr_headers/arch/arm/cortex_m/nvic.h
            zephyr_headers/arch/arm/irq.h
            zephyr_headers/sw_isr_table.h
            zephyr_headers/arch/arm/error.h
            zephyr_headers/arch/arm/misc.h
            zephyr_headers/arch/arm/arch.h
            zephyr_headers/arch/common/sys_io.h
            zephyr_headers/arch/common/addr_types.h
            zephyr_headers/arch/common/ffs.h
            zephyr_headers/arch/arm/nmi.h
            zephyr_headers/arch/arm/asm_inline.h
            zephyr_headers/arch/arm/asm_inline_gcc.h
            zephyr_headers/arch/common/sys_bitops.h
            zephyr_headers/arch/arm/gdbstub.h
            zephyr_headers/arch/arm/cortex_m/cpu.h
            zephyr_headers/arch/arm/cortex_m/memory_map.h
            zephyr_headers/fatal_types.h
            zephyr_headers/arch/arm/mpu/arm_mpu.h
            zephyr_headers/arch/arm/mpu/arm_mpu_v7m.h
            zephyr_headers/arch/arm/mpu/arm_mpu_v8.h
            zephyr_headers/arch/arm/mpu/nxp_mpu.h
            zephyr_headers/arch/arm/mmu/arm_mmu.h
            zephyr_headers/sys/rb.h
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
            zephyr_headers/logging/log.h
            # zephyr/logging/log_instance.h
            zephyr_headers/sys/iterable_sections.h
            # zephyr/logging/log_core.h
            # zephyr/logging/log_msg.h
            # zephyr_headers/sys/mpsc_packet.h
            # zephyr_headers/sys/cbprintf.h
            # zephyr_headers/sys/cbprintf_internal.h
            # zephyr_headers/sys/cbprintf_enums.h
            # zephyr_headers/sys/cbprintf_cxx.h
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
            zephyr_headers/sys/atomic.h
            zephyr_headers/sys/atomic_types.h
            # zephyr_headers/sys/atomic_c.h
            zephyr_headers/sys/atomic_builtin.h
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
            zephyr_headers/kernel.h
            zephyr_headers/kernel_includes.h
            zephyr_headers/sys/dlist.h
            zephyr_headers/sys/slist.h
            zephyr_headers/sys/list_gen.h
            zephyr_headers/sys/sflist.h
            zephyr_headers/kernel/obj_core.h
            zephyr_headers/kernel_structs.h
            zephyr_headers/sys/sys_heap.h
            zephyr_headers/arch/structs.h
            zephyr_headers/kernel/stats.h
            zephyr_headers/kernel/obj_core.h
            zephyr_headers/kernel_version.h
            zephyr_headers/syscall.h
            zephyr_headers/sys/printk.h
            zephyr_headers/sys_clock.h
            zephyr_headers/spinlock.h
            zephyr_headers/fatal.h
            zephyr_headers/fatal_types.h
            zephyr_headers/arch/exception.h
            zephyr_headers/irq.h
            zephyr_headers/kernel/thread_stack.h
            zephyr_headers/app_memory/mem_domain.h
            zephyr_headers/sys/kobject.h
            zephyr_headers/sys/iterable_sections.h
            zephyr_headers/sys/internal/kobject_internal.h
            zephyr_headers/kernel/thread.h
            zephyr_headers/kernel/mm/demand_paging.h
            zephyr_headers/kernel/mm.h
            zephyr_headers/kernel/internal/mm.h
            zephyr_headers/sys/mem_manage.h
            zephyr_headers/kernel/internal/smp.h
            zephyr_headers/tracing/tracing_macros.h
            zephyr_headers/sys/mem_stats.h
            zephyr_headers/tracing/tracing.h
            zephyr_headers/tracing/tracking.h
            zephyr_headers/arch/arm/structs.h
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
            zephyr_headers/debug/coredump.h
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
            zephyr_headers/storage/flash_map.h
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
            zephyr_headers/storage/stream_flash.h
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
