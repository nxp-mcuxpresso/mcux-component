/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _EEPROM_EMULATION_PLATFORM_H_
#define _EEPROM_EMULATION_PLATFORM_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define EEPROM_EMU_EOK      0
#define EEPROM_EMU_EIO      5
#define EEPROM_EMU_EACCES  13
#define EEPROM_EMU_ENODEV  19
#define EEPROM_EMU_EINVAL  22

struct k_mutex {
    int m;
};

typedef int32_t off_t;

#endif /* _EEPROM_EMULATION_PLATFORM_H_ */

