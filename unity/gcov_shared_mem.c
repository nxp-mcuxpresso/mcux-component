/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "gcov_shared_mem.h"

/* Shared memory section definition */
// #if defined(__ICCARM__) /* IAR Workbench */
// #pragma location = "rpmsg_sh_mem_section"
// __no_init gcov_shared_mem_t gcov_sh_mem;
// #elif defined(__CC_ARM) || defined(__ARMCC_VERSION) /* Keil MDK */
// gcov_shared_mem_t gcov_sh_mem __attribute__((section("rpmsg_sh_mem_section")));
// #elif defined(__GNUC__)
// gcov_shared_mem_t gcov_sh_mem __attribute__((section(".noinit.$rpmsg_sh_mem")));
// #endif

/* This is reusing shared memory from linker file */
extern uint8_t __RPMSG_SH_MEM_START__;

gcov_shared_mem_t *gcov_get_shared_mem(void)
{
    // Cast the pointer to the start of the shared memory region
    return (gcov_shared_mem_t *)(&__RPMSG_SH_MEM_START__);
}
