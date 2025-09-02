/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief Header files included by kernel.h.
 */

#ifndef ZEPHYR_INCLUDE_KERNEL_INCLUDES_H_
#define ZEPHYR_INCLUDE_KERNEL_INCLUDES_H_

#ifndef ZEPHYR_INCLUDE_KERNEL_H_
#error  Please do not include kernel-specific headers directly, use <zephyr/kernel.h> instead
#endif

#include <stddef.h>
#include <zephyr_headers/types.h>
#include <limits.h>
#include <zephyr_headers/toolchain.h>
#include <zephyr_headers/linker/sections.h>
#include <zephyr_headers/sys/atomic.h>
#include <zephyr_headers/sys/__assert.h>
#include <zephyr_headers/sys/dlist.h>
#include <zephyr_headers/sys/slist.h>
#include <zephyr_headers/sys/sflist.h>
#include <zephyr_headers/sys/util.h>
#include <zephyr_headers/kernel/obj_core.h>
#include <zephyr_headers/kernel_structs.h>
#include <zephyr_headers/kernel_version.h>
#include <zephyr_headers/syscall.h>
#include <zephyr_headers/sys/printk.h>
#include <zephyr_headers/arch/cpu.h>
#include <zephyr_headers/sys/rb.h>
#include <zephyr_headers/sys_clock.h>
#include <zephyr_headers/spinlock.h>
#include <zephyr_headers/fatal.h>
#include <zephyr_headers/irq.h>
#include <zephyr_headers/kernel/thread_stack.h>
#include <zephyr_headers/app_memory/mem_domain.h>
#include <zephyr_headers/sys/kobject.h>
#include <zephyr_headers/kernel/thread.h>
/* FIXME This needs to be removed. Exposes some private APIs to SOF */
#include <zephyr_headers/kernel/internal/smp.h>

#endif /* ZEPHYR_INCLUDE_KERNEL_INCLUDES_H_ */
