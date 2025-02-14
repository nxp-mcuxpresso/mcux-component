/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright 2025 NXP.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Kernel initialization module
 *
 * This module contains routines that are used to initialize the kernel.
 */

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <errno.h>

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __init_start              Image$$ER_m_init_start$$Base
#define __init_EARLY_start        Image$$ER_m_init_early$$Base
#define __init_PRE_KERNEL_1_start Image$$ER_m_init_pre_kernel_1$$Base
#define __init_PRE_KERNEL_2_start Image$$ER_m_init_pre_kernel_2$$Base
#define __init_POST_KERNEL_start  Image$$ER_m_init_post_kernel$$Base
#define __init_APPLICATION_start  Image$$ER_m_init_application$$Base
#define __init_SMP_start          Image$$ER_m_init_smp$$Base
#define __init_end                Image$$ER_m_init_end$$Limit
#endif

#if defined(__ICCARM__)
#pragma section="INIT_EARLY"
#pragma section="INIT_PRE_KERNEL_1"
#pragma section="INIT_PRE_KERNEL_2"
#pragma section="INIT_POST_KERNEL"
#pragma section="INIT_APPLICATION"
#ifdef CONFIG_SMP
#pragma section="INIT_SMP"
#endif

#define __init_start               __section_begin("INIT_EARLY")
#define __init_EARLY_start         __section_begin("INIT_EARLY")
#define __init_PRE_KERNEL_1_start  __section_begin("INIT_PRE_KERNEL_1")
#define __init_PRE_KERNEL_2_start  __section_begin("INIT_PRE_KERNEL_2")
#define __init_POST_KERNEL_start   __section_begin("INIT_POST_KERNEL")
#define __init_APPLICATION_start   __section_begin("INIT_APPLICATION")

#ifdef CONFIG_SMP
#define __init_SMP_start           __section_begin("INIT_SMP")
#define __init_end                 __section_end("INIT_SMP")
#else
#define __init_end                 __section_end("INIT_APPLICATION")
#endif

#endif

#if !defined(__ICCARM__)
extern const struct init_entry __init_start[];
extern const struct init_entry __init_EARLY_start[];
extern const struct init_entry __init_PRE_KERNEL_1_start[];
extern const struct init_entry __init_PRE_KERNEL_2_start[];
extern const struct init_entry __init_POST_KERNEL_start[];
extern const struct init_entry __init_APPLICATION_start[];
extern const struct init_entry __init_end[];
#endif

enum init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#ifdef CONFIG_SMP
	INIT_LEVEL_SMP,
#endif /* CONFIG_SMP */
};

extern void z_device_state_init(void);

static int do_device_init(const struct init_entry *entry)
{
	const struct device *dev = entry->dev;
	int rc = 0;

	if (entry->init_fn.dev != NULL) {
		rc = entry->init_fn.dev(dev);
		/* Mark device initialized. If initialization
		 * failed, record the error condition.
		 */
		if (rc != 0) {
			if (rc < 0) {
				rc = -rc;
			}
			if (rc > UINT8_MAX) {
				rc = UINT8_MAX;
			}
			dev->state->init_res = rc;
		}
	}

	dev->state->initialized = true;

	return rc;
}

/**
 * @brief Execute all the init entry initialization functions at a given level
 *
 * @details Invokes the initialization routine for each init entry object
 * created by the INIT_ENTRY_DEFINE() macro using the specified level.
 * The linker script places the init entry objects in memory in the order
 * they need to be invoked, with symbols indicating where one level leaves
 * off and the next one begins.
 *
 * @param level init level to run.
 */
static void z_sys_init_run_level(enum init_level level)
{
	static const struct init_entry *levels[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#ifdef CONFIG_SMP
		__init_SMP_start,
#endif /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};

	const struct init_entry *entry;

	for (entry = levels[level]; entry < levels[level+1]; entry++) {
		const struct device *dev = entry->dev;
		int result;

		//sys_trace_sys_init_enter(entry, level);
		if (dev != NULL) {
			result = do_device_init(entry);
		} else {
			result = entry->init_fn.sys();
		}
		//sys_trace_sys_init_exit(entry, level, result);
	}
}


int z_impl_device_init(const struct device *dev)
{
	if (dev == NULL) {
		return -ENOENT;
	}

	STRUCT_SECTION_FOREACH_ALTERNATE(_deferred_init, init_entry, entry) {
		if (entry->dev == dev) {
			return do_device_init(entry);
		}
	}

	return -ENOENT;
}

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
static void bg_thread_main(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	z_sys_init_run_level(INIT_LEVEL_POST_KERNEL);

	/* Final init level before app starts */
	z_sys_init_run_level(INIT_LEVEL_APPLICATION);
}


/**
 *
 * @brief Initialize kernel
 *
 * This routine is invoked when the system is ready to run C code. The
 * processor must be running in 32-bit mode, and the BSS must have been
 * cleared/zeroed.
 *
 * @return Does not return
 */
void genhal_init(void)
{
	/* initialize early init calls */
	z_sys_init_run_level(INIT_LEVEL_EARLY);

	/* do any necessary initialization of static devices */
	z_device_state_init();

	/* perform basic hardware initialization */
	z_sys_init_run_level(INIT_LEVEL_PRE_KERNEL_1);
	z_sys_init_run_level(INIT_LEVEL_PRE_KERNEL_2);

	bg_thread_main(NULL, NULL, NULL);
}
