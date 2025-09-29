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
    dcif_layer_format_t dcifFormat;
} dc_fb_dcif_pixel_foramt_map_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static status_t DC_FB_DCIF_GetPixelFormat(video_pixel_format_t input, dcif_layer_format_t *output);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const dc_fb_dcif_pixel_foramt_map_t s_dcifPixelFormatMap[] = {
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
static status_t DC_FB_DCIF_GetPixelFormat(video_pixel_format_t input, dcif_layer_format_t *output)
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

    dc_fb_dcif_handle_t *dcHandle = dc->prvData;

    if (0U == dcHandle->initTimes++)
    {
        dcConfig = (const dc_fb_dcif_config_t *)(dc->config);

        dcif_output_config_t outputConfig = {
            .interface = kDCIF_OutputDpi,
            .width     = dcConfig->width,
            .height    = dcConfig->height,
        };

        dcif_dpi_config_t dpiConfig = {
            .hsw           = (uint8_t)dcConfig->hsw,
            .hfp           = (uint8_t)dcConfig->hfp,
            .hbp           = (uint8_t)dcConfig->hbp,
            .vsw           = (uint8_t)dcConfig->vsw,
            .vfp           = (uint8_t)dcConfig->vfp,
            .vbp           = (uint8_t)dcConfig->vbp,
            .polarityFlags = dcConfig->polarityFlags,
            .format        = dcConfig->output,
            .displayMode   = kDCIF_DpiNormal,
            .frameFetch    = kDCIF_DpiVswBegin,
            .txFifoFill    = kDCIF_DpiVbpEnd,
            .enableBackground = true,
        };
        dcHandle->height = dcConfig->height;
        dcHandle->width  = dcConfig->width;
        dcHandle->dcif   = dcConfig->dcif;
        dcHandle->domain = dcConfig->domain;

        DCIF_Init(dcHandle->dcif);

        /* Set panel output config. */
        DCIF_SetOutputConfig(dcHandle->dcif, &outputConfig);

        /* Set DPI config. */
        DCIF_DpiSetConfig(dcHandle->dcif, &dpiConfig);

        /* Background black. */
        DCIF_SetBackGroundLayerColor(dcHandle->dcif, 0x0U);

        DCIF_EnableOutput(dcHandle->dcif, true);
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
            DCIF_DisableInterrupts(dcHandle->dcif, dcHandle->domain, (uint32_t)kDCIF_InterruptVsync);
            DCIF_EnableOutput(dcHandle->dcif, false);
            DCIF_Deinit(dcHandle->dcif);//todo
        }
    }

    return kStatus_Success;
}

status_t DC_FB_DCIF_EnableLayer(const dc_fb_t *dc, uint8_t layer)
{
    assert(layer < DC_FB_DCIF_MAX_LAYER);

    status_t status               = kStatus_Success;
    dc_fb_dcif_handle_t *dcHandle = dc->prvData;

    /* If the layer is not started. */
    if (!dcHandle->layers[layer].enabled)
    {
        DCIF_EnableLayer(dcHandle->dcif, layer, true);
        DCIF_TriggerLayerShadowLoad(dcHandle->dcif, layer);
        DCIF_EnableInterrupts(dcHandle->dcif, dcHandle->domain, (uint32_t)kDCIF_InterruptVsync);
        dcHandle->layers[layer].shadowLoadPending = true;

        while (true == dcHandle->layers[layer].shadowLoadPending)
        {
#if defined(SDK_OS_FREE_RTOS)
            vTaskDelay(1);
#endif
        }

        dcHandle->layers[layer].activeBuffer = dcHandle->layers[layer].inactiveBuffer;
        dcHandle->layers[layer].enabled      = true;
    }

    return status;
}

status_t DC_FB_DCIF_DisableLayer(const dc_fb_t *dc, uint8_t layer)
{
    assert(layer < DC_FB_DCIF_MAX_LAYER);

    dc_fb_dcif_handle_t *dcHandle = dc->prvData;

    if (dcHandle->layers[layer].enabled)
    {
        DCIF_EnableLayer(dcHandle->dcif, layer, false);
        DCIF_TriggerLayerShadowLoad(dcHandle->dcif, layer);
        dcHandle->layers[layer].enabled = false;
    }

    return kStatus_Success;
}

status_t DC_FB_DCIF_SetLayerConfig(const dc_fb_t *dc, uint8_t layer, dc_fb_info_t *fbInfo)
{
    assert(layer < DC_FB_DCIF_MAX_LAYER);

    dcif_layer_config_t layerConfig = {0};
    dcif_layer_format_t pixelFormat;
    dc_fb_dcif_handle_t *dcHandle = (dc_fb_dcif_handle_t *)(dc->prvData);
    DCIF_Type *dcif = dcHandle->dcif;
    status_t status;

    status = DC_FB_DCIF_GetPixelFormat(fbInfo->pixelFormat, &pixelFormat);
    if (kStatus_Success != status)
    {
        return status;
    }

    layerConfig.enable           = false;
    layerConfig.format           = pixelFormat;
    layerConfig.topLeftX         = fbInfo->startX;
    layerConfig.topLeftY         = fbInfo->startY;
    layerConfig.width            = fbInfo->width;
    layerConfig.height           = fbInfo->height;
    layerConfig.background       = 0U;
    layerConfig.panic.enable     = false;
    layerConfig.globalAlpha      = 0xFFU;
    layerConfig.alphaBlendMode   = kDCIF_AlphaBlendEmbedded;

    DCIF_SetLayerStride(dcif, 0, fbInfo->strideBytes);

    DCIF_SetLayerConfig(dcif, layer, &layerConfig);

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

    DCIF_SetLayerAddr(dcHandle->dcif, layer, (uint32_t)(uint8_t *)frameBuffer);
    dcHandle->layers[layer].inactiveBuffer = frameBuffer;

    if (dcHandle->layers[layer].enabled)
    {
        DCIF_TriggerLayerShadowLoad(dcHandle->dcif, layer);
        dcHandle->layers[layer].shadowLoadPending = true;
        dcHandle->layers[layer].framePending      = true;
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

    dcHandle->layers[layer].callback = callback;
    dcHandle->layers[layer].cbParam  = param;
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

    intStatus = DCIF_GetInterruptStatus(dcHandle->dcif, dcHandle->domain);
    DCIF_ClearInterruptStatus(dcHandle->dcif, dcHandle->domain, intStatus);

    if (0U == (intStatus & (uint32_t)kDCIF_InterruptVsync))
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
