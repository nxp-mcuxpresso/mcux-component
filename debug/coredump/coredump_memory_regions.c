/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#ifdef __ZEPHYR__
#include <kernel_internal.h>
#endif
#include <zephyr_headers/toolchain.h>
#include <zephyr_headers/debug/coredump.h>
#ifdef __ZEPHYR__
#include <zephyr_headers/linker/linker-defs.h>
#endif
#include <zephyr_headers/sys/byteorder.h>
#include <zephyr_headers/sys/util.h>

#include "coredump_internal.h"

#ifdef CONFIG_DEBUG_COREDUMP_MEMORY_DUMP_LINKER_RAM
struct z_coredump_memory_region_t __weak z_coredump_memory_regions[] = {
	{(uintptr_t)&_image_ram_start, (uintptr_t)&_image_ram_end},
	{0, 0} /* End of list */
};
#endif