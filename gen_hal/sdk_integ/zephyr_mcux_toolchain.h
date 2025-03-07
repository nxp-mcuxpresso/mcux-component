/*
 * Copyright 2024 - 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ZEPHYR_MCUX_TOOLCHAIN_H_
#define ZEPHYR_MCUX_TOOLCHAIN_H_

#include "fsl_common.h"
#include "sdk_integ/errno.h"

#ifdef SDK_OS_FREE_RTOS
#include "FreeRTOSConfig_Gen.h"
#endif
#include "ZephyrMap_Gen.h"

#if defined(__ICCARM__)

#define _device_list_start DEVICE_AREA$$Base
#define _device_list_end   DEVICE_AREA$$Limit

extern char _image_ram_start[];
extern char _image_ram_end[];
extern char _image_ram_size[];

#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define _device_list_start         Image$$ER_m_device_list_start$$Limit
#define _device_list_end           Image$$ER_m_device_list_end$$Base
#define __deferred_init_list_start Image$$ER_m_deferred_init_list_start$$Limit
#define __deferred_init_list_end   Image$$ER_m_deferred_init_list_end$$Base

extern uint32_t Image$$Coredump_ram_start$$RW$$Base;
extern uint32_t Image$$Coredump_ram_end$$Base;
#define _image_ram_start           Image$$Coredump_ram_start$$RW$$Base
#define _image_ram_end             Image$$Coredump_ram_end$$Base

#elif defined(__GNUC__)
extern char _image_ram_start[];
extern char _image_ram_end[];
extern char _image_ram_size[];
#endif

#ifndef _OFF_T_DECLARED
typedef intptr_t off_t;
#define _OFF_T_DECLARED
#endif


#endif /* ZEPHYR_MCUX_TOOLCHAIN_H_ */
