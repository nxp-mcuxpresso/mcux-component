/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdbool.h>
#include "fsl_nand_flash.h"
#include "fsl_xspi_nand_flash.h"
#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* LUT sequence slots. Slot 0 is the runtime scratch. */
#define NAND_CMD_LUT_SEQ_IDX_SCRATCH         0U
#define NAND_CMD_LUT_SEQ_IDX_READCACHE       1U
#define NAND_CMD_LUT_SEQ_IDX_READPAGE        2U
#define NAND_CMD_LUT_SEQ_IDX_READSTATUS      3U
#define NAND_CMD_LUT_SEQ_IDX_WRITEENABLE     4U
#define NAND_CMD_LUT_SEQ_IDX_ERASEBLOCK      5U
#define NAND_CMD_LUT_SEQ_IDX_PROGLOAD        6U
#define NAND_CMD_LUT_SEQ_IDX_PROGLOAD_RANDOM 7U
#define NAND_CMD_LUT_SEQ_IDX_PROGEXECUTE     8U

/* Total LUT words to upload = full lookupTable element count. */
#define NAND_XSPI_LUT_ENTRY_COUNT(lut) ((uint8_t)(sizeof(lut) / sizeof((lut)[0][0])))

/* Standard SPI NAND command opcodes. */
#define NAND_CMD_READ_DATA        0x6BU /* Fast Read Quad Output (Read Cache) */
#define NAND_CMD_GET_FEATURE      0x0FU /* Get Feature */
#define NAND_CMD_WRITE_ENABLE     0x06U /* Write Enable */
#define NAND_CMD_SET_FEATURE      0x1FU /* Set Feature */
#define NAND_CMD_PROG_LOAD        0x32U /* Quad Load Program Data (reset buffer) */
#define NAND_CMD_PROG_LOAD_RANDOM 0x34U /* Quad Random Load Program Data (no reset) */
#define NAND_CMD_PROG_EXECUTE     0x10U /* Program Execute */
#define NAND_CMD_PAGE_DATA_READ   0x13U /* Page Data Read */
#define NAND_CMD_BLOCK_ERASE      0xD8U /* Block Erase */
#define NAND_CMD_READ_JEDEC_ID    0x9FU /* Read JEDEC ID */
#define NAND_CMD_DEVICE_RESET     0xFFU /* Device Reset */

/* XSPI RBDR depth (64 x 4 bytes) - max bytes per IP read. */
#define NAND_XSPI_RBDR_BYTES 256U
/* Max bytes per Program Load chunk (XSPI TX buffer depth). */
#define NAND_XSPI_PROG_CHUNK_BYTES 1024U

/* Max simultaneous NAND instances. */
#ifndef NAND_XSPI_MAX_INSTANCES
#define NAND_XSPI_MAX_INSTANCES 2U
#endif

/* Busy-poll timeout. */
#ifndef NAND_XSPI_BUSY_TIMEOUT_US
#define NAND_XSPI_BUSY_TIMEOUT_US 500000U
#endif

/* ONFI parameter page. */
#define NAND_ONFI_PARAM_PAGE_ADDR  0x01U       /* Row address. */
#define NAND_ONFI_PARAM_PAGE_BYTES 256U        /* ONFI parameter-page read size. */
#define NAND_ONFI_SIGNATURE        0x49464E4FU /* "ONFI" little-endian. */
#define NAND_ONFI_OFF_DATA_BYTES   80U         /* Data bytes per page. */
#define NAND_ONFI_OFF_SPARE_BYTES  84U         /* Spare bytes per page. */
#define NAND_ONFI_OFF_PAGES_BLOCK  92U         /* Pages per block. */
#define NAND_ONFI_OFF_BLOCKS_LUN   96U         /* Blocks per LUN. */
#define NAND_ONFI_OFF_LUNS         100U        /* LUN count. */

/* Standard SPI NAND status/feature register addresses. */
#define NAND_STATUS_REG1 0xA0U /* SR1: block protection. */
#define NAND_STATUS_REG2 0xB0U /* SR2: configuration. */
#define NAND_STATUS_REG3 0xC0U /* SR3: status. */

/* SR2 (configuration) bit masks. */
#define NAND_SR2_BUF_MASK   0x08U /* bit3: buffer read mode. */
#define NAND_SR2_ECC_E_MASK 0x10U /* bit4: on-die ECC enable. */
#define NAND_SR2_OTP_E_MASK 0x40U /* bit6: OTP / parameter-page access enable. */

/* SR3 (status) bit masks. */
#define NAND_SR3_BUSY_MASK  0x01U /* bit0: busy. */
#define NAND_SR3_EFAIL_MASK 0x04U /* bit2: erase failure. */
#define NAND_SR3_PFAIL_MASK 0x08U /* bit3: program failure. */
#define NAND_SR3_ECC_MASK   0x30U /* bit[5:4]: ECC status field. */
#define NAND_SR3_ECC_UNCORR 0x20U /* ECC field 10b: uncorrectable. */

/* Page/block row index -> byte offset. */
#define NAND_XSPI_ROW_ADDR(h, row) ((uint32_t)(row) << (h)->colShift)

/* Per-device info not carried by ONFI. */
typedef struct _xspi_nand_device_info
{
    uint8_t manufacturerId;                                               /* JEDEC MFR ID. */
    uint16_t deviceId;                                                    /* JEDEC device ID. */
    uint8_t cfgSetBits;                                                   /* Config register. */
    void (*buildLut)(XSPI_Type *base, xspi_mem_nand_config_t *memConfig); /* per-vendor LUT builder. */
    uint16_t resetDelayUs;                                                /* Device recovery timing after RESET. */
} xspi_nand_device_info_t;

/* Per-instance state; nand_handle_t.deviceSpecific points at one pool slot. */
typedef struct _xspi_mem_nand_handle
{
    XSPI_Type *base;                 /* Owning XSPI peripheral; also the pool key + free sentinel (NULL == free) */
    xspi_target_group_t targetGroup; /* Target group */
    uint32_t ambaBase;               /* XSPI AHB base */
    uint8_t colShift;                /* Column-address width; left-shift applied to page/row index */
    uint32_t chunkBuf[NAND_XSPI_PROG_CHUNK_BYTES / 4U]; /* Per-instance word-aligned bounce buffer */
} xspi_mem_nand_handle_t;

/* NAND layout parsed from the ONFI parameter page. */
typedef struct _xspi_nand_onfi_params
{
    uint32_t dataBytesPerPage;
    uint32_t spareBytesPerPage;
    uint32_t pagesPerBlock;
    uint32_t blocksPerLun;
    uint32_t luns;
} xspi_nand_onfi_params_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Per-instance state pool. */
static xspi_mem_nand_handle_t s_xspiMemHandlePool[NAND_XSPI_MAX_INSTANCES];

/* Forward decl for the device table below. */
static void xspi_nand_build_lut_std(XSPI_Type *base, xspi_mem_nand_config_t *memConfig);

/* Supported-device table: a part must be listed here to be driven. Layout comes from ONFI. */
static const xspi_nand_device_info_t s_xspiNandDevs[] = {
    /* mfr, devId,   cfgSetBits,                              buildLut,                 tRST max */
    {0xEFU, 0x21BEU, NAND_SR2_BUF_MASK | NAND_SR2_ECC_E_MASK, &xspi_nand_build_lut_std, 500U}, /* Winbond W25N01KW. */
};

/*******************************************************************************
 * Private helpers
 ******************************************************************************/
static xspi_mem_nand_handle_t *xspi_nand_pool_claim(XSPI_Type *base, xspi_target_group_t targetGroup)
{
    xspi_mem_nand_handle_t *slot = NULL;
    uint32_t i;

    for (i = 0U; i < NAND_XSPI_MAX_INSTANCES; i++)
    {
        if ((s_xspiMemHandlePool[i].base == base) && (s_xspiMemHandlePool[i].targetGroup == targetGroup))
        {
            return &s_xspiMemHandlePool[i];
        }
        if ((slot == NULL) && (s_xspiMemHandlePool[i].base == NULL))
        {
            slot = &s_xspiMemHandlePool[i];
        }
    }

    return slot;
}

static status_t xspi_nand_run_scratch(XSPI_Type *base,
                                      xspi_mem_nand_handle_t *handle,
                                      const uint32_t *seq,
                                      uint8_t seqWords,
                                      xspi_command_type_t cmdType,
                                      void *data,
                                      uint32_t dataSize)
{
    xspi_transfer_t xfer = {0};

    XSPI_UpdateLUT(base, NAND_CMD_LUT_SEQ_IDX_SCRATCH * NAND_XSPI_LUT_WORDS_PER_SEQ, seq, seqWords);

    xfer.deviceAddress   = handle->ambaBase;
    xfer.targetGroup     = handle->targetGroup;
    xfer.cmdType         = cmdType;
    xfer.seqIndex        = NAND_CMD_LUT_SEQ_IDX_SCRATCH;
    xfer.data            = data;
    xfer.dataSize        = dataSize;
    xfer.lockArbitration = false;

    return XSPI_TransferBlocking(base, &xfer);
}

static status_t xspi_nand_reset(XSPI_Type *base, xspi_mem_nand_handle_t *handle, uint32_t resetDelayUs)
{
    uint32_t seq =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_DEVICE_RESET, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U);
    status_t status;

    status = xspi_nand_run_scratch(base, handle, &seq, 1U, kXSPI_Command, NULL, 0U);
    if (status != kStatus_Success)
    {
        return status;
    }
    SDK_DelayAtLeastUs(resetDelayUs, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    return status;
}

static status_t xspi_nand_write_enable(XSPI_Type *base, xspi_mem_nand_handle_t *handle)
{
    xspi_transfer_t xfer = {0};

    xfer.deviceAddress   = handle->ambaBase;
    xfer.targetGroup     = handle->targetGroup;
    xfer.cmdType         = kXSPI_Command;
    xfer.seqIndex        = NAND_CMD_LUT_SEQ_IDX_WRITEENABLE;
    xfer.lockArbitration = false;

    return XSPI_TransferBlocking(base, &xfer);
}

static status_t xspi_nand_read_sr3(XSPI_Type *base, xspi_mem_nand_handle_t *handle, uint8_t *reg)
{
    uint32_t val         = 0U;
    xspi_transfer_t xfer = {0};
    status_t status;

    xfer.deviceAddress = handle->ambaBase;
    xfer.targetGroup   = handle->targetGroup;
    xfer.cmdType       = kXSPI_Read;
    xfer.seqIndex      = NAND_CMD_LUT_SEQ_IDX_READSTATUS;
    xfer.data          = &val;
    xfer.dataSize      = 1U;
    status             = XSPI_TransferBlocking(base, &xfer);
    if (status == kStatus_Success)
    {
        *reg = (uint8_t)(val & 0xFFU);
    }

    return status;
}

static status_t xspi_nand_wait_bus_busy(XSPI_Type *base, xspi_mem_nand_handle_t *handle)
{
    bool isBusy       = true;
    uint32_t waitedUs = 0U;
    uint8_t reg       = 0U;
    status_t status;

    do
    {
        status = xspi_nand_read_sr3(base, handle, &reg);
        if (status != kStatus_Success)
        {
            return status;
        }

        isBusy = (reg & NAND_SR3_BUSY_MASK) != 0U;
        if (isBusy)
        {
            if (waitedUs >= NAND_XSPI_BUSY_TIMEOUT_US)
            {
                return kStatus_Timeout;
            }
            SDK_DelayAtLeastUs(10U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
            waitedUs += 10U;
        }
    } while (isBusy);

    return status;
}

static status_t xspi_nand_set_feature(XSPI_Type *base,
                                      xspi_mem_nand_handle_t *handle,
                                      uint8_t featureAddr,
                                      uint8_t value)
{
    uint32_t seq[2] = {
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_SET_FEATURE, kXSPI_Command_SDR, kXSPI_1PAD, featureAddr),
        XSPI_LUT_SEQ(kXSPI_Command_WRITE_SDR, kXSPI_1PAD, 0x01U, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U)};
    uint32_t val = value;
    status_t status;

    status = xspi_nand_write_enable(base, handle);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_run_scratch(base, handle, seq, 2U, kXSPI_Write, &val, 1U);
    if (status != kStatus_Success)
    {
        return status;
    }

    return xspi_nand_wait_bus_busy(base, handle);
}

static status_t xspi_nand_unlock_blocks(XSPI_Type *base, xspi_mem_nand_handle_t *handle)
{
    return xspi_nand_set_feature(base, handle, NAND_STATUS_REG1, 0x00U);
}

/* Common SPI-NAND LUT. */
static void xspi_nand_build_lut_std(XSPI_Type *base, xspi_mem_nand_config_t *memConfig)
{
    (void)memset(memConfig->lookupTable, 0, sizeof(memConfig->lookupTable));

    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_READCACHE][0] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_READ_DATA, kXSPI_Command_CADDR_SDR, kXSPI_1PAD, 0x10U);
    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_READCACHE][1] =
        XSPI_LUT_SEQ(kXSPI_Command_DUMMY_SDR, kXSPI_1PAD, 0x08U, kXSPI_Command_READ_SDR, kXSPI_4PAD, 0x80U);

    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_READSTATUS][0] = XSPI_LUT_SEQ(
        kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_GET_FEATURE, kXSPI_Command_SDR, kXSPI_1PAD, NAND_STATUS_REG3);
    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_READSTATUS][1] =
        XSPI_LUT_SEQ(kXSPI_Command_READ_SDR, kXSPI_1PAD, 0x01U, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U);

    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_WRITEENABLE][0] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_WRITE_ENABLE, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U);

    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_ERASEBLOCK][0] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_BLOCK_ERASE, kXSPI_Command_DUMMY_SDR, kXSPI_1PAD, 0x08U);
    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_ERASEBLOCK][1] =
        XSPI_LUT_SEQ(kXSPI_Command_RADDR_SDR, kXSPI_1PAD, 0x10U, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U);

    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_PROGLOAD][0] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_PROG_LOAD, kXSPI_Command_CADDR_SDR, kXSPI_1PAD, 0x10U);
    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_PROGLOAD][1] =
        XSPI_LUT_SEQ(kXSPI_Command_WRITE_SDR, kXSPI_4PAD, 0x40U, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U);

    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_PROGLOAD_RANDOM][0] = XSPI_LUT_SEQ(
        kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_PROG_LOAD_RANDOM, kXSPI_Command_CADDR_SDR, kXSPI_1PAD, 0x10U);
    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_PROGLOAD_RANDOM][1] =
        XSPI_LUT_SEQ(kXSPI_Command_WRITE_SDR, kXSPI_4PAD, 0x40U, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U);

    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_READPAGE][0] = XSPI_LUT_SEQ(
        kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_PAGE_DATA_READ, kXSPI_Command_DUMMY_SDR, kXSPI_1PAD, 0x08U);
    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_READPAGE][1] =
        XSPI_LUT_SEQ(kXSPI_Command_RADDR_SDR, kXSPI_1PAD, 0x10U, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U);

    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_PROGEXECUTE][0] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_PROG_EXECUTE, kXSPI_Command_DUMMY_SDR, kXSPI_1PAD, 0x08U);
    memConfig->lookupTable[NAND_CMD_LUT_SEQ_IDX_PROGEXECUTE][1] =
        XSPI_LUT_SEQ(kXSPI_Command_RADDR_SDR, kXSPI_1PAD, 0x10U, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U);

    XSPI_UpdateLUT(base, 0U, &memConfig->lookupTable[0][0], NAND_XSPI_LUT_ENTRY_COUNT(memConfig->lookupTable));
}

static const xspi_nand_device_info_t *xspi_nand_find_device(uint8_t mfr, uint16_t deviceId)
{
    uint32_t i;

    for (i = 0U; i < ARRAY_SIZE(s_xspiNandDevs); i++)
    {
        if ((s_xspiNandDevs[i].manufacturerId == mfr) && (s_xspiNandDevs[i].deviceId == deviceId))
        {
            return &s_xspiNandDevs[i];
        }
    }

    return NULL;
}

static inline uint32_t xspi_nand_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool xspi_nand_parse_onfi_params(const uint8_t *buf, xspi_nand_onfi_params_t *p)
{
    if (xspi_nand_le32(&buf[0]) != NAND_ONFI_SIGNATURE)
    {
        return false;
    }

    p->dataBytesPerPage = xspi_nand_le32(&buf[NAND_ONFI_OFF_DATA_BYTES]);
    p->spareBytesPerPage =
        (uint32_t)buf[NAND_ONFI_OFF_SPARE_BYTES] | ((uint32_t)buf[NAND_ONFI_OFF_SPARE_BYTES + 1U] << 8);
    p->pagesPerBlock = xspi_nand_le32(&buf[NAND_ONFI_OFF_PAGES_BLOCK]);
    p->blocksPerLun  = xspi_nand_le32(&buf[NAND_ONFI_OFF_BLOCKS_LUN]);
    p->luns          = buf[NAND_ONFI_OFF_LUNS];

    return (p->dataBytesPerPage != 0U) && (p->spareBytesPerPage != 0U) && (p->pagesPerBlock != 0U) &&
           (p->blocksPerLun != 0U) && (p->luns != 0U);
}

/* Read the ONFI parameter page. */
static status_t xspi_nand_read_onfi_params(XSPI_Type *base, xspi_mem_nand_handle_t *handle, xspi_nand_onfi_params_t *p)
{
    SDK_ALIGN(uint8_t buf[NAND_ONFI_PARAM_PAGE_BYTES], 4U);
    xspi_transfer_t xfer = {0};
    status_t status;

    /* Load the parameter page into the device data buffer. */
    xfer.deviceAddress   = handle->ambaBase + NAND_XSPI_ROW_ADDR(handle, NAND_ONFI_PARAM_PAGE_ADDR);
    xfer.targetGroup     = handle->targetGroup;
    xfer.cmdType         = kXSPI_Command;
    xfer.seqIndex        = NAND_CMD_LUT_SEQ_IDX_READPAGE;
    xfer.lockArbitration = false;
    status               = XSPI_TransferBlocking(base, &xfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_wait_bus_busy(base, handle);
    if (status != kStatus_Success)
    {
        return status;
    }

    xfer.cmdType       = kXSPI_Read;
    xfer.seqIndex      = NAND_CMD_LUT_SEQ_IDX_READCACHE;
    xfer.deviceAddress = handle->ambaBase;
    xfer.data          = (uint32_t *)(uintptr_t)buf;
    xfer.dataSize      = NAND_ONFI_PARAM_PAGE_BYTES;
    status             = XSPI_TransferBlocking(base, &xfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    return xspi_nand_parse_onfi_params(buf, p) ? kStatus_Success : kStatus_Fail;
}

static inline uint32_t xspi_nand_total_pages(const nand_handle_t *handle)
{
    return handle->pagesInBlock * handle->blocksInPlane * handle->planesInDevice;
}

static inline uint32_t xspi_nand_page_bytes(const nand_handle_t *handle)
{
    return handle->bytesInPageDataArea + handle->bytesInPageSpareArea;
}

/* Read the JEDEC ID and match it against the supported-device table. */
static status_t xspi_nand_identify(nand_handle_t *handle, const xspi_nand_device_info_t **device)
{
    uint8_t mfr     = 0U;
    uint16_t devId  = 0U;
    status_t status;

    status = Nand_Flash_ReadJedecId(handle, &mfr, &devId);
    if (status != kStatus_Success)
    {
        return status;
    }

    *device = xspi_nand_find_device(mfr, devId);

    return (*device != NULL) ? kStatus_Success : kStatus_Fail;
}

static status_t xspi_nand_read_layout(XSPI_Type *base,
                                      xspi_mem_nand_handle_t *memHandle,
                                      nand_handle_t *handle,
                                      const xspi_nand_device_info_t *device)
{
    xspi_nand_onfi_params_t onfi = {0};
    uint8_t reg = 0U;
    status_t status;
    status_t restore;

    status = Nand_Flash_GetFeature(handle, NAND_STATUS_REG2, &reg);
    if (status != kStatus_Success)
    {
        return status;
    }
    reg    = (uint8_t)(reg | device->cfgSetBits);
    status = xspi_nand_set_feature(base, memHandle, NAND_STATUS_REG2, reg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Read the parameter page in OTP access mode, then always restore the BUF-enabled base. */
    status = xspi_nand_set_feature(base, memHandle, NAND_STATUS_REG2, (uint8_t)(reg | NAND_SR2_OTP_E_MASK));
    if (status != kStatus_Success)
    {
        return status;
    }
    status  = xspi_nand_read_onfi_params(base, memHandle, &onfi);
    restore = xspi_nand_set_feature(base, memHandle, NAND_STATUS_REG2, reg);
    if (status != kStatus_Success)
    {
        return status;
    }
    if (restore != kStatus_Success)
    {
        return restore;
    }

    if ((onfi.dataBytesPerPage + onfi.spareBytesPerPage) > (1UL << memHandle->colShift))
    {
        return kStatus_OutOfRange;
    }

    handle->bytesInPageDataArea  = onfi.dataBytesPerPage;
    handle->bytesInPageSpareArea = onfi.spareBytesPerPage;
    handle->pagesInBlock         = onfi.pagesPerBlock;
    handle->blocksInPlane        = onfi.blocksPerLun;
    handle->planesInDevice       = onfi.luns;

    return kStatus_Success;
}

/* Device bring-up once the XSPI peripheral is up and the slot fields are set: configure the
 * interface, identify the part, reset it, build its LUT, unlock all blocks, and read the layout. */
static status_t xspi_nand_bring_up(XSPI_Type *base,
                                   xspi_mem_nand_config_t *memConfig,
                                   xspi_mem_nand_handle_t *memHandle,
                                   nand_handle_t *handle)
{
    const xspi_nand_device_info_t *deviceInfo;
    status_t status;

    status = XSPI_SetDeviceConfig(base, &memConfig->deviceConfig);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_identify(handle, &deviceInfo);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_reset(base, memHandle, deviceInfo->resetDelayUs);
    if (status != kStatus_Success)
    {
        return status;
    }

    deviceInfo->buildLut(base, memConfig);

    status = xspi_nand_unlock_blocks(base, memHandle);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_read_layout(base, memHandle, handle, deviceInfo);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* The configured SFAD window must cover the whole device; otherwise high blocks fall outside
     * SFAR and their row commands are silently dropped. */
    if (((uint64_t)xspi_nand_total_pages(handle) << memHandle->colShift) >
        ((uint64_t)memConfig->deviceConfig.deviceSize[0] * 1024U))
    {
        return kStatus_InvalidArgument;
    }

    return kStatus_Success;
}

/*******************************************************************************
 * Public API
 ******************************************************************************/
status_t Nand_Flash_Init(nand_config_t *config, nand_handle_t *handle)
{
    assert(config != NULL);
    assert(handle != NULL);
    assert(config->memControlConfig != NULL);
    assert(config->driverBaseAddr != NULL);

    xspi_mem_nand_config_t *memConfig = (xspi_mem_nand_config_t *)config->memControlConfig;
    XSPI_Type *base                   = (XSPI_Type *)config->driverBaseAddr;
    xspi_mem_nand_handle_t *memHandle;
    xspi_config_t xspiConfig;
    status_t status;

    if ((memConfig->deviceConfig.columnAddrWidth == 0U) || (memConfig->deviceConfig.columnAddrWidth >= 32U))
    {
        return kStatus_InvalidArgument;
    }

    xspi_ip_access_config_t ipAccess = {
        .ptrSfpMdadConfig               = NULL,
        .ptrSfpFradConfig               = NULL,
        .sfpArbitrationLockTimeoutValue = 0xFFFFFFFFU,
        .ipAccessTimeoutValue           = 0xFFFFFFFFU,
    };

    (void)memset(&xspiConfig, 0, sizeof(xspiConfig));
    XSPI_GetDefaultConfig(&xspiConfig);
    xspiConfig.ptrIpAccessConfig = &ipAccess;
    XSPI_Init(base, &xspiConfig);

    (void)memset(handle, 0, sizeof(*handle));

    memHandle = xspi_nand_pool_claim(base, memConfig->targetGroup);
    if (memHandle == NULL)
    {
        return kStatus_Fail;
    }
    memHandle->targetGroup = memConfig->targetGroup;
    memHandle->ambaBase    = memConfig->ambaBase;
    memHandle->colShift    = (uint8_t)memConfig->deviceConfig.columnAddrWidth;
    handle->deviceSpecific = memHandle;
    handle->driverBaseAddr = config->driverBaseAddr;

    status = xspi_nand_bring_up(base, memConfig, memHandle, handle);
    if (status != kStatus_Success)
    {
        memHandle->base        = NULL;
        handle->deviceSpecific = NULL;
        return status;
    }

    memHandle->base = base; /* Commit the pool slot */

    return kStatus_Success;
}

status_t Nand_Flash_Read_Page(nand_handle_t *handle, uint32_t pageIndex, uint8_t *buffer, uint32_t length)
{
    assert(handle != NULL);
    assert(handle->deviceSpecific != NULL);
    assert(handle->driverBaseAddr != NULL);
    assert(buffer != NULL);

    if ((pageIndex >= xspi_nand_total_pages(handle)) || (length > xspi_nand_page_bytes(handle)))
    {
        return kStatus_OutOfRange;
    }

    if (length == 0U)
    {
        return kStatus_Success;
    }

    xspi_mem_nand_handle_t *memHandle = (xspi_mem_nand_handle_t *)handle->deviceSpecific;
    XSPI_Type *base                   = (XSPI_Type *)handle->driverBaseAddr;
    uint8_t *chunkBuf                 = (uint8_t *)memHandle->chunkBuf;
    xspi_transfer_t xfer              = {0};
    status_t status;
    uint8_t reg;

    xfer.deviceAddress   = memHandle->ambaBase + NAND_XSPI_ROW_ADDR(memHandle, pageIndex);
    xfer.targetGroup     = memHandle->targetGroup;
    xfer.cmdType         = kXSPI_Command;
    xfer.seqIndex        = NAND_CMD_LUT_SEQ_IDX_READPAGE;
    xfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &xfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_wait_bus_busy(base, memHandle);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Stream in NAND_XSPI_RBDR_BYTES chunks(per-IP-read limit), unaligned caller
     * buffers go through the bounce buffer. */
    uint8_t *dst   = buffer;
    bool unaligned = (((uintptr_t)buffer & 3U) != 0U);
    uint32_t col   = 0U;
    xfer.cmdType   = kXSPI_Read;
    xfer.seqIndex  = NAND_CMD_LUT_SEQ_IDX_READCACHE;
    while (col < length)
    {
        uint32_t chunk = length - col;
        if (chunk > NAND_XSPI_RBDR_BYTES)
        {
            chunk = NAND_XSPI_RBDR_BYTES;
        }
        xfer.deviceAddress = memHandle->ambaBase + col;
        xfer.data          = unaligned ? (uint32_t *)(uintptr_t)chunkBuf : (uint32_t *)(uintptr_t)dst;
        xfer.dataSize      = chunk;
        status             = XSPI_TransferBlocking(base, &xfer);
        if (status != kStatus_Success)
        {
            return status;
        }
        if (unaligned)
        {
            (void)memcpy(dst, chunkBuf, chunk);
        }
        dst += chunk;
        col += chunk;
    }

    status = xspi_nand_read_sr3(base, memHandle, &reg);
    if (status != kStatus_Success)
    {
        return status;
    }

    return ((reg & NAND_SR3_ECC_MASK) != NAND_SR3_ECC_UNCORR) ? kStatus_Success : kStatus_Fail;
}

status_t Nand_Flash_Read_Page_Partial(
    nand_handle_t *handle, uint32_t pageIndex, uint32_t offset_bytes, uint8_t *buffer, uint32_t length)
{
    assert(handle != NULL);
    assert(handle->deviceSpecific != NULL);
    assert(handle->driverBaseAddr != NULL);
    assert(buffer != NULL);

    if ((pageIndex >= xspi_nand_total_pages(handle)) || (offset_bytes > (UINT32_MAX - length)) ||
        ((offset_bytes + length) > xspi_nand_page_bytes(handle)))
    {
        return kStatus_OutOfRange;
    }
    if (length == 0U)
    {
        return kStatus_Success;
    }

    xspi_mem_nand_handle_t *memHandle = (xspi_mem_nand_handle_t *)handle->deviceSpecific;
    XSPI_Type *base                   = (XSPI_Type *)handle->driverBaseAddr;
    uint8_t *chunkBuf                 = (uint8_t *)memHandle->chunkBuf;
    xspi_transfer_t xfer              = {0};
    status_t status;
    uint8_t reg;

    xfer.deviceAddress   = memHandle->ambaBase + NAND_XSPI_ROW_ADDR(memHandle, pageIndex);
    xfer.targetGroup     = memHandle->targetGroup;
    xfer.cmdType         = kXSPI_Command;
    xfer.seqIndex        = NAND_CMD_LUT_SEQ_IDX_READPAGE;
    xfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &xfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_wait_bus_busy(base, memHandle);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Each IP read is capped at the XSPI RX buffer NAND_XSPI_RBDR_BYTES. */
    uint32_t rangeEnd = offset_bytes + length;
    uint32_t col      = offset_bytes & ~(NAND_XSPI_RBDR_BYTES - 1U);

    xfer.cmdType  = kXSPI_Read;
    xfer.seqIndex = NAND_CMD_LUT_SEQ_IDX_READCACHE;
    while (col < rangeEnd)
    {
        uint32_t copyStart = (col > offset_bytes) ? col : offset_bytes;
        uint32_t copyEnd   = ((col + NAND_XSPI_RBDR_BYTES) < rangeEnd) ? (col + NAND_XSPI_RBDR_BYTES) : rangeEnd;

        xfer.deviceAddress = memHandle->ambaBase + col;
        xfer.data          = (uint32_t *)(uintptr_t)chunkBuf;
        xfer.dataSize      = NAND_XSPI_RBDR_BYTES;
        status             = XSPI_TransferBlocking(base, &xfer);
        if (status != kStatus_Success)
        {
            return status;
        }

        (void)memcpy(buffer + (copyStart - offset_bytes), &chunkBuf[copyStart - col], copyEnd - copyStart);
        col += NAND_XSPI_RBDR_BYTES;
    }

    status = xspi_nand_read_sr3(base, memHandle, &reg);
    if (status != kStatus_Success)
    {
        return status;
    }

    return ((reg & NAND_SR3_ECC_MASK) != NAND_SR3_ECC_UNCORR) ? kStatus_Success : kStatus_Fail;
}

status_t Nand_Flash_Page_Program(nand_handle_t *handle, uint32_t pageIndex, const uint8_t *src, uint32_t length)
{
    assert(handle != NULL);
    assert(handle->deviceSpecific != NULL);
    assert(handle->driverBaseAddr != NULL);
    assert(src != NULL);

    if ((pageIndex >= xspi_nand_total_pages(handle)) || (length > xspi_nand_page_bytes(handle)))
    {
        return kStatus_OutOfRange;
    }

    xspi_mem_nand_handle_t *memHandle = (xspi_mem_nand_handle_t *)handle->deviceSpecific;
    XSPI_Type *base                   = (XSPI_Type *)handle->driverBaseAddr;
    uint8_t *chunkBuf                 = (uint8_t *)memHandle->chunkBuf;
    xspi_transfer_t xfer              = {0};
    status_t status;
    uint8_t reg;

    if (length == 0U)
    {
        return kStatus_Success;
    }

    status = xspi_nand_write_enable(base, memHandle);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Program Load in column-aligned chunks: the first chunk resets the page buffer, the rest
     * append without reset. */
    uint32_t offset      = 0U;
    bool unaligned       = (((uintptr_t)src & 3U) != 0U);
    xfer.targetGroup     = memHandle->targetGroup;
    xfer.cmdType         = kXSPI_Write;
    xfer.lockArbitration = false;

    while (offset < length)
    {
        uint32_t remaining = length - offset;
        uint32_t chunk     = (remaining < NAND_XSPI_PROG_CHUNK_BYTES) ? remaining : NAND_XSPI_PROG_CHUNK_BYTES;

        if (unaligned)
        {
            (void)memcpy(chunkBuf, src + offset, chunk);
        }
        xfer.deviceAddress = memHandle->ambaBase + offset;
        xfer.seqIndex      = (offset == 0U) ? NAND_CMD_LUT_SEQ_IDX_PROGLOAD : NAND_CMD_LUT_SEQ_IDX_PROGLOAD_RANDOM;
        xfer.data          = unaligned ? (uint32_t *)(uintptr_t)chunkBuf : (uint32_t *)(uintptr_t)(src + offset);
        xfer.dataSize      = chunk;

        status = XSPI_TransferBlocking(base, &xfer);
        if (status != kStatus_Success)
        {
            return status;
        }
        offset += xfer.dataSize;
    }

    xfer.deviceAddress = memHandle->ambaBase + NAND_XSPI_ROW_ADDR(memHandle, pageIndex);
    xfer.cmdType       = kXSPI_Command;
    xfer.seqIndex      = NAND_CMD_LUT_SEQ_IDX_PROGEXECUTE;
    xfer.data          = NULL;
    xfer.dataSize      = 0U;
    status             = XSPI_TransferBlocking(base, &xfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_wait_bus_busy(base, memHandle);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_read_sr3(base, memHandle, &reg);
    if (status != kStatus_Success)
    {
        return status;
    }

    return ((reg & NAND_SR3_PFAIL_MASK) != 0U) ? kStatus_Fail : kStatus_Success;
}

status_t Nand_Flash_Erase_Block(nand_handle_t *handle, uint32_t blockIndex)
{
    assert(handle != NULL);
    assert(handle->deviceSpecific != NULL);
    assert(handle->driverBaseAddr != NULL);

    xspi_mem_nand_handle_t *memHandle = (xspi_mem_nand_handle_t *)handle->deviceSpecific;
    XSPI_Type *base                   = (XSPI_Type *)handle->driverBaseAddr;
    xspi_transfer_t xfer              = {0};
    status_t status;
    uint8_t reg;

    if (blockIndex >= (handle->blocksInPlane * handle->planesInDevice))
    {
        return kStatus_OutOfRange;
    }

    uint32_t rowAddr = blockIndex * handle->pagesInBlock;

    status = xspi_nand_write_enable(base, memHandle);
    if (status != kStatus_Success)
    {
        return status;
    }

    xfer.deviceAddress   = memHandle->ambaBase + NAND_XSPI_ROW_ADDR(memHandle, rowAddr);
    xfer.targetGroup     = memHandle->targetGroup;
    xfer.cmdType         = kXSPI_Command;
    xfer.seqIndex        = NAND_CMD_LUT_SEQ_IDX_ERASEBLOCK;
    xfer.lockArbitration = false;
    status               = XSPI_TransferBlocking(base, &xfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_wait_bus_busy(base, memHandle);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nand_read_sr3(base, memHandle, &reg);
    if (status != kStatus_Success)
    {
        return status;
    }
    return ((reg & NAND_SR3_EFAIL_MASK) != 0U) ? kStatus_Fail : kStatus_Success;
}

status_t Nand_Flash_GetFeature(nand_handle_t *handle, uint8_t featureAddr, uint8_t *value)
{
    assert(handle != NULL);
    assert(handle->deviceSpecific != NULL);
    assert(handle->driverBaseAddr != NULL);
    assert(value != NULL);

    uint32_t seq[2] = {
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_GET_FEATURE, kXSPI_Command_SDR, kXSPI_1PAD, featureAddr),
        XSPI_LUT_SEQ(kXSPI_Command_READ_SDR, kXSPI_1PAD, 0x01U, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U)};
    xspi_mem_nand_handle_t *memHandle = (xspi_mem_nand_handle_t *)handle->deviceSpecific;
    XSPI_Type *base                   = (XSPI_Type *)handle->driverBaseAddr;
    uint32_t val                      = 0U;
    status_t status;

    status = xspi_nand_run_scratch(base, memHandle, seq, 2U, kXSPI_Read, &val, 1U);
    if (status == kStatus_Success)
    {
        *value = (uint8_t)(val & 0xFFU);
    }

    return status;
}

status_t Nand_Flash_SetFeature(nand_handle_t *handle, uint8_t featureAddr, uint8_t value)
{
    assert(handle != NULL);
    assert(handle->deviceSpecific != NULL);
    assert(handle->driverBaseAddr != NULL);

    return xspi_nand_set_feature((XSPI_Type *)handle->driverBaseAddr, (xspi_mem_nand_handle_t *)handle->deviceSpecific,
                                 featureAddr, value);
}

status_t Nand_Flash_ReadJedecId(nand_handle_t *handle, uint8_t *mfr, uint16_t *deviceId)
{
    assert(handle != NULL);
    assert(handle->deviceSpecific != NULL);
    assert(handle->driverBaseAddr != NULL);
    assert(mfr != NULL);
    assert(deviceId != NULL);

    uint32_t seq[2] = {
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, NAND_CMD_READ_JEDEC_ID, kXSPI_Command_SDR, kXSPI_1PAD, 0x00U),
        XSPI_LUT_SEQ(kXSPI_Command_READ_SDR, kXSPI_1PAD, 0x03U, kXSPI_Command_STOP, kXSPI_1PAD, 0x00U)};
    xspi_mem_nand_handle_t *memHandle = (xspi_mem_nand_handle_t *)handle->deviceSpecific;
    XSPI_Type *base                   = (XSPI_Type *)handle->driverBaseAddr;
    uint32_t value                    = 0U;
    status_t status;

    status    = xspi_nand_run_scratch(base, memHandle, seq, 2U, kXSPI_Read, &value, 3U);
    *mfr      = (uint8_t)(value & 0xFFU);
    *deviceId = (uint16_t)((value >> 8U) & 0xFFFFU);

    return status;
}
