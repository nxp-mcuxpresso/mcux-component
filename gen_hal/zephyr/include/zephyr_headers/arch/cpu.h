/* cpu.h - automatically selects the correct arch.h file to include */

/*
 * Copyright (c) 1997-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_CPU_H_
#define ZEPHYR_INCLUDE_ARCH_CPU_H_

#include <zephyr_headers/arch/arch_interface.h>

#if defined(CONFIG_X86)
#include <zephyr_headers/arch/x86/arch.h>
#elif defined(CONFIG_ARM64)
#include <zephyr_headers/arch/arm64/arch.h>
#elif defined(CONFIG_ARM)
#include <zephyr_headers/arch/arm/arch.h>
#elif defined(CONFIG_ARC)
#include <zephyr_headers/arch/arc/arch.h>
#elif defined(CONFIG_NIOS2)
#include <zephyr_headers/arch/nios2/arch.h>
#elif defined(CONFIG_RISCV)
#include <zephyr_headers/arch/riscv/arch.h>
#elif defined(CONFIG_XTENSA)
#include <zephyr_headers/arch/xtensa/arch.h>
#elif defined(CONFIG_MIPS)
#include <zephyr_headers/arch/mips/arch.h>
#elif defined(CONFIG_ARCH_POSIX)
#include <zephyr_headers/arch/posix/arch.h>
#elif defined(CONFIG_SPARC)
#include <zephyr_headers/arch/sparc/arch.h>
#endif

#endif /* ZEPHYR_INCLUDE_ARCH_CPU_H_ */
