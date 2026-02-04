/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_PI4IO6408_H_
#define _FSL_PI4IO6408_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"

/*
 * Change log:
 *
 *   1.0.0
 *     - Initial version.
 */

/*!
 * @addtogroup pi4io6408
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Define the Register Memory Map of PI4IOE5V6408. */
#define PI4IO6408_OUTPUT_PORT        (0x05U) /*!< Output value register. */
#define PI4IO6408_CONFIG_PORT        (0x03U) /*!< Direction register: 1=output, 0=input. */
#define PI4IO6408_OUTPUT_HIZ_PORT    (0x07U) /*!< Hi-Z control register: 1=Hi-Z, 0=drive. */

/*! @brief PI4IO6408 IO direction.
 *
 * NOTE:
 *   This is different from PCAL6408.
 *   PI4IOE5V6408 uses:
 *     - 1 = Output
 *     - 0 = Input
 */
typedef enum _pi4io6408_dir
{
    kPI4IO6408_Input = 0U,  /*!< Set pin as input. */
    kPI4IO6408_Output,      /*!< Set pin as output. */
} pi4io6408_dir_t;

/*! @brief PI4IO6408 I2C receive function. */
typedef status_t (*pi4io6408_i2c_receive_func_t)(void *base,
                                                 uint8_t deviceAddress,
                                                 uint32_t subAddress,
                                                 uint8_t subaddressSize,
                                                 uint8_t *rxBuff,
                                                 uint8_t rxBuffSize,
                                                 uint32_t flags);

/*! @brief PI4IO6408 I2C send function. */
typedef status_t (*pi4io6408_i2c_send_func_t)(void *base,
                                              uint8_t deviceAddress,
                                              uint32_t subAddress,
                                              uint8_t subaddressSize,
                                              const uint8_t *txBuff,
                                              uint8_t txBuffSize,
                                              uint32_t flags);

/*! @brief PI4IO6408 configure structure. */
typedef struct _pi4io6408_config
{
    void *i2cBase;                               /*!< I2C instance base address. */
    uint8_t i2cAddr;                             /*!< I2C address. */
    pi4io6408_i2c_send_func_t    I2C_SendFunc;   /*!< Function to send I2C data. */
    pi4io6408_i2c_receive_func_t I2C_ReceiveFunc;/*!< Function to receive I2C data. */
} pi4io6408_config_t;

/*! @brief PI4IO6408 driver handle. */
typedef struct _pi4io6408_handle
{
    void *i2cBase;                               /*!< I2C instance base address. */
    uint8_t i2cAddr;                             /*!< I2C address. */
    pi4io6408_i2c_send_func_t    I2C_SendFunc;   /*!< Function to send I2C data. */
    pi4io6408_i2c_receive_func_t I2C_ReceiveFunc;/*!< Function to receive I2C data. */
} pi4io6408_handle_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name Initialization and Control function
 * @{
 */

/*!
 * @brief Initializes the PI4IO6408 driver handle.
 *
 * @param handle Pointer to the PI4IO6408 handle.
 * @param config Pointer to the PI4IO6408 configuration structure.
 */
void PI4IO6408_Init(pi4io6408_handle_t *handle,
                    const pi4io6408_config_t *config);

/*!
 * @brief Configure all PI4IO6408 pins as outputs.
 *
 * This function sets all pins to output mode and disables Hi-Z, so that
 * all outputs are actively driven. This function is typically called once
 * during system initialization.
 *
 * @param handle Pointer to the PI4IO6408 handle.
 * @return kStatus_Success if successed, otherwise returns error code.
 */
status_t PI4IO6408_EnableAllPinOutput(pi4io6408_handle_t *handle);

/*!
 * @brief Set PI4IO6408 pins direction.
 *
 * This function sets multiple pins direction; the pins to modify are passed in
 * as a bit OR'ed value.
 *
 * The Hi-Z state is updated together with the direction:
 *   - Input  -> Hi-Z enabled
 *   - Output -> Hi-Z disabled
 *
 * Example:
 * @code
 *   PI4IO6408_SetDirection(handle, (1U<<2) | (1U<<3), kPI4IO6408_Output);
 * @endcode
 *
 * @param handle Pointer to the PI4IO6408 handle.
 * @param pins Pins to change direction.
 * @param dir Pin direction.
 * @return kStatus_Success if successed, otherwise returns error code.
 */
status_t PI4IO6408_SetDirection(pi4io6408_handle_t *handle,
                                uint8_t pins,
                                pi4io6408_dir_t dir);

/*!
 * @brief Set PI4IO6408 pins output value to 1.
 *
 * Example:
 * @code
 *   PI4IO6408_SetPins(handle, (1U<<1) | (1U<<4));
 * @endcode
 *
 * @param handle Pointer to the PI4IO6408 handle.
 * @param pins Pins to set high.
 * @return kStatus_Success if successed, otherwise returns error code.
 */
status_t PI4IO6408_SetPins(pi4io6408_handle_t *handle, uint8_t pins);

/*!
 * @brief Set PI4IO6408 pins output value to 0.
 *
 * @param handle Pointer to the PI4IO6408 handle.
 * @param pins Pins to set low.
 * @return kStatus_Success if successed, otherwise returns error code.
 */
status_t PI4IO6408_ClearPins(pi4io6408_handle_t *handle, uint8_t pins);

/*!
 * @brief Toggle PI4IO6408 pins output value.
 *
 * @param handle Pointer to the PI4IO6408 handle.
 * @param pins Pins to toggle.
 * @return kStatus_Success if successed, otherwise returns error code.
 */
status_t PI4IO6408_TogglePins(pi4io6408_handle_t *handle, uint8_t pins);

/* @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* _FSL_PI4IO6408_H_ */


/*******************************************************************************
 * EOF
 ******************************************************************************/
