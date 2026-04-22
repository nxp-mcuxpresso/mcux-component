/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_waveshare_dsi2dpi.h"
#include "fsl_video_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define WAVESHARE_DSI2DPI_I2C_WriteReg(handle, addr, reg, value)                             \
    VIDEO_I2C_WriteReg(addr, kVIDEO_RegAddr8Bit, (reg), kVIDEO_RegWidth8Bit, (value),        \
                       ((const waveshare_dsi2dpi_resource_t *)((handle)->resource))->i2cSendFunc)

#define WAVESHARE_DSI2DPI_CHECK_RET(x)    \
    do                                    \
    {                                     \
        status = (x);                     \
        if (kStatus_Success != status)    \
        {                                 \
            return status;                \
        }                                 \
    } while (false)

static uint8_t WAVESHARE_DSI2DPI_GetI2cAddr(const display_handle_t *handle)
{
    uint8_t i2cAddr = (((const waveshare_dsi2dpi_resource_t *)(handle->resource))->i2cAddr);

    return (i2cAddr == 0U) ? WAVESHARE_DSI2DPI_I2C_ADDR : i2cAddr;
}

static status_t WAVESHARE_DSI2DPI_Enable(display_handle_t *handle, bool enable)
{
    return WAVESHARE_DSI2DPI_I2C_WriteReg(handle, WAVESHARE_DSI2DPI_GetI2cAddr(handle),
                                          WAVESHARE_DSI2DPI_REG_AD, enable ? 0x01U : 0x00U);
}

/*******************************************************************************
 * Variables
 ******************************************************************************/

const display_operations_t waveshare_dsi2dpi_ops = {
    .init   = WAVESHARE_DSI2DPI_Init,
    .deinit = WAVESHARE_DSI2DPI_Deinit,
    .start  = WAVESHARE_DSI2DPI_Start,
    .stop   = WAVESHARE_DSI2DPI_Stop,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t WAVESHARE_DSI2DPI_SetBrightness(display_handle_t *handle, uint8_t brightness)
{
    status_t status;
    uint8_t i2cAddr = WAVESHARE_DSI2DPI_GetI2cAddr(handle);

    WAVESHARE_DSI2DPI_CHECK_RET(
        WAVESHARE_DSI2DPI_I2C_WriteReg(handle, i2cAddr, WAVESHARE_DSI2DPI_REG_AB, (uint8_t)(0xFFU - brightness)));

    WAVESHARE_DSI2DPI_CHECK_RET(
        WAVESHARE_DSI2DPI_I2C_WriteReg(handle, i2cAddr, WAVESHARE_DSI2DPI_REG_AA, 0x01U));

    return kStatus_Success;
}

status_t WAVESHARE_DSI2DPI_Init(display_handle_t *handle, const display_config_t *config)
{
    status_t status;
    uint8_t i2cAddr = WAVESHARE_DSI2DPI_GetI2cAddr(handle);

    if ((config->dsiLanes == 0U) || (config->dsiLanes > 4U))
    {
        return kStatus_InvalidArgument;
    }

    handle->height      = FSL_VIDEO_EXTRACT_HEIGHT(config->resolution);
    handle->width       = FSL_VIDEO_EXTRACT_WIDTH(config->resolution);
    handle->pixelFormat = config->pixelFormat;

    WAVESHARE_DSI2DPI_CHECK_RET(WAVESHARE_DSI2DPI_I2C_WriteReg(handle, i2cAddr, WAVESHARE_DSI2DPI_REG_C0, 0x01U));
    WAVESHARE_DSI2DPI_CHECK_RET(WAVESHARE_DSI2DPI_I2C_WriteReg(handle, i2cAddr, WAVESHARE_DSI2DPI_REG_C2, 0x01U));
    WAVESHARE_DSI2DPI_CHECK_RET(WAVESHARE_DSI2DPI_I2C_WriteReg(handle, i2cAddr, WAVESHARE_DSI2DPI_REG_AC, 0x01U));

    /* Default brightness: full. */
    WAVESHARE_DSI2DPI_CHECK_RET(WAVESHARE_DSI2DPI_SetBrightness(handle, 0xFFU));

    /* Enable output. */
    WAVESHARE_DSI2DPI_CHECK_RET(WAVESHARE_DSI2DPI_Enable(handle, true));

    return kStatus_Success;
}

status_t WAVESHARE_DSI2DPI_Deinit(display_handle_t *handle)
{
    return WAVESHARE_DSI2DPI_Enable(handle, false);
}

status_t WAVESHARE_DSI2DPI_Start(display_handle_t *handle)
{
    return WAVESHARE_DSI2DPI_Enable(handle, true);
}

status_t WAVESHARE_DSI2DPI_Stop(display_handle_t *handle)
{
    return WAVESHARE_DSI2DPI_Enable(handle, false);
}
