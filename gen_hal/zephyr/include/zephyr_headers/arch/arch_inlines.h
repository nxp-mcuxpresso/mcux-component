/*
 * arch_inlines.h - automatically selects the correct arch_inlines.h file to
 *   include based on the selected architecture.
 */

/*
 * Copyright (c) 2019 Stephanos Ioannidis <root@stephanos.io>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_INLINES_H_
#define ZEPHYR_INCLUDE_ARCH_INLINES_H_

#if defined(CONFIG_X86)
#include <zephyr_headers/arch/x86/arch_inlines.h>
#elif defined(CONFIG_ARM)
#include <zephyr_headers/arch/arm/arch_inlines.h>
#elif defined(CONFIG_ARM64)
#include <zephyr_headers/arch/arm64/arch_inlines.h>
#elif defined(CONFIG_ARC)
#include <zephyr_headers/arch/arc/arch_inlines.h>
#elif defined(CONFIG_XTENSA)
#include <zephyr_headers/arch/xtensa/arch_inlines.h>
#elif defined(CONFIG_RISCV)
#include <zephyr_headers/arch/riscv/arch_inlines.h>
#elif defined(CONFIG_NIOS2)
#include <zephyr_headers/arch/nios2/arch_inlines.h>
#elif defined(CONFIG_MIPS)
#include <zephyr_headers/arch/mips/arch_inlines.h>
#elif defined(CONFIG_ARCH_POSIX)
#include <zephyr_headers/arch/posix/arch_inlines.h>
#elif defined(CONFIG_SPARC)
#include <zephyr_headers/arch/sparc/arch_inlines.h>
#endif

#endif /* ZEPHYR_INCLUDE_ARCH_INLINES_H_ */
