/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_MCUX_TOOLCHAIN_H_
#define ZEPHYR_MCUX_TOOLCHAIN_H_

#if defined(__ICCARM__)

#define _device_list_start DEVICE_AREA$$Base
#define _device_list_end   DEVICE_AREA$$Limit

#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)

#define _device_list_start         Image$$ER_m_device_list_start$$Limit
#define _device_list_end           Image$$ER_m_device_list_end$$Base
#define __deferred_init_list_start Image$$ER_m_deferred_init_list_start$$Limit
#define __deferred_init_list_end   Image$$ER_m_deferred_init_list_end$$Base

#endif

#endif /* ZEPHYR_MCUX_TOOLCHAIN_H_ */
