/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_dbi.h"
#include "fsl_dbi_dcif.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct
{
    video_pixel_format_t videoFormat;
    dcif_layer_format_t dcifFormat;
} dbi_dcif_pixel_foramt_map_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static status_t DBI_DCIF_GetPixelFormat(video_pixel_format_t input,
                                         dcif_layer_format_t *output);

static void DBI_DCIF_WriteMemoryInternal(
    dbi_iface_t *dbiIface, uint32_t command, const void *data, uint32_t stride_byte, bool isInterleave);

/*******************************************************************************
 * Variables
 ******************************************************************************/
const dbi_iface_xfer_ops_t g_dbiLcdifXferOps = {
    .writeCommandData = DBI_DCIF_WriteCommandData,
    .writeMemory      = DBI_DCIF_WriteMemory,
    .writeMemory2D    = DBI_DCIF_WriteMemory2D,
};

const dbi_config_ops_t g_dbiLcdifConfigOps = {
    .setPixelFormat = DBI_DCIF_SetPixelFormat,
};

static const dbi_dcif_pixel_foramt_map_t s_dcifPixelFormatMap[] = {
    {kVIDEO_PixelFormatLUT8,     kDCIF_LayerPixelFormatIndex8BPP},
    {kVIDEO_PixelFormatRGB565,   kDCIF_LayerPixelFormatRGB565},
    {kVIDEO_PixelFormatRGB888,   kDCIF_LayerPixelFormatRGB888},
    {kVIDEO_PixelFormatXRGB1555, kDCIF_LayerPixelFormatARGB1555},
    {kVIDEO_PixelFormatXRGB4444, kDCIF_LayerPixelFormatARGB4444},
    {kVIDEO_PixelFormatUYVY,     kDCIF_LayerPixelFormatUYVY},
    {kVIDEO_PixelFormatVYUY,     kDCIF_LayerPixelFormatVYUY},
    {kVIDEO_PixelFormatYUYV,     kDCIF_LayerPixelFormatYUYV},
    {kVIDEO_PixelFormatYVYU,     kDCIF_LayerPixelFormatYVYU},
    {kVIDEO_PixelFormatXRGB8888, kDCIF_LayerPixelFormatARGB8888},
    {kVIDEO_PixelFormatXBGR8888, kDCIF_LayerPixelFormatABGR8888},
};

/*******************************************************************************
 * Code
 ******************************************************************************/
void DBI_DCIF_IRQHandler(dbi_iface_t *dbiIface)
{
    dbi_dcif_prv_data_t *prvData = (dbi_dcif_prv_data_t *)dbiIface->prvData;
    DCIF_Type *dcif              = prvData->dcif;
    uint32_t intStatus           = DCIF_GetInterruptStatus(dcif, prvData->domain);

    DCIF_ClearInterruptStatus(dcif, prvData->domain, intStatus);

    /* Completion callback. */
    dbiIface->memDoneCallback(kStatus_Success, dbiIface->memDoneCallbackParam);
}

void DBI_DCIF_InitController(dbi_iface_t *dbiIface, dbi_dcif_prv_data_t *prvData)
{
    assert(prvData != NULL);
    assert(prvData->dcif != NULL);

    dcif_output_config_t outputConfig = {
        .interface = kDCIF_OutputDbi,
        .width     = prvData->width,
        .height    = prvData->height,
    };

    /* Set panel output config. */
    DCIF_SetOutputConfig(prvData->dcif, &outputConfig);

    /* Background black. */
    DCIF_SetBackGroundLayerColor(prvData->dcif, 0x0U);

    /* Configure interface. */
    memset(dbiIface, 0, sizeof(dbi_iface_t));

    dbiIface->xferOps   = &g_dbiLcdifXferOps;
    dbiIface->configOps = &g_dbiLcdifConfigOps;
    dbiIface->prvData   = (void *)prvData;
}

status_t DBI_DCIF_WriteCommandData(dbi_iface_t *dbiIface, uint32_t command, const void *data, uint32_t len_byte)
{
    dbi_dcif_prv_data_t *prvData = (dbi_dcif_prv_data_t *)dbiIface->prvData;
    DCIF_Type *dcif              = prvData->dcif;
    uint8_t cmd                  = (uint8_t)command;
    uint8_t *pData               = (uint8_t *)data;

    if ((cmd == (uint8_t)kMIPI_DBI_SetColumnAddress) || (cmd == (uint8_t)kMIPI_DBI_SetPageAddress))
    {
        uint16_t lrValue = ((uint16_t)pData[2] << 8U) | (uint16_t)pData[3];
        uint16_t tpValue = ((uint16_t)pData[0] << 8U) | (uint16_t)pData[1];

        if (lrValue < tpValue)
        {
            return kStatus_Fail;
        }

        if (cmd == kMIPI_DBI_SetColumnAddress)
        {
            prvData->width = lrValue - tpValue + 1U;
        }
        else
        {
            prvData->height = lrValue - tpValue + 1U;
        }
    }

    DCIF_DbiWriteCommand(dcif, cmd);

    while ((dcif->DBI_CTRL & DCIF_DBI_CTRL_DBI_CMD_TRIG_MASK) != 0U)
    {
        /* Wait for the command to be completed. */
    }

    if (len_byte != 0U)
    {
        DCIF_DbiWriteParam(dcif, pData, len_byte);
        while ((dcif->DBI_CTRL & DCIF_DBI_CTRL_DBI_CMD_TRIG_MASK) != 0U)
        {
            /* Wait for the command to be completed. */
        }
    }

    return kStatus_Success;
}

status_t DBI_DCIF_WriteMemory(dbi_iface_t *dbiIface, uint32_t command, const void *data, uint32_t len_byte)
{
    DBI_DCIF_WriteMemoryInternal(dbiIface, command, data, 0U, false);

    return kStatus_Success;
}

status_t DBI_DCIF_WriteMemory2D(
    dbi_iface_t *dbiIface, uint32_t command, const void *data, uint32_t len_byte, uint32_t stride_byte)
{
    DBI_DCIF_WriteMemoryInternal(dbiIface, command, data, stride_byte, true);

    return kStatus_Success;
}

status_t DBI_DCIF_SetPixelFormat(dbi_iface_t *dbiIface, video_pixel_format_t format)
{
    dcif_layer_format_t pixelFormat;
    status_t status              = kStatus_Success;
    dbi_dcif_prv_data_t *prvData = (dbi_dcif_prv_data_t *)dbiIface->prvData;
    DCIF_Type *dcif              = prvData->dcif;

    status = DBI_DCIF_GetPixelFormat(format, &pixelFormat);

    if (kStatus_Success != status)
    {
        return status;
    }

    dcif_layer_config_t layerConfig = {
        .enable         = true,
        .format         = pixelFormat,
        .topLeftX       = 0U,
        .topLeftY       = 0U,
        .width          = prvData->width,
        .height         = prvData->height,
        .background     = 0U,
        .panic.enable   = false,
        .globalAlpha    = 0xFFU,
        .alphaBlendMode = kDCIF_AlphaBlendEmbedded,
    };

    /* Enable DCIF. todo*/
    DCIF_EnableOutput(dcif, true);

    /* Configure layer */
    DCIF_SetLayerConfig(dcif, 0U, &layerConfig);

    /* Set byte per pixel for stride calculation later. */
    prvData->bytePerPixel = VIDEO_GetPixelSizeBits(format) / 8U;

    return status;
}

static status_t DBI_DCIF_GetPixelFormat(video_pixel_format_t input,
                                         dcif_layer_format_t *output)
{
    uint8_t i;

    for (i = 0; i < ARRAY_SIZE(s_dcifPixelFormatMap); i++)
    {
        if (s_dcifPixelFormatMap[i].videoFormat == input)
        {
            *output = s_dcifPixelFormatMap[i].dcifFormat;
            return kStatus_Success;
        }
    }

    return kStatus_InvalidArgument;
}

static void DBI_DCIF_WriteMemoryInternal(
    dbi_iface_t *dbiIface, uint32_t command, const void *data, uint32_t stride_byte, bool isInterleave)
{
    uint32_t stride;
    uint8_t cmd                  = (uint8_t)command;
    dbi_dcif_prv_data_t *prvData = (dbi_dcif_prv_data_t *)dbiIface->prvData;
    DCIF_Type *dcif              = prvData->dcif;

    if (isInterleave)
    {
        stride = stride_byte;
    }
    else
    {
        stride = (uint32_t)prvData->width * (uint32_t)prvData->bytePerPixel;
    }

    /* Configure buffer position, address and area. */
    DCIF_SetLayerPosition(dcif, 0U, 0U, 0U, prvData->width, prvData->height);

    /* Set layer buffer address, stride, and enable the layer. */
    DCIF_SetLayerStride(dcif, 0U, stride);
    DCIF_SetLayerAddr(dcif, 0U, (uint32_t)data);

    /* Disable interrupts. */
    DCIF_DisableInterrupts(dcif, prvData->domain, kDCIF_InterruptDbiCommandDone);

    /* Send command, write memory start or continue. */
    DCIF_DbiWriteCommand(dcif, cmd);

    while ((dcif->DBI_CTRL & DCIF_DBI_CTRL_DBI_CMD_TRIG_MASK) != 0U)
    {
        /* Wait for the command to be completed. */
    }

    /* Start memory transfer. */
    DCIF_DbiWritePixel(dcif);

    /* Enable memory write done interrupt. */
    DCIF_EnableInterrupts(dcif, prvData->domain, kDCIF_InterruptDbiCommandDone);
}
