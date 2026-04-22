/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_WAVESHARE_DSI2DPI_H_
#define _FSL_WAVESHARE_DSI2DPI_H_

#include "fsl_common.h"
#include "fsl_display.h"
#include "fsl_video_i2c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Waveshare DSI2DPI default I2C 7-bit address.
 *
 * Per Waveshare documentation the default address is 0x45U.
 * Set @ref waveshare_dsi2dpi_resource_t::i2cAddr to override.
 */
#define WAVESHARE_DSI2DPI_I2C_ADDR (0x45U)

/* Register definitions */
#define WAVESHARE_DSI2DPI_REG_C0 (0xC0U)
#define WAVESHARE_DSI2DPI_REG_C2 (0xC2U)
#define WAVESHARE_DSI2DPI_REG_AC (0xACU)
#define WAVESHARE_DSI2DPI_REG_AA (0xAAU)
#define WAVESHARE_DSI2DPI_REG_AB (0xABU)
#define WAVESHARE_DSI2DPI_REG_AD (0xADU)

/*! @brief Waveshare DSI2DPI resource.
 *
 * The I2C instance should be initialized before calling @ref WAVESHARE_DSI2DPI_Init.
 */
typedef struct _waveshare_dsi2dpi_resource
{
    video_i2c_send_func_t i2cSendFunc;       /*!< I2C send function. */
    video_i2c_receive_func_t i2cReceiveFunc; /*!< I2C receive function (optional). */
    /*! I2C 7-bit address.
     *
     * Set to 0 to use @ref WAVESHARE_DSI2DPI_I2C_ADDR (0x45).
     */
    uint8_t i2cAddr;
} waveshare_dsi2dpi_resource_t;

extern const display_operations_t waveshare_dsi2dpi_ops;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

status_t WAVESHARE_DSI2DPI_Init(display_handle_t *handle, const display_config_t *config);
status_t WAVESHARE_DSI2DPI_Deinit(display_handle_t *handle);
status_t WAVESHARE_DSI2DPI_Start(display_handle_t *handle);
status_t WAVESHARE_DSI2DPI_Stop(display_handle_t *handle);

/*! @brief Set backlight brightness.
 *
 * @param handle Display handle.
 * @param brightness 0x00 (off) ~ 0xFF (full).
 */
status_t WAVESHARE_DSI2DPI_SetBrightness(display_handle_t *handle, uint8_t brightness);

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_WAVESHARE_DSI2DPI_H_ */
