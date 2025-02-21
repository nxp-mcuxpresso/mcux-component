/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 * Copyright (c) 2019 Nordic Semiconductor ASA.
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_ARM_IRQ_H_
#define ZEPHYR_INCLUDE_ARCH_ARM_IRQ_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#if defined(CONFIG_ARM_SECURE_FIRMWARE)
/* Architecture-specific definition for the target security
 * state of an NVIC IRQ line.
 */
typedef enum {
	IRQ_TARGET_STATE_SECURE = 0,
	IRQ_TARGET_STATE_NON_SECURE
} irq_target_state_t;

#endif /* CONFIG_ARM_SECURE_FIRMWARE */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_ARCH_ARM_IRQ_H_ */
