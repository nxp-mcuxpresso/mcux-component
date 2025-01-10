# Copyright 2025 NXP
# SPDX-License-Identifier: Apache-2.0

if (CONFIG_MCUX_COMPONENT_component.gen_hal)

mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/flash)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/memc)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/pinctrl)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/zephyr/drivers/clock_control)
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/devices/${soc_portfolio}/${soc_series})
mcux_add_cmakelists(${SdkRootDirPath}/components/gen_hal/_boards/${board})

mcux_add_source(
    BASE_PATH ${SdkRootDirPath}/components/gen_hal
    SOURCES init.c
            zephyr/kernel/device.c
            sdk_integ/zephyr_mcux_helper.c
            zephyr/misc/empty_file.c
)

endif()

if (CONFIG_MCUX_COMPONENT_component.gen_hal_headers)
# Header
mcux_add_source(
    BASE_PATH ${SdkRootDirPath}/components/gen_hal
    SOURCES sdk_integ/*.h
            zephyr/include/zephyr/arch/arch_inlines.h
            zephyr/include/zephyr/arch/arch_interface.h
            zephyr/include/zephyr/arch/arm/arch.h
            zephyr/include/zephyr/arch/arm/arch_inlines.h
            zephyr/include/zephyr/arch/arm/asm_inline.h
            zephyr/include/zephyr/arch/arm/asm_inline_gcc.h
            zephyr/include/zephyr/arch/arm/barrier.h
            zephyr/include/zephyr/arch/arm/cortex_m/arm_mpu_mem_cfg.h
            zephyr/include/zephyr/arch/arm/cortex_m/cpu.h
            zephyr/include/zephyr/arch/arm/cortex_m/exception.h
            zephyr/include/zephyr/arch/arm/cortex_m/fpu.h
            zephyr/include/zephyr/arch/arm/cortex_m/memory_map.h
            zephyr/include/zephyr/arch/arm/cortex_m/nvic.h
            zephyr/include/zephyr/arch/arm/error.h
            zephyr/include/zephyr/arch/arm/exception.h
            zephyr/include/zephyr/arch/arm/gdbstub.h
            zephyr/include/zephyr/arch/arm/irq.h
            zephyr/include/zephyr/arch/arm/misc.h
            zephyr/include/zephyr/arch/arm/nmi.h
            zephyr/include/zephyr/arch/arm/structs.h
            zephyr/include/zephyr/arch/arm/syscall.h
            zephyr/include/zephyr/arch/arm/thread.h
            zephyr/include/zephyr/arch/common/sys_io.h
            zephyr/include/zephyr/arch/cpu.h
            zephyr/include/zephyr/arch/syscall.h
            zephyr/include/zephyr/debug/coredump.h
            zephyr/include/zephyr/device.h
            zephyr/include/zephyr/devicetree/can.h
            zephyr/include/zephyr/devicetree/clocks.h
            zephyr/include/zephyr/devicetree/dma.h
            zephyr/include/zephyr/devicetree/fixed-partitions.h
            zephyr/include/zephyr/devicetree/gpio.h
            zephyr/include/zephyr/devicetree/interrupt_controller.h
            zephyr/include/zephyr/devicetree/io-channels.h
            zephyr/include/zephyr/devicetree/mbox.h
            zephyr/include/zephyr/devicetree/ordinals.h
            zephyr/include/zephyr/devicetree/pinctrl.h
            zephyr/include/zephyr/devicetree/pwms.h
            zephyr/include/zephyr/devicetree/reset.h
            zephyr/include/zephyr/devicetree/spi.h
            zephyr/include/zephyr/devicetree.h
            zephyr/include/zephyr/drivers/clock_control.h
            zephyr/include/zephyr/drivers/flash.h
            zephyr/include/zephyr/drivers/pinctrl.h
            zephyr/include/zephyr/errno.h
            zephyr/include/zephyr/fatal_types.h
            zephyr/include/zephyr/init.h
            zephyr/include/zephyr/internal/syscall_handler.h
            zephyr/include/zephyr/irq.h
            zephyr/include/zephyr/kernel/internal/mm.h
            zephyr/include/zephyr/kernel/mm.h
            zephyr/include/zephyr/kernel/obj_core.h
            zephyr/include/zephyr/kernel.h
            zephyr/include/zephyr/kernel_includes.h
            zephyr/include/zephyr/kernel_structs.h
            zephyr/include/zephyr/kernel_version.h
            zephyr/include/zephyr/linker/sections.h
            zephyr/include/zephyr/linker/section_tags.h
            zephyr/include/zephyr/logging/log.h
            zephyr/include/zephyr/pm/device.h
            zephyr/include/zephyr/pm/state.h
            zephyr/include/zephyr/storage/flash_map.h
            zephyr/include/zephyr/storage/stream_flash.h
            zephyr/include/zephyr/sys/atomic.h
            zephyr/include/zephyr/sys/atomic_builtin.h
            zephyr/include/zephyr/sys/atomic_types.h
            zephyr/include/zephyr/sys/byteorder.h
            zephyr/include/zephyr/sys/device_mmio.h
            zephyr/include/zephyr/sys/dlist.h
            zephyr/include/zephyr/sys/errno_private.h
            zephyr/include/zephyr/sys/internal/kobject_internal.h
            zephyr/include/zephyr/sys/iterable_sections.h
            zephyr/include/zephyr/sys/kobject.h
            zephyr/include/zephyr/sys/list_gen.h
            zephyr/include/zephyr/sys/math_extras.h
            zephyr/include/zephyr/sys/math_extras_impl.h
            zephyr/include/zephyr/sys/mem_manage.h
            zephyr/include/zephyr/sys/sflist.h
            zephyr/include/zephyr/sys/slist.h
            zephyr/include/zephyr/sys/sys_io.h
            zephyr/include/zephyr/sys/time_units.h
            zephyr/include/zephyr/sys/util.h
            zephyr/include/zephyr/sys/util_internal.h
            zephyr/include/zephyr/sys/util_internal_is_eq.h
            zephyr/include/zephyr/sys/util_internal_util_dec.h
            zephyr/include/zephyr/sys/util_internal_util_inc.h
            zephyr/include/zephyr/sys/util_internal_util_x2.h
            zephyr/include/zephyr/sys/util_listify.h
            zephyr/include/zephyr/sys/util_loops.h
            zephyr/include/zephyr/sys/util_macro.h
            zephyr/include/zephyr/sys/__assert.h
            zephyr/include/zephyr/syscall.h
            zephyr/include/zephyr/sys_clock.h
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
            zephyr/include/zephyr/types.h
            zephyr/misc/generated/zephyr/*.h
            zephyr/misc/generated/zephyr/syscalls/*.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}/components/gen_hal
    INCLUDES ./zephyr/include
             ./zephyr/misc/generated
             ./sdk_integ
             ./
)

endif()
