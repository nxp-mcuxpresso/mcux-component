/*
 * Copyright (c) 2013-2020 Arm Limited. All rights reserved.
 * Copyright 2026 NXP. Not a Contribution.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * CMSIS Flash driver for NXP MCX devices based on ROM API
 *
 * Driver_Flash0 -> Main Flash
 * Driver_Flash1 -> IFR0 (Not applicable for all devices, may be used completely by the ROM)
 *
 * Address model:
 *   - CMSIS flash address is physical, starting from 0
 *   - NXP ROM APIs receive absolute addresses, starting from FSL_FEATURE_FLASH_PFLASH_START_ADDRESS
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "Driver_Flash.h"
#include "fsl_flash.h"

/*
 * Expected device feature macros from the device header:
 *
 * FSL_FEATURE_FLASH_PFLASH_START_ADDRESS
 * FSL_FEATURE_FLASH_PFLASH_BLOCK_SIZE
 * FSL_FEATURE_FLASH_PFLASH_SECTOR_SIZE
 * FSL_FEATURE_FLASH_PFLASH_PAGE_SIZE
 * FSL_FEATURE_FLASH_PFLASH_PHRASE_SIZE
 * FSL_FEATURE_FLASH_IFR0_START_ADDRESS
 * FSL_FEATURE_FLASH_IFR0_SIZE
 * FSL_FEATURE_FLASH_IFR_SECTOR_SIZE
 * FSL_FEATURE_FLASH_IFR_PAGE_SIZE
 */

#ifndef ARM_FLASH_DRV_VERSION
#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)
#endif

#define MCX_FLASH_ERASED_VALUE (0xFFU)

typedef struct
{
    uint32_t base;
    uint32_t size;
    bool supports_phrase_program;
    bool initialized;

    flash_config_t nxp_cfg;
    ARM_Flash_SignalEvent_t cb_event;
    ARM_FLASH_STATUS status;
    ARM_FLASH_INFO info;
} mcx_flash_ctx_t;

/* -------------------------------------------------------------------------- */
/* Status mapping                                                              */
/* -------------------------------------------------------------------------- */

static int32_t NxpStatusToArm(status_t st)
{
    switch (st)
    {
        case kStatus_FLASH_Success:
            return ARM_DRIVER_OK;

        case kStatus_FLASH_InvalidArgument:
        case kStatus_FLASH_SizeError:
        case kStatus_FLASH_AlignmentError:
        case kStatus_FLASH_AddressError:
        case kStatus_FLASH_EraseKeyError:
            return ARM_DRIVER_ERROR_PARAMETER;

        case kStatus_FLASH_CommandNotSupported:
        case kStatus_FLASH_ReadOnlyProperty:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        case kStatus_FLASH_CommandOperationInProgress:
            return ARM_DRIVER_ERROR_BUSY;

        default:
            return ARM_DRIVER_ERROR;
    }
}

/* -------------------------------------------------------------------------- */
/* Helpers                                                                     */
/* -------------------------------------------------------------------------- */

static bool FlashRangeValid(const mcx_flash_ctx_t *ctx, uint32_t addr, uint32_t cnt)
{
    if (cnt == 0U)
    {
        return true;
    }

    if (addr >= ctx->size)
    {
        return false;
    }

    return (cnt <= (ctx->size - addr));
}

static uint32_t ToAbsoluteAddr(const mcx_flash_ctx_t *ctx, uint32_t addr)
{
    return ctx->base + addr;
}

static int32_t EnsureReady(mcx_flash_ctx_t *ctx)
{
    if (!ctx->initialized)
    {
        return ARM_DRIVER_ERROR;
    }
    return ARM_DRIVER_OK;
}

static int32_t InitCommon(mcx_flash_ctx_t *ctx, ARM_Flash_SignalEvent_t cb_event)
{
    status_t st;

    ctx->cb_event = cb_event;
    ctx->status.busy = 0U;
    ctx->status.error = 0U;

    st = FLASH_Init(&ctx->nxp_cfg);
    if (st != kStatus_FLASH_Success)
    {
        ctx->status.error = 1U;
        return NxpStatusToArm(st);
    }

    ctx->initialized = true;
    return ARM_DRIVER_OK;
}

static int32_t UninitCommon(mcx_flash_ctx_t *ctx)
{
    ctx->initialized = false;
        ctx->cb_event = NULL;
        ctx->status.busy = 0U;
        ctx->status.error = 0U;
        return ARM_DRIVER_OK;
}

static int32_t PowerCommon(mcx_flash_ctx_t *ctx, ARM_POWER_STATE state)
{
    switch (state)
    {
        case ARM_POWER_OFF:
            break;

        case ARM_POWER_LOW:
            break;

        case ARM_POWER_FULL:
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ARM_DRIVER_OK;
}

static int32_t ReadCommon(mcx_flash_ctx_t *ctx, uint32_t addr, void *data, uint32_t cnt)
{
    status_t st;
    uint32_t abs_addr;

    if ((data == NULL) || (!FlashRangeValid(ctx, addr, cnt)))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (EnsureReady(ctx) != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    abs_addr = ToAbsoluteAddr(ctx, addr);

    ctx->status.busy = 1U;
    ctx->status.error = 0U;

    st = FLASH_Read(&ctx->nxp_cfg, abs_addr, (uint8_t *)data, cnt);

    ctx->status.busy = 0U;
    if (st != kStatus_FLASH_Success)
    {
        ctx->status.error = 1U;
        return NxpStatusToArm(st);
    }

    /* data_width = 8-bit => cnt is in bytes */
    return (int32_t)cnt;
}

static int32_t ProgramCommon(mcx_flash_ctx_t *ctx, uint32_t addr, const void *data, uint32_t cnt)
{
    status_t st;
    const uint8_t *src = (const uint8_t *)data;
    uint32_t abs_addr;
    uint32_t chunk;

    if ((data == NULL) || (cnt == 0U))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (!FlashRangeValid(ctx, addr, cnt))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (((addr % ctx->info.program_unit) != 0U) || ((cnt % ctx->info.program_unit) != 0U))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (EnsureReady(ctx) != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    ctx->status.busy = 1U;
    ctx->status.error = 0U;

    while (cnt != 0U)
    {
        abs_addr = ToAbsoluteAddr(ctx, addr);

        /*
         * If phrase programming is supported (main flash), use page program for full
         * aligned pages when possible, otherwise use phrase programming.
         *
         * If phrase programming is not supported program only by page.
         */
        if (ctx->supports_phrase_program)
        {
            if (((addr % ctx->info.page_size) == 0U) && (cnt >= ctx->info.page_size))
            {
                chunk = ctx->info.page_size;
                st = FLASH_ProgramPage(&ctx->nxp_cfg, abs_addr, (uint8_t *)(uintptr_t)src, chunk);
            }
            else
            {
                chunk = ctx->info.program_unit;
                st = FLASH_ProgramPhrase(&ctx->nxp_cfg, abs_addr, (uint8_t *)(uintptr_t)src, chunk);
            }
        }
        else
        {
            chunk = ctx->info.page_size;
            st = FLASH_ProgramPage(&ctx->nxp_cfg, abs_addr, (uint8_t *)(uintptr_t)src, chunk);
        }

        if (st != kStatus_FLASH_Success)
        {
            ctx->status.busy = 0U;
            ctx->status.error = 1U;
            return NxpStatusToArm(st);
        }

        addr += chunk;
        src  += chunk;
        cnt  -= chunk;
    }

    ctx->status.busy = 0U;
    return (int32_t)(src - (const uint8_t *)data);
}

static int32_t EraseSectorCommon(mcx_flash_ctx_t *ctx, uint32_t addr)
{
    status_t st;
    uint32_t abs_addr;

    if (!FlashRangeValid(ctx, addr, ctx->info.sector_size))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if ((addr % ctx->info.sector_size) != 0U)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (EnsureReady(ctx) != ARM_DRIVER_OK)
    {
        return ARM_DRIVER_ERROR;
    }

    abs_addr = ToAbsoluteAddr(ctx, addr);

    ctx->status.busy = 1U;
    ctx->status.error = 0U;

    st = FLASH_EraseSector(&ctx->nxp_cfg, abs_addr, ctx->info.sector_size, kFLASH_ApiEraseKey);

    ctx->status.busy = 0U;
    if (st != kStatus_FLASH_Success)
    {
        ctx->status.error = 1U;
        return NxpStatusToArm(st);
    }

    return ARM_DRIVER_OK;
}

static int32_t EraseChipCommon(mcx_flash_ctx_t *ctx)
{
    (void)ctx;
    /* No full-chip erase API in the ROM Flash layer */
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/* -------------------------------------------------------------------------- */
/* Static instance data                                                        */
/* -------------------------------------------------------------------------- */

static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

static const ARM_FLASH_CAPABILITIES DriverCapabilities = {
    0U, /* event_ready: synchronous implementation */
    0U, /* data_width = 0 => 8-bit */
    0U, /* erase_chip not supported */
    0U
};

static mcx_flash_ctx_t g_main = {
    .base = FSL_FEATURE_FLASH_PFLASH_START_ADDRESS,
    .size = FSL_FEATURE_FLASH_PFLASH_BLOCK_SIZE,
    .supports_phrase_program = true,
    .initialized = false,
    .cb_event = NULL,
    .status = {0U, 0U, 0U},
    .info = {
        .sector_info  = NULL,
        .sector_count = (FSL_FEATURE_FLASH_PFLASH_BLOCK_SIZE / FSL_FEATURE_FLASH_PFLASH_SECTOR_SIZE),
        .sector_size  = FSL_FEATURE_FLASH_PFLASH_SECTOR_SIZE,
        .page_size    = FSL_FEATURE_FLASH_PFLASH_PAGE_SIZE,
        .program_unit = FSL_FEATURE_FLASH_PFLASH_PHRASE_SIZE,
        .erased_value = MCX_FLASH_ERASED_VALUE,
        .reserved     = {0U, 0U, 0U}
    }
};

static mcx_flash_ctx_t g_ifr0 = {
    .base = FSL_FEATURE_FLASH_IFR0_START_ADDRESS,
    .size = FSL_FEATURE_FLASH_IFR0_SIZE,
    .supports_phrase_program = true,
    .initialized = false,
    .cb_event = NULL,
    .status = {0U, 0U, 0U},
    .info = {
        .sector_info  = NULL,
        .sector_count = (FSL_FEATURE_FLASH_IFR0_SIZE / FSL_FEATURE_FLASH_IFR_SECTOR_SIZE),
        .sector_size  = FSL_FEATURE_FLASH_IFR_SECTOR_SIZE,
        .page_size    = FSL_FEATURE_FLASH_IFR_PAGE_SIZE,
        .program_unit = 16, /* not provided in the feature header */
        .erased_value = MCX_FLASH_ERASED_VALUE,
        .reserved     = {0U, 0U, 0U}
    }
};

/* -------------------------------------------------------------------------- */
/* Driver_Flash0 : Main PFlash                                                 */
/* -------------------------------------------------------------------------- */

static ARM_DRIVER_VERSION Main_GetVersion(void)                { return DriverVersion; }
static ARM_FLASH_CAPABILITIES Main_GetCapabilities(void)       { return DriverCapabilities; }
static int32_t Main_Initialize(ARM_Flash_SignalEvent_t cb)     { return InitCommon(&g_main, cb); }
static int32_t Main_Uninitialize(void)                         { return UninitCommon(&g_main); }
static int32_t Main_PowerControl(ARM_POWER_STATE state)        { return PowerCommon(&g_main, state); }
static int32_t Main_ReadData(uint32_t a, void *d, uint32_t c)  { return ReadCommon(&g_main, a, d, c); }
static int32_t Main_ProgramData(uint32_t a, const void *d, uint32_t c)
                                                                { return ProgramCommon(&g_main, a, d, c); }
static int32_t Main_EraseSector(uint32_t a)                    { return EraseSectorCommon(&g_main, a); }
static int32_t Main_EraseChip(void)                            { return EraseChipCommon(&g_main); }
static ARM_FLASH_STATUS Main_GetStatus(void)                   { return g_main.status; }
static ARM_FLASH_INFO *Main_GetInfo(void)                      { return &g_main.info; }

/* -------------------------------------------------------------------------- */
/* Driver_Flash1 : IFR0                                                        */
/* -------------------------------------------------------------------------- */

static ARM_DRIVER_VERSION Ifr0_GetVersion(void)                { return DriverVersion; }
static ARM_FLASH_CAPABILITIES Ifr0_GetCapabilities(void)       { return DriverCapabilities; }
static int32_t Ifr0_Initialize(ARM_Flash_SignalEvent_t cb)     { return InitCommon(&g_ifr0, cb); }
static int32_t Ifr0_Uninitialize(void)                         { return UninitCommon(&g_ifr0); }
static int32_t Ifr0_PowerControl(ARM_POWER_STATE state)        { return PowerCommon(&g_ifr0, state); }
static int32_t Ifr0_ReadData(uint32_t a, void *d, uint32_t c)  { return ReadCommon(&g_ifr0, a, d, c); }
static int32_t Ifr0_ProgramData(uint32_t a, const void *d, uint32_t c)
                                                                { return ProgramCommon(&g_ifr0, a, d, c); }
static int32_t Ifr0_EraseSector(uint32_t a)                    { return EraseSectorCommon(&g_ifr0, a); }
static int32_t Ifr0_EraseChip(void)                            { return EraseChipCommon(&g_ifr0); }
static ARM_FLASH_STATUS Ifr0_GetStatus(void)                   { return g_ifr0.status; }
static ARM_FLASH_INFO *Ifr0_GetInfo(void)                      { return &g_ifr0.info; }

/* -------------------------------------------------------------------------- */
/* Exported driver access structures                                           */
/* -------------------------------------------------------------------------- */

ARM_DRIVER_FLASH Driver_Flash0 = {
    Main_GetVersion,
    Main_GetCapabilities,
    Main_Initialize,
    Main_Uninitialize,
    Main_PowerControl,
    Main_ReadData,
    Main_ProgramData,
    Main_EraseSector,
    Main_EraseChip,
    Main_GetStatus,
    Main_GetInfo
};

ARM_DRIVER_FLASH Driver_Flash1 = {
    Ifr0_GetVersion,
    Ifr0_GetCapabilities,
    Ifr0_Initialize,
    Ifr0_Uninitialize,
    Ifr0_PowerControl,
    Ifr0_ReadData,
    Ifr0_ProgramData,
    Ifr0_EraseSector,
    Ifr0_EraseChip,
    Ifr0_GetStatus,
    Ifr0_GetInfo
};
