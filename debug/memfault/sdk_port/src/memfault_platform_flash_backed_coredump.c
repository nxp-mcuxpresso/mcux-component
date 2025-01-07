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

/*
 * A flash region shall be reserved for memfault. This flash region is specified
 * by such config macros, which should be defined by the project.
 *
 * CONFIG_MEMFAULT_FLASH_BASE_OFFSET: Offset of flash region start address
 * CONFIG_MEMFAULT_FLASH_SIZE: Size of the flash region
 */

#define MEMFAULT_FLASH_SECTOR_SIZE MFLASH_SECTOR_SIZE
#define MEMFAULT_FLASH_PAGE_SIZE MFLASH_PAGE_SIZE
#define MEMFAULT_PHYS_ADDR(offset)    (CONFIG_MEMFAULT_FLASH_BASE_OFFSET + (offset)) /* physical address. */
#define MEMFAULT_FLASH_LOG_ADDR(offset) (MFLASH_BASE_ADDRESS + MEMFAULT_PHYS_ADDR(offset)) /* Logical address. */

/* Memfault writing content is not page size aligned, so need to read the page,
 * chagne the content, and then write it back.
 * Some platforms don't support program one page multiple times, so use workaround here.
 * Memfault uses such workflow to write data to flash:
 * 1. Write first data from offset sizeof(sMfltCoredumpHeader)
 * 2. Write data continusly to the following address.
 * 3. Write the core dump header to the offset 0.
 * The workaround is, use dedicated page buffer s_memfaultFlashPage0Buffer for
 * the page at offset 0, don't write it to flash until memfault writes the header.
 * For other pages, save the data in s_memfaultFlashPageBuffer, and only write it
 * to flash when memfault is writing the next page.
 */
#if !defined(CONFIG_MEMFAULT_FLASH_SUPPORT_PAGE_REPROGRAM)
#define CONFIG_MEMFAULT_FLASH_SUPPORT_PAGE_REPROGRAM 0
#endif

static uint8_t s_memfaultFlashPageBuffer[MEMFAULT_FLASH_PAGE_SIZE];

#if !CONFIG_MEMFAULT_FLASH_SUPPORT_PAGE_REPROGRAM
static uint8_t s_memfaultFlashPage0Buffer[MEMFAULT_FLASH_PAGE_SIZE];
static bool s_memfaultPageBuffer0Valid = false;
static bool s_memfaultPageBufferValid = false;
static uint32_t s_memfaultPageBufferOffset = 0; /* Offset of data in s_memfaultFlashPageBuffer. */
static uint32_t s_memfaultWriteEnd = 0;
#endif

void memfault_platform_coredump_storage_get_info(sMfltCoredumpStorageInfo *info)
{
    *info = (sMfltCoredumpStorageInfo){
        .size        = CONFIG_MEMFAULT_FLASH_SIZE,
        .sector_size = MEMFAULT_FLASH_SECTOR_SIZE,
    };
}

static bool prv_op_within_flash_bounds(uint32_t offset, size_t data_len)
{
	return (offset + data_len) <= CONFIG_MEMFAULT_FLASH_SIZE;
}

bool memfault_platform_coredump_storage_read_from_flash(uint32_t offset, void *data, size_t read_len)
{
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

bool memfault_platform_coredump_storage_read(uint32_t offset, void *data, size_t read_len)
{
    if (!prv_op_within_flash_bounds(offset, read_len))
    {
        return false;
    }

#if CONFIG_MEMFAULT_FLASH_SUPPORT_PAGE_REPROGRAM
    return memfault_platform_coredump_storage_read_from_flash(offset, data, read_len);

#else
    uint32_t pageAlignedOffset = offset & ~(MEMFAULT_FLASH_PAGE_SIZE - 1U);
    uint32_t remaining_sz      = read_len;
    uint32_t readOffsetInPage = offset - pageAlignedOffset;
    uint8_t * dest = data;

    while (remaining_sz > 0)
    {
        uint32_t readLenInPage = MEMFAULT_FLASH_PAGE_SIZE - readOffsetInPage;

        if (readLenInPage > remaining_sz)
        {
            readLenInPage = remaining_sz;
        }

        if ((pageAlignedOffset == 0) && s_memfaultPageBuffer0Valid)
        {
            memcpy(dest, s_memfaultFlashPage0Buffer + readOffsetInPage, readLenInPage);
        }
        else if ((pageAlignedOffset == s_memfaultPageBufferOffset) && s_memfaultPageBufferValid)
        {
            memcpy(dest, s_memfaultFlashPageBuffer + readOffsetInPage, readLenInPage);
        }
        else
        {
            if (!memfault_platform_coredump_storage_read_from_flash(offset, dest, readLenInPage))
            {
                return false;
            }
        }

        remaining_sz      -= readLenInPage;
        dest              += readLenInPage;
        offset            += readLenInPage;
        pageAlignedOffset += MEMFAULT_FLASH_PAGE_SIZE;
        readOffsetInPage  = 0;
    }

    return true;
#endif
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

#if !CONFIG_MEMFAULT_FLASH_SUPPORT_PAGE_REPROGRAM
    uint32_t sectorStart = offset & ~(MEMFAULT_FLASH_SECTOR_SIZE - 1U);
    uint32_t sectorEnd   = ((offset + erase_size + MEMFAULT_FLASH_SECTOR_SIZE - 1U) & ~(MEMFAULT_FLASH_SECTOR_SIZE - 1U));
    if (0 == sectorStart)
    {
        s_memfaultPageBuffer0Valid = false;
    }
    if ((s_memfaultPageBufferOffset >= sectorStart) && (s_memfaultPageBufferOffset < sectorEnd))
    {
        s_memfaultPageBufferValid = false;
    }
#endif

    for (uint32_t curOffset = offset; curOffset < (offset + erase_size); curOffset += MEMFAULT_FLASH_SECTOR_SIZE)
    {
        if (kStatus_Success != mflash_drv_sector_erase(MEMFAULT_PHYS_ADDR(curOffset)))
        {
            return false;
        }
    }

    return true;
}

bool memfault_platform_coredump_storage_write(uint32_t offset, const void *data, size_t data_len)
{
    status_t status;

    if (!prv_op_within_flash_bounds(offset, data_len))
    {
        return false;
    }

#if CONFIG_MEMFAULT_FLASH_SUPPORT_PAGE_REPROGRAM
    do
    {
        uint32_t page_addr;
        const uint8_t *src;
        uint32_t write_len;
        uint32_t remaining_sz;
        uint32_t address;

        address = MEMFAULT_PHYS_ADDR(offset);
        src = (const uint8_t *)data;

        remaining_sz = data_len;

        page_addr = address & ~(MEMFAULT_FLASH_PAGE_SIZE - 1U);
        if (page_addr != address)
        {
            uint32_t write_offset;

            status = mflash_drv_read(page_addr, (uint32_t *)s_memfaultFlashPageBuffer, MEMFAULT_FLASH_PAGE_SIZE);
            if (kStatus_Success != status)
            {
                break;
            }
            write_offset = (address - page_addr);
            write_len    = MEMFAULT_FLASH_PAGE_SIZE - write_offset;
            if (write_len > data_len)
            {
                write_len = data_len;
            }
            memcpy(&s_memfaultFlashPageBuffer[write_offset], src, write_len);
            status = mflash_drv_page_program(page_addr, (uint32_t *)&s_memfaultFlashPageBuffer[0]);
            if (kStatus_Success != status)
            {
                break;
            }
            src += write_len;
            page_addr += MEMFAULT_FLASH_PAGE_SIZE;
            remaining_sz -= write_len;
        }

        /* From now the address in flash is page aligned */
        while (remaining_sz >= MEMFAULT_FLASH_PAGE_SIZE)
        {
            memcpy(s_memfaultFlashPageBuffer, src, MEMFAULT_FLASH_PAGE_SIZE);
            status = mflash_drv_page_program(page_addr, (uint32_t *)&s_memfaultFlashPageBuffer[0]);
            if (status != kStatus_Success)
            {
                break;
            }
            remaining_sz -= MEMFAULT_FLASH_PAGE_SIZE;
            src += MEMFAULT_FLASH_PAGE_SIZE;
            page_addr += MEMFAULT_FLASH_PAGE_SIZE;
        }

        if (remaining_sz > 0u)
        {
            /* Copy partial contents of page - if any to RAM */
            status = mflash_drv_read(page_addr, (uint32_t *)s_memfaultFlashPageBuffer, MEMFAULT_FLASH_PAGE_SIZE);
            if (kStatus_Success != status)
            {
                break;
            }

            memcpy(s_memfaultFlashPageBuffer, src, remaining_sz);

            status = mflash_drv_page_program(page_addr, (uint32_t *)&s_memfaultFlashPageBuffer[0]);
            if (kStatus_Success != status)
            {
                break;
            }
        }

        status = kStatus_Success;
    } while (false);

    if (kStatus_Success != status)
    {
        return false;
    }
    else
    {
        return true;
    }

#else
    /* Write to offset 0, which means this is the last write (writing sMfltCoredumpHeader), flush all the data in buffer to flash. */
    if (0u == offset)
    {
        /* Assume sMfltCoredumpHeader is less than 1 page size. */
        if (data_len > MEMFAULT_FLASH_PAGE_SIZE)
        {
            return false;
        }

        if (s_memfaultPageBufferValid)
        {
            status = mflash_drv_page_program(MEMFAULT_PHYS_ADDR(s_memfaultPageBufferOffset), (uint32_t *)&s_memfaultFlashPageBuffer[0]);
            if (kStatus_Success != status)
            {
                return false;
            }
        }

        memcpy(&s_memfaultFlashPage0Buffer[0], data, data_len);
        status = mflash_drv_page_program(MEMFAULT_PHYS_ADDR(0U), (uint32_t *)&s_memfaultFlashPage0Buffer[0]);

        return (kStatus_Success == status);
    }

    /*
     * Assumption: Memfault writes the memory continuously, so current write must starts from
     * previous finished place.
     */
    if ((offset != s_memfaultWriteEnd) && (s_memfaultWriteEnd != 0))
    {
        return false;
    }

    uint32_t remaining_sz      = data_len;
    uint32_t pageAlignedOffset = offset & ~(MEMFAULT_FLASH_PAGE_SIZE - 1U);
    uint32_t writeOffsetInPage = offset - pageAlignedOffset;
    const uint8_t * src = data;

    while (remaining_sz > 0)
    {
        uint8_t *curUsedBuf;
        uint32_t writeLenInPage;

        if (pageAlignedOffset == 0U)
        {
            curUsedBuf = &s_memfaultFlashPage0Buffer[0];
            s_memfaultPageBuffer0Valid = true;
        }
        else
        {
            curUsedBuf = &s_memfaultFlashPageBuffer[0];
            s_memfaultPageBufferValid = true;
        }

        writeLenInPage    = MEMFAULT_FLASH_PAGE_SIZE - writeOffsetInPage;

        if (writeLenInPage > remaining_sz)
        {
            writeLenInPage = remaining_sz;
        }

        memcpy(curUsedBuf + writeOffsetInPage, src, writeLenInPage);
        memset(curUsedBuf + writeOffsetInPage + writeLenInPage, 0xFF, MEMFAULT_FLASH_PAGE_SIZE - (writeLenInPage + writeOffsetInPage));

        /* Record the data offset in page buffer. */
        s_memfaultPageBufferOffset = pageAlignedOffset;

        /* If the page is full, flush the page to flash. */
        if (((writeLenInPage + writeOffsetInPage) == MEMFAULT_FLASH_PAGE_SIZE) && (pageAlignedOffset != 0U))
        {
            status = mflash_drv_page_program(MEMFAULT_PHYS_ADDR(pageAlignedOffset), (uint32_t *)curUsedBuf);
            if (kStatus_Success != status)
            {
                return false;
            }
        }

        remaining_sz      -= writeLenInPage;
        src               += writeLenInPage;
        pageAlignedOffset += MEMFAULT_FLASH_PAGE_SIZE;
        writeOffsetInPage  = 0;
    }

    s_memfaultWriteEnd = offset + data_len;

    return true;
#endif
}

void memfault_platform_coredump_storage_clear(void) {
#if CONFIG_MEMFAULT_FLASH_SUPPORT_PAGE_REPROGRAM
    const uint8_t clear_byte = 0x0;
    memfault_platform_coredump_storage_write(0, &clear_byte, sizeof(clear_byte));
#else
    memfault_platform_coredump_storage_erase(0, MEMFAULT_FLASH_SECTOR_SIZE);
    s_memfaultWriteEnd = 0;
#endif
}
