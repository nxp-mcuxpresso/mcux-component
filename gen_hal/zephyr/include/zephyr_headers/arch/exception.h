/* exception.h - automatically selects the correct exception.h file to include */

/*
 * Copyright (c) 2024 Meta Platforms
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_EXCEPTION_H_
#define ZEPHYR_INCLUDE_ARCH_EXCEPTION_H_

#if defined(CONFIG_X86_64)
#include <zephyr_headers/arch/x86/intel64/exception.h>
#elif defined(CONFIG_X86)
#include <zephyr_headers/arch/x86/ia32/exception.h>
#elif defined(CONFIG_ARM64)
#include <zephyr_headers/arch/arm64/exception.h>
#elif defined(CONFIG_ARM)
#include <zephyr_headers/arch/arm/exception.h>
#elif defined(CONFIG_ARC)
#include <zephyr_headers/arch/arc/v2/exception.h>
#elif defined(CONFIG_NIOS2)
#include <zephyr_headers/arch/nios2/exception.h>
#elif defined(CONFIG_RISCV)
#include <zephyr_headers/arch/riscv/exception.h>
#elif defined(CONFIG_XTENSA)
#include <zephyr_headers/arch/xtensa/exception.h>
#elif defined(CONFIG_MIPS)
#include <zephyr_headers/arch/mips/exception.h>
#elif defined(CONFIG_ARCH_POSIX)
#include <zephyr_headers/arch/posix/exception.h>
#elif defined(CONFIG_SPARC)
#include <zephyr_headers/arch/sparc/exception.h>
#endif

#endif /* ZEPHYR_INCLUDE_ARCH_EXCEPTION_H_ */
