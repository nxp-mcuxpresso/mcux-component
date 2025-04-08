//! @file
//!
//! Copyright (c) Memfault, Inc.
//! See LICENSE for details
//!
//! Copyright 2025 Memfault, Inc
//! Copyright 2025 NXP
//!
//! Licensed under the Apache License, Version 2.0 (the "License");
//! you may not use this file except in compliance with the License.
//! You may obtain a copy of the License at
//!
//!     http://www.apache.org/licenses/LICENSE-2.0
//!
//! Unless required by applicable law or agreed to in writing, software
//! distributed under the License is distributed on an "AS IS" BASIS,
//! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//! See the License for the specific language governing permissions and
//! limitations under the License.
//!
//! Glue layer between the Memfault SDK and the underlying platform
//!

#include "memfault/config.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "memfault/components.h"

#include "mflash_drv.h"
#include "memfault/ports/buffered_coredump_storage.h"

/*
 * A flash region shall be reserved for memfault. This flash region is specified
 * by such config macros, which should be defined by the project.
 *
 * CONFIG_MEMFAULT_FLASH_BASE_OFFSET: Offset of flash region start address
 * CONFIG_MEMFAULT_FLASH_SIZE: Size of the flash region
 */

#define MEMFAULT_FLASH_SECTOR_SIZE MFLASH_SECTOR_SIZE
#define MEMFAULT_FLASH_PAGE_SIZE MFLASH_PAGE_SIZE
#define MEMFAULT_PHYS_ADDR(offset)    ((uint32_t)CONFIG_MEMFAULT_FLASH_BASE_OFFSET + (uint32_t)(offset)) /* physical address. */
#define MEMFAULT_FLASH_LOG_ADDR(offset) ((uint32_t)MFLASH_BASE_ADDRESS + MEMFAULT_PHYS_ADDR(offset)) /* Logical address. */

void memfault_platform_coredump_storage_get_info(sMfltCoredumpStorageInfo *info)
{
    *info = (sMfltCoredumpStorageInfo){
        .size        = CONFIG_MEMFAULT_FLASH_SIZE,
        .sector_size = MEMFAULT_FLASH_SECTOR_SIZE,
    };
}

static bool prv_op_within_flash_bounds(uint32_t offset, size_t data_len)
{
	return (((UINT32_MAX - data_len) >= offset) && ((offset + data_len) <= (uint32_t)CONFIG_MEMFAULT_FLASH_SIZE));
}

bool memfault_platform_coredump_storage_read(uint32_t offset, void *data, size_t read_len)
{
    if (!prv_op_within_flash_bounds(offset, read_len))
    {
        return false;
    }

#if defined(MFLASH_REMAP_ACTIVE)
    if (!MFLASH_REMAP_ACTIVE())
    {
        memcpy(data, (uint8_t *)MEMFAULT_FLASH_LOG_ADDR(offset), read_len);
    }
    else
#endif
    {
        if (kStatus_Success != mflash_drv_read(MEMFAULT_PHYS_ADDR(offset), (uint32_t *)data, read_len))
        {
            return false;
        }
    }

    return true;
}

bool memfault_platform_coredump_storage_erase(uint32_t offset, size_t erase_size)
{
    if (!prv_op_within_flash_bounds(offset, erase_size))
    {
        return false;
    }

    /* Flash erasing should be sector size aligned. */
    if ((0U != (offset % MEMFAULT_FLASH_SECTOR_SIZE)) || (0U != (erase_size % MEMFAULT_FLASH_SECTOR_SIZE)))
    {
        return false;
    }

    for (uint32_t curOffset = offset; curOffset < (offset + erase_size); curOffset += MEMFAULT_FLASH_SECTOR_SIZE)
    {
        if (kStatus_Success != mflash_drv_sector_erase(MEMFAULT_PHYS_ADDR(curOffset)))
        {
            return false;
        }
    }

    return true;
}

bool memfault_platform_coredump_storage_buffered_write(sCoredumpWorkingBuffer *blk) {
    if (!prv_op_within_flash_bounds(blk->write_offset, MEMFAULT_COREDUMP_STORAGE_WRITE_SIZE))
    {
        return false;
    }

    return (kStatus_Success == mflash_drv_page_program(MEMFAULT_PHYS_ADDR(blk->write_offset), (uint32_t*)(blk->data)));
}

void memfault_platform_coredump_storage_clear(void) {
    (void)memfault_platform_coredump_storage_erase(0, MEMFAULT_FLASH_SECTOR_SIZE);
}
