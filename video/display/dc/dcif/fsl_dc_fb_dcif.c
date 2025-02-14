/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_dc_fb_dcif.h"
#if defined(SDK_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
const dc_fb_ops_t g_dcFbOpsDcif = {
    .init                  = DC_FB_DCIF_Init,
    .deinit                = DC_FB_DCIF_Deinit,
    .enableLayer           = DC_FB_DCIF_EnableLayer,
    .disableLayer          = DC_FB_DCIF_DisableLayer,
    .setLayerConfig        = DC_FB_DCIF_SetLayerConfig,
    .getLayerDefaultConfig = DC_FB_DCIF_GetLayerDefaultConfig,
    .setFrameBuffer        = DC_FB_DCIF_SetFrameBuffer,
    .getProperty           = DC_FB_DCIF_GetProperty,
    .setCallback           = DC_FB_DCIF_SetCallback,
};

typedef struct
{
    video_pixel_format_t videoFormat;
    dcif_pixel_format_t dcifFormat;
} dc_fb_dcif_pixel_foramt_map_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static status_t DC_FB_DCIF_GetPixelFormat(video_pixel_format_t input, dcif_pixel_format_t *output);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const dc_fb_dcif_pixel_foramt_map_t s_dcifPixelFormatMap[] = {
    {kVIDEO_PixelFormatRGB565, kDCIF_PixelFormatRGB565},
    {kVIDEO_PixelFormatRGB888, kDCIF_PixelFormatRGB888},
    {kVIDEO_PixelFormatXRGB8888, kDCIF_PixelFormatARGB8888},
    {kVIDEO_PixelFormatXBGR8888, kDCIF_PixelFormatABGR8888},
    {kVIDEO_PixelFormatXRGB4444, kDCIF_PixelFormatARGB4444},
    {kVIDEO_PixelFormatXRGB1555, kDCIF_PixelFormatARGB1555}};
;

/*******************************************************************************
 * Code
 ******************************************************************************/
static status_t DC_FB_DCIF_GetPixelFormat(video_pixel_format_t input, dcif_pixel_format_t *output)
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

status_t DC_FB_DCIF_Init(const dc_fb_t *dc)
{
    status_t status = kStatus_Success;
    const dc_fb_dcif_config_t *dcConfig;

    dcif_display_config_t dcifConfig = {0};

    dc_fb_dcif_handle_t *dcHandle = dc->prvData;

    if (0U == dcHandle->initTimes++)
    {
        dcConfig = (const dc_fb_dcif_config_t *)(dc->config);

        DCIF_DisplayGetDefaultConfig(&dcifConfig);

        dcifConfig.panelWidth    = dcConfig->width;
        dcifConfig.panelHeight   = dcConfig->height;
        dcifConfig.hsw           = (uint8_t)dcConfig->hsw;
        dcifConfig.hfp           = (uint8_t)dcConfig->hfp;
        dcifConfig.hbp           = (uint8_t)dcConfig->hbp;
        dcifConfig.vsw           = (uint8_t)dcConfig->vsw;
        dcifConfig.vfp           = (uint8_t)dcConfig->vfp;
        dcifConfig.vbp           = (uint8_t)dcConfig->vbp;
        dcifConfig.polarityFlags = dcConfig->polarityFlags;
        dcifConfig.lineOrder     = dcConfig->lineOrder;

        dcHandle->height  = dcConfig->height;
        dcHandle->width   = dcConfig->width;
        dcHandle->dcif    = dcConfig->dcif;
        dcHandle->domain  = dcConfig->domain;

        DCIF_Init(dcHandle->dcif);

        DCIF_SetDisplayConfig(dcHandle->dcif, &dcifConfig);

        DCIF_EnableInterrupts(dcHandle->dcif, (uint32_t)kDCIF_VerticalBlankingInterrupt);

        DCIF_EnableDisplay(dcHandle->dcif, true);
    }

    return status;
}

status_t DC_FB_DCIF_Deinit(const dc_fb_t *dc)
{
    dc_fb_dcif_handle_t *dcHandle = dc->prvData;

    if (dcHandle->initTimes > 0U)
    {
        if (--dcHandle->initTimes == 0U)
        {
            DCIF_DisableInterrupts(dcHandle->dcif, dcHandle->domain,
                                      (uint32_t)kDCIF_VerticalBlankingInterrupt);
            DCIF_Deinit(dcHandle->dcif);
        }
    }

    return kStatus_Success;
}

status_t DC_FB_DCIF_EnableLayer(const dc_fb_t *dc, uint8_t layer)
{
    assert(layer < DC_FB_DCIF_MAX_LAYER);

    status_t status                  = kStatus_Success;
    dc_fb_dcif_handle_t *dcHandle = dc->prvData;

    /* If the layer is not started. */
    if (!dcHandle->layers[1].enabled)
    {
        DCIF_EnableLayer(dcHandle->dcif, 1, true);
        DCIF_TriggerLayerShadowLoad(dcHandle->dcif, 1);
        dcHandle->layers[1].shadowLoadPending = true;

        dcHandle->layers[1].activeBuffer = dcHandle->layers[1].inactiveBuffer;
        dcHandle->layers[1].enabled      = true;
    }

    return status;
}

status_t DC_FB_DCIF_DisableLayer(const dc_fb_t *dc, uint8_t layer)
{
    assert(layer < DC_FB_DCIF_MAX_LAYER);

    dc_fb_dcif_handle_t *dcHandle = dc->prvData;

    if (dcHandle->layers[1].enabled)
    {
        DCIF_EnableLayer(dcHandle->dcif, 1, false);
        DCIF_TriggerLayerShadowLoad(dcHandle->dcif, 1);
        dcHandle->layers[1].enabled = false;
    }

    return kStatus_Success;
}

status_t DC_FB_DCIF_SetLayerConfig(const dc_fb_t *dc, uint8_t layer, dc_fb_info_t *fbInfo)
{
    assert(layer < DC_FB_DCIF_MAX_LAYER);

    dcif_buffer_config_t bufferConfig = {0};
    dcif_pixel_format_t pixelFormat;
    DCIF_Type *dcif;
    status_t status;

    dc_fb_dcif_handle_t *dcHandle = (dc_fb_dcif_handle_t *)(dc->prvData);

    dcif = dcHandle->dcif;

    status = DC_FB_DCIF_GetPixelFormat(fbInfo->pixelFormat, &pixelFormat);
    if (kStatus_Success != status)
    {
        return status;
    }

    DCIF_SetLayerSize(dcif, 1, fbInfo->width, fbInfo->height);

    bufferConfig.strideBytes = fbInfo->strideBytes;
    bufferConfig.pixelFormat = pixelFormat;
    bufferConfig.globalAlpha = DC_FB_DCIF_GLOBAL_ALPHA;
    DCIF_SetLayerBufferConfig(dcif, 1, &bufferConfig);

    return kStatus_Success;
}

status_t DC_FB_DCIF_GetLayerDefaultConfig(const dc_fb_t *dc, uint8_t layer, dc_fb_info_t *fbInfo)
{
    assert(layer < DC_FB_DCIF_MAX_LAYER);

    dc_fb_dcif_handle_t *dcHandle = (dc_fb_dcif_handle_t *)(dc->prvData);

    fbInfo->startX      = 0;
    fbInfo->startY      = 0;
    fbInfo->width       = dcHandle->width;
    fbInfo->height      = dcHandle->height;
    fbInfo->strideBytes = DC_FB_DCIF_DEFAULT_BYTE_PER_PIXEL * dcHandle->width;
    fbInfo->pixelFormat = DC_FB_DCIF_DEFAULT_PIXEL_FORMAT;

    return kStatus_Success;
}

status_t DC_FB_DCIF_SetFrameBuffer(const dc_fb_t *dc, uint8_t layer, void *frameBuffer)
{
    assert(layer < DC_FB_DCIF_MAX_LAYER);
    dc_fb_dcif_handle_t *dcHandle = dc->prvData;

    DCIF_SetLayerBufferAddr(dcHandle->dcif, 1, (uint32_t)(uint8_t *)frameBuffer);
    dcHandle->layers[1].inactiveBuffer = frameBuffer;

    if (dcHandle->layers[1].enabled)
    {
        DCIF_TriggerLayerShadowLoad(dcHandle->dcif, 1);
        dcHandle->layers[1].shadowLoadPending = true;
        dcHandle->layers[1].framePending      = true;
    }
    else
    {
    }

    return kStatus_Success;
}

void DC_FB_DCIF_SetCallback(const dc_fb_t *dc, uint8_t layer, dc_fb_callback_t callback, void *param)
{
    assert(layer < DC_FB_DCIF_MAX_LAYER);
    dc_fb_dcif_handle_t *dcHandle = dc->prvData;

    dcHandle->layers[1].callback = callback;
    dcHandle->layers[1].cbParam  = param;
}

uint32_t DC_FB_DCIF_GetProperty(const dc_fb_t *dc)
{
    return (uint32_t)kDC_FB_ReserveFrameBuffer;
}

void DC_FB_DCIF_IRQHandler(const dc_fb_t *dc)
{
    uint32_t intStatus;
    dc_fb_dcif_handle_t *dcHandle = dc->prvData;
    dc_fb_dcif_layer_t *layer;
    void *oldActiveBuffer;

    intStatus = DCIF_GetInterruptStatus(dcHandle->dcif);
    DCIF_ClearInterruptStatus(dcHandle->dcif, intStatus);

    if (0U == (intStatus & (uint32_t)kDCIF_VerticalBlankingInterrupt))
    {
        return;
    }

    for (uint8_t i = 0; i < DC_FB_DCIF_MAX_LAYER; i++)
    {
        if (dcHandle->layers[i].shadowLoadPending)
        {
            dcHandle->layers[i].shadowLoadPending = false;
        }

        if (dcHandle->layers[i].framePending)
        {
            layer = &dcHandle->layers[i];

            oldActiveBuffer                  = layer->activeBuffer;
            layer->activeBuffer              = layer->inactiveBuffer;
            dcHandle->layers[i].framePending = false;

            layer->callback(layer->cbParam, oldActiveBuffer);
        }
    }
}
