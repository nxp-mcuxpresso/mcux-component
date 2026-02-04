/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pi4io6408.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _pi4io6408_reg_ops
{
    kPI4IO6408_SetRegBits = 0,
    kPI4IO6408_ClearRegBits,
    kPI4IO6408_ToggleRegBits,
} pi4io6408_reg_ops_t;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*! @brief Read port register value. */
static status_t PI4IO6408_ReadPort(pi4io6408_handle_t *handle,
                                   uint8_t reg,
                                   uint8_t *value)
{
    return handle->I2C_ReceiveFunc(handle->i2cBase,
                                   handle->i2cAddr,
                                   reg, 1U,
                                   value, 1U,
                                   0U);
}

/*! @brief Write port register value. */
static status_t PI4IO6408_WritePort(pi4io6408_handle_t *handle,
                                    uint8_t reg,
                                    uint8_t value)
{
    return handle->I2C_SendFunc(handle->i2cBase,
                                handle->i2cAddr,
                                reg, 1U,
                                &value, 1U,
                                0U);
}

/*! @brief Modify PI4IO6408 port register bits. */
static status_t PI4IO6408_ModifyPortRegBits(pi4io6408_handle_t *handle,
                                            uint8_t reg,
                                            uint8_t bits,
                                            pi4io6408_reg_ops_t ops)
{
    uint8_t regValue;
    status_t status;

    status = PI4IO6408_ReadPort(handle, reg, &regValue);
    if (status != kStatus_Success)
    {
        return status;
    }

    if (ops == kPI4IO6408_SetRegBits)
    {
        regValue |= bits;
    }
    else if (ops == kPI4IO6408_ClearRegBits)
    {
        regValue &= (uint8_t)~bits;
    }
    else
    {
        regValue ^= bits;
    }

    return PI4IO6408_WritePort(handle, reg, regValue);
}

/*!
 * @brief Initializes the PI4IO6408 driver handle.
 *
 * @param handle Pointer to the PI4IO6408 handle.
 * @param config Pointer to the PI4IO6408 configuration structure.
 */
void PI4IO6408_Init(pi4io6408_handle_t *handle,
                    const pi4io6408_config_t *config)
{
    handle->i2cBase         = config->i2cBase;
    handle->i2cAddr         = config->i2cAddr;
    handle->I2C_SendFunc    = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc = config->I2C_ReceiveFunc;
}

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
status_t PI4IO6408_EnableAllPinOutput(pi4io6408_handle_t *handle)
{
    status_t status;

    status = PI4IO6408_WritePort(handle,
                                 PI4IO6408_CONFIG_PORT,
                                 0xFFU);
    if (status != kStatus_Success)
    {
        return status;
    }

    return PI4IO6408_WritePort(handle,
                               PI4IO6408_OUTPUT_HIZ_PORT,
                               0x00U);
}

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
                                pi4io6408_dir_t dir)
{
    status_t status;

    if (dir == kPI4IO6408_Output)
    {
        status = PI4IO6408_ModifyPortRegBits(handle,
                                             PI4IO6408_CONFIG_PORT,
                                             pins,
                                             kPI4IO6408_SetRegBits);
        if (status != kStatus_Success)
        {
            return status;
        }

        return PI4IO6408_ModifyPortRegBits(handle,
                                           PI4IO6408_OUTPUT_HIZ_PORT,
                                           pins,
                                           kPI4IO6408_ClearRegBits);
    }
    else
    {
        status = PI4IO6408_ModifyPortRegBits(handle,
                                             PI4IO6408_CONFIG_PORT,
                                             pins,
                                             kPI4IO6408_ClearRegBits);
        if (status != kStatus_Success)
        {
            return status;
        }

        return PI4IO6408_ModifyPortRegBits(handle,
                                           PI4IO6408_OUTPUT_HIZ_PORT,
                                           pins,
                                           kPI4IO6408_SetRegBits);
    }
}

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
status_t PI4IO6408_SetPins(pi4io6408_handle_t *handle, uint8_t pins)
{
    return PI4IO6408_ModifyPortRegBits(handle,
                                       PI4IO6408_OUTPUT_PORT,
                                       pins,
                                       kPI4IO6408_SetRegBits);
}

/*!
 * @brief Set PI4IO6408 pins output value to 0.
 *
 * @param handle Pointer to the PI4IO6408 handle.
 * @param pins Pins to set low.
 * @return kStatus_Success if successed, otherwise returns error code.
 */
status_t PI4IO6408_ClearPins(pi4io6408_handle_t *handle, uint8_t pins)
{
    return PI4IO6408_ModifyPortRegBits(handle,
                                       PI4IO6408_OUTPUT_PORT,
                                       pins,
                                       kPI4IO6408_ClearRegBits);
}

/*!
 * @brief Toggle PI4IO6408 pins output value.
 *
 * @param handle Pointer to the PI4IO6408 handle.
 * @param pins Pins to toggle.
 * @return kStatus_Success if successed, otherwise returns error code.
 */
status_t PI4IO6408_TogglePins(pi4io6408_handle_t *handle, uint8_t pins)
{
    return PI4IO6408_ModifyPortRegBits(handle,
                                       PI4IO6408_OUTPUT_PORT,
                                       pins,
                                       kPI4IO6408_ToggleRegBits);
}
