/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/debug/coredump.h>
#include "coredump_internal.h"

#include "mflash_common.h"
#include "mflash_file.h"

#ifndef CONFIG_MFLASH_COREDUMP_REGION_START_ADDR
#error "coredump region start address not defined!"
#endif

#ifndef CONFIG_MFLASH_COREDUMP_REGION_SIZE
#error "coredump region size not defined!"
#endif

#define MFLASH_COREDUMP_REGION_SECTOR_NUM (CONFIG_MFLASH_COREDUMP_REGION_SIZE / MFLASH_SECTOR_SIZE)

struct backend_mflash_ctx
{
    uint8_t pageBuffer[MFLASH_PAGE_SIZE];
    uint16_t checksum;
    size_t bytesWritten;
    uint8_t pageBufferOffset;
    uint32_t addrOffset;
    uint32_t bytesAvailble;
} backend_ctx;

struct flash_hdr_t
{
    /* 'C', 'D' */
    char id[2];

    /* Header version */
    uint16_t hdr_version;

    /* Coredump size, excluding this header */
    size_t size;

    /* Flags */
    uint16_t flags;

    /* Checksum */
    uint16_t checksum;

    /* Error */
    int32_t error;
} __packed;

typedef int32_t (*data_read_cb_t)(void *arg, uint8_t *buf, size_t len);

static void coredump_mflash_page_sync(uint8_t *buf, size_t bufLen, bool flush, bool rmw)
{
    /* step 1: The zephyr header start from 0x10 */
    /* step 2: write data into flash, and this addr is consecutive. */
    /* step 3: coredump header start from 0x0. */

    int32_t status   = 0;
    uint32_t addr    = backend_ctx.addrOffset;
    size_t remaining = bufLen;
    uint8_t *ptr     = buf;
    size_t copySize  = 0UL;

    if ((bufLen + backend_ctx.bytesWritten + backend_ctx.pageBufferOffset) > backend_ctx.bytesAvailble)
    {
        assert(false);
        return;
    }

    if (rmw == true)
    {
        status = mflash_drv_read(addr, (uint32_t *)(uint32_t)(backend_ctx.pageBuffer), MFLASH_PAGE_SIZE);
        if ((status_t)status != kStatus_Success)
        {
            assert(false);
            return;
        }
        memcpy(backend_ctx.pageBuffer, ptr, backend_ctx.pageBufferOffset);
        status = mflash_drv_page_program(addr, (uint32_t *)(uint32_t)(backend_ctx.pageBuffer));
        if ((status_t)status != kStatus_Success)
        {
            assert(false);
            return;
        }
        backend_ctx.pageBufferOffset = 0U;
        return;
    }

    while (remaining > 0UL)
    {
        copySize = (((remaining + backend_ctx.pageBufferOffset) <= MFLASH_PAGE_SIZE) ?
                        remaining :
                        (MFLASH_PAGE_SIZE - backend_ctx.pageBufferOffset));

        memcpy(backend_ctx.pageBuffer + backend_ctx.pageBufferOffset, ptr, copySize);
        for (uint32_t i = 0UL; i < copySize; i++)
        {
            backend_ctx.checksum += backend_ctx.pageBuffer[backend_ctx.pageBufferOffset + i];
        }

        backend_ctx.pageBufferOffset = (backend_ctx.pageBufferOffset + copySize) % MFLASH_PAGE_SIZE;
        if (backend_ctx.pageBufferOffset == 0U)
        {
            status = mflash_drv_page_program(addr, (uint32_t *)(uint32_t)(backend_ctx.pageBuffer));
            if ((status_t)status != kStatus_Success)
            {
                assert(false);
                return;
            }
            addr += MFLASH_PAGE_SIZE;
        }
        remaining -= copySize;
        ptr += copySize;
    }

    if (flush)
    {
        if (backend_ctx.pageBufferOffset != 0U)
        {
            for (uint32_t i = backend_ctx.pageBufferOffset; i < MFLASH_PAGE_SIZE; i++)
            {
                backend_ctx.pageBuffer[i] = 0xFFU;
            }
        }
        status = mflash_drv_page_program(addr, (uint32_t *)(uint32_t)(backend_ctx.pageBuffer));
        if ((status_t)status != kStatus_Success)
        {
            assert(false);
            return;
        }
        addr += MFLASH_PAGE_SIZE;
        backend_ctx.pageBufferOffset = 0U;
    }

    backend_ctx.addrOffset = addr;
    backend_ctx.bytesWritten += bufLen;
}

static void coredump_mflash_backend_start(void)
{
    assert(MFLASH_COREDUMP_REGION_SECTOR_NUM != 0UL);

    status_t status;
    size_t headerSize = sizeof(struct flash_hdr_t);

    if (mflash_is_initialized() == false)
    {
        status = mflash_drv_init();
        if (status == kStatus_Fail)
        {
            assert(false);
            return;
        }
    }

    backend_ctx.bytesAvailble = 0UL;
    for (uint32_t i = 0U; i < MFLASH_COREDUMP_REGION_SECTOR_NUM; i++)
    {
        status = mflash_drv_sector_erase(CONFIG_MFLASH_COREDUMP_REGION_START_ADDR + i * MFLASH_SECTOR_SIZE);
        if (status != kStatus_Success)
        {
            assert(false);
            return;
        }
        backend_ctx.bytesAvailble += MFLASH_SECTOR_SIZE;
    }

    memset(backend_ctx.pageBuffer, 0xFF, MFLASH_PAGE_SIZE);
    backend_ctx.checksum         = 0U;
    backend_ctx.bytesWritten     = 0UL;
    backend_ctx.pageBufferOffset = headerSize;
    backend_ctx.addrOffset       = CONFIG_MFLASH_COREDUMP_REGION_START_ADDR;
}

static void coredump_mflash_backend_end(void)
{
    struct flash_hdr_t hdr = {
        .id          = {'C', 'D'},
        .hdr_version = 1U,
    };

    hdr.size     = backend_ctx.bytesWritten;
    hdr.checksum = backend_ctx.checksum;
    hdr.flags    = 0;
    hdr.error    = 0;

    coredump_mflash_page_sync(NULL, 0U, true, false);
    backend_ctx.addrOffset       = CONFIG_MFLASH_COREDUMP_REGION_START_ADDR;
    backend_ctx.pageBufferOffset = sizeof(struct flash_hdr_t);
    coredump_mflash_page_sync((uint8_t *)&hdr, sizeof(hdr), false, true);
}

static void coredump_mflash_backend_buffer_output(uint8_t *buf, size_t buflen)
{
    coredump_mflash_page_sync(buf, buflen, false, false);
}

static int32_t coredump_mflash_calc_buf_checksum(void *arg, uint8_t *buf, size_t len)
{
    for (uint32_t i = 0UL; i < len; i++)
    {
        backend_ctx.checksum += buf[i];
    }

    (void)arg;
    return 0;
}

static int32_t coredump_mflash_process_stored_dump(data_read_cb_t cb, void *cb_arg)
{
    struct flash_hdr_t hdr;
    uint32_t addr = CONFIG_MFLASH_COREDUMP_REGION_START_ADDR;

    int32_t ret = 0;

    if (mflash_drv_read(addr, (uint32_t *)(uint32_t)&hdr, sizeof(hdr)) != kStatus_Success)
    {
        assert(false);
        return 0;
    }

    if ((hdr.id[0] != 'C') && (hdr.id[1] != 'D'))
    {
        return 0;
    }

    if (hdr.error != 0)
    {
        return 0;
    }

    backend_ctx.checksum = 0U;

    size_t remaining  = hdr.size;
    size_t copySize   = 0UL;
    size_t adjustSize = 0UL;
    addr += sizeof(hdr);
    while (remaining > 0)
    {
        copySize = MFLASH_PAGE_SIZE;
        if (remaining > MFLASH_PAGE_SIZE)
        {
            adjustSize = 0UL;
        }
        else
        {
            adjustSize = MFLASH_PAGE_SIZE - remaining;
        }

        if (mflash_drv_read(addr, (uint32_t *)(uint32_t)backend_ctx.pageBuffer, copySize) != kStatus_Success)
        {
            assert(false);
            return 0;
        }
        if (cb != NULL)
        {
            ret = (*cb)(cb_arg, (uint8_t *)(backend_ctx.pageBuffer), (MFLASH_PAGE_SIZE - adjustSize));
            if (ret < 0)
            {
                break;
            }
        }

        remaining -= (copySize - adjustSize);
        addr += copySize;
    }

    if (ret == 0)
    {
        ret = (backend_ctx.checksum == hdr.checksum) ? 1 : 0;
    }

    return ret;
}

static int32_t coredump_mflash_get_stored_dump(uint32_t addr, uint8_t *dst, size_t len)
{
    struct flash_hdr_t hdr;
    uint32_t tmpAddr = ((addr == 0UL) ? CONFIG_MFLASH_COREDUMP_REGION_START_ADDR : addr);

    int32_t ret;

    if (mflash_drv_read(tmpAddr, (uint32_t *)(uint32_t)&hdr, sizeof(hdr)) != kStatus_Success)
    {
        return 0;
    }

    if ((hdr.id[0] != 'C') && (hdr.id[1] != 'D'))
    {
        return 0;
    }

    if (hdr.error != 0)
    {
        return 0;
    }

    /* Return the dump size if no destination buffer available. */
    if (dst == NULL)
    {
        ret = (int32_t)hdr.size;
        return ret;
    }

    tmpAddr += sizeof(struct flash_hdr_t);
    if (mflash_drv_read(tmpAddr, (uint32_t *)(uint32_t)dst, len) != kStatus_Success)
    {
        return 0;
    }

    /* Return size of buffer. */
    ret = len;

    return ret;
}

static int32_t coredump_mflash_backend_query(enum coredump_query_id query_id, void *arg)
{
    int32_t ret;

    switch (query_id)
    {
        case COREDUMP_QUERY_GET_ERROR:
        {
            /* Not support yet. */
            ret = 0;
            break;
        }
        case COREDUMP_QUERY_HAS_STORED_DUMP:
        {
            ret = coredump_mflash_process_stored_dump(coredump_mflash_calc_buf_checksum, NULL);
            break;
        }
        case COREDUMP_QUERY_GET_STORED_DUMP_SIZE:
        {
            ret = coredump_mflash_get_stored_dump(0UL, NULL, 0UL);
            break;
        }
        default:
        {
            ret = -ENOTSUP;
            break;
        }
    }

    return ret;
}

static int32_t coredump_mflash_backend_cmd(enum coredump_cmd_id cmd_id, void *arg)
{
    int32_t ret;

    switch (cmd_id)
    {
        case COREDUMP_CMD_CLEAR_ERROR:
        {
            /* Not support yet. */
            ret = 0;
            break;
        }
        case COREDUMP_CMD_VERIFY_STORED_DUMP:
        {
            ret = coredump_mflash_process_stored_dump(coredump_mflash_calc_buf_checksum, NULL);
            break;
        }
        case COREDUMP_CMD_ERASE_STORED_DUMP:
        {
            for (uint32_t i = 0U; i < MFLASH_COREDUMP_REGION_SECTOR_NUM; i++)
            {
                if (mflash_drv_sector_erase(CONFIG_MFLASH_COREDUMP_REGION_START_ADDR + i * MFLASH_SECTOR_SIZE) !=
                    kStatus_Success)
                {
                    ret = -EINVAL;
                    break;
                }
            }
            ret = 0;
            break;
        }
        case COREDUMP_CMD_COPY_STORED_DUMP:
        {
            if (arg)
            {
                struct coredump_cmd_copy_arg *copy_arg = (struct coredump_cmd_copy_arg *)arg;

                ret = coredump_mflash_get_stored_dump(copy_arg->offset, copy_arg->buffer, copy_arg->length);
            }
            else
            {
                ret = -EINVAL;
            }
            break;
        }
        case COREDUMP_CMD_INVALIDATE_STORED_DUMP:
        {
            if (mflash_drv_sector_erase(CONFIG_MFLASH_COREDUMP_REGION_START_ADDR) != kStatus_Success)
            {
                ret = -EINVAL;
            }
            ret = 0;
            break;
        }
        default:
        {
            ret = -ENOTSUP;
            break;
        }
    }

    return ret;
}

struct coredump_backend_api coredump_backend_other = {
    .start         = coredump_mflash_backend_start,
    .end           = coredump_mflash_backend_end,
    .buffer_output = coredump_mflash_backend_buffer_output,
    .query         = coredump_mflash_backend_query,
    .cmd           = coredump_mflash_backend_cmd,
};
