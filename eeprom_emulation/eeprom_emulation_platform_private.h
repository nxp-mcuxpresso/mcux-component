/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _EEPROM_EMULATION_PLATFORM_PRIVATE_H_
#define _EEPROM_EMULATION_PLATFORM_PRIVATE_H_

#include "fsl_debug_console.h"

//#define LOG_DBG(s, ...) PRINTF("[DBG] "s"\r\n", __VA_ARGS__)
#define LOG_DBG(...)
#define LOG_ERR(s)      PRINTF("[ERR] "s"\r\n")

#define k_mutex_init(lock)    (void)(lock)
#define k_mutex_lock(lock, t) (void)(lock)
#define k_mutex_unlock(lock)  (void)(lock)

#define device_is_ready(...) true

#define __ASSERT(test, ...) assert(test)


#endif /* _EEPROM_EMULATION_PLATFORM_PRIVATE_H_ */

