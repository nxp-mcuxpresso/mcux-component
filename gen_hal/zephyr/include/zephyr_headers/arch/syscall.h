/* syscall.h - automatically selects the correct syscall.h file to include */

/*
 * Copyright (c) 1997-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_SYSCALL_H_
#define ZEPHYR_INCLUDE_ARCH_SYSCALL_H_

#if defined(CONFIG_X86_64)
#include <zephyr_headers/arch/x86/intel64/syscall.h>
#elif defined(CONFIG_X86)
#include <zephyr_headers/arch/x86/ia32/syscall.h>
#elif defined(CONFIG_ARM64)
#include <zephyr_headers/arch/arm64/syscall.h>
#elif defined(CONFIG_ARM)
#include <zephyr_headers/arch/arm/syscall.h>
#elif defined(CONFIG_ARC)
#include <zephyr_headers/arch/arc/syscall.h>
#elif defined(CONFIG_RISCV)
#include <zephyr_headers/arch/riscv/syscall.h>
#elif defined(CONFIG_XTENSA)
#include <zephyr_headers/arch/xtensa/syscall.h>
#endif

#endif /* ZEPHYR_INCLUDE_ARCH_SYSCALL_H_ */
