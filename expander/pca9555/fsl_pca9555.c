/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pca9555.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _pca9555_reg_ops
{
    kPCA9555_SetRegBits = 0U,
    kPCA9555_ClearRegBits,
    kPCA9555_ToggleRegBits,
} pca9555_reg_ops_t;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*! @brief Read 2 consecutive port registers (port 0/1) into a uint16_t. */
static status_t PCA9555_ReadPort(pca9555_handle_t *handle, uint8_t startReg, uint16_t *value)
{
    *value = 0U;
    return handle->I2C_ReceiveFunc(handle->i2cBase, handle->i2cAddr, startReg, 1U, (uint8_t *)value, 2U, 0U);
}

/*! @brief Write 2 consecutive port registers (port 0/1) from a uint16_t. */
static status_t PCA9555_WritePort(pca9555_handle_t *handle, uint8_t startReg, uint16_t value)
{
    return handle->I2C_SendFunc(handle->i2cBase, handle->i2cAddr, startReg, 1U, (const uint8_t *)&value, 2U, 0U);
}

/*! @brief Read-modify-write on a 2-register port group. */
static status_t PCA9555_ModifyPortRegBits(pca9555_handle_t *handle,
                                          uint8_t startReg,
                                          uint16_t bits,
                                          pca9555_reg_ops_t ops)
{
    uint16_t regValue = 0U;
    status_t status;

    if (NULL != handle->lockFunc)
    {
        status = handle->lockFunc(true);
        if (kStatus_Success != status)
        {
            return status;
        }
    }

    status = PCA9555_ReadPort(handle, startReg, &regValue);

    if (kStatus_Success == status)
    {
        if (kPCA9555_SetRegBits == ops)
        {
            regValue |= bits;
        }
        else if (kPCA9555_ClearRegBits == ops)
        {
            regValue &= ~bits;
        }
        else if (kPCA9555_ToggleRegBits == ops)
        {
            regValue ^= bits;
        }
        else
        {
            /* Add for MISRA 15.7 */
        }

        status = PCA9555_WritePort(handle, startReg, regValue);
    }

    if (NULL != handle->lockFunc)
    {
        (void)handle->lockFunc(false);
    }
    return status;
}

/*
 * brief Initializes the PCA9555 driver handle.
 *
 * param handle Pointer to the PCA9555 handle.
 * param config Pointer to the PCA9555 configuration structure.
 */
status_t PCA9555_Init(pca9555_handle_t *handle, const pca9555_config_t *config)
{
    assert(NULL != handle);
    assert(NULL != config);

    handle->i2cBase         = config->i2cBase;
    handle->i2cAddr         = config->i2cAddr;
    handle->I2C_SendFunc    = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc = config->I2C_ReceiveFunc;
    handle->lockFunc        = config->lockFunc;

#if PCA9555_CALLBACK_PER_PIN
    for (uint8_t i = 0U; i < PCA9555_PIN_COUNT; i++)
    {
        handle->pinCallbacks[i] = NULL;
        handle->pinUserData[i]  = NULL;
    }
#else
    handle->callback = NULL;
    handle->userData = NULL;
#endif

    /* Prime prevPinStates with the current hardware state. This also clears
     * the INT output if it was asserted at power-on. */
    return PCA9555_ReadPort(handle, PCA9555_INPUT_PORT0, &handle->prevPinStates);
}

#if PCA9555_CALLBACK_PER_PIN
/*
 * brief Install PCA9555 per-pin interrupt callback.
 *
 * param handle   Pointer to the PCA9555 handle.
 * param pin      The pin number (0~15).
 * param callback Interrupt callback function, or NULL to uninstall.
 * param userData User-defined context pointer passed to the callback.
 * return Return ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_InstallPinCallback(pca9555_handle_t *handle, uint8_t pin,
                                    pca9555_pin_callback_t callback, void *userData)
{
    assert(NULL != handle);

    if (pin >= PCA9555_PIN_COUNT)
    {
        return kStatus_InvalidArgument;
    }

    handle->pinCallbacks[pin] = callback;
    handle->pinUserData[pin]  = userData;
    return kStatus_Success;
}
#else
/*
 * brief Install PCA9555 interrupt callback.
 *
 * param handle   Pointer to the PCA9555 handle.
 * param callback Interrupt callback function, or NULL to uninstall.
 * param userData User-defined context pointer passed to the callback.
 */
void PCA9555_InstallCallback(pca9555_handle_t *handle, pca9555_int_callback_t callback, void *userData)
{
    assert(NULL != handle);

    handle->callback = callback;
    handle->userData = userData;
}
#endif

/*
 * brief Set PCA9555 pins direction.
 *
 * param handle Pointer to the PCA9555 handle.
 * param pins The pins to change.
 * param dir Pin direction.
 * return Return ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_SetDirection(pca9555_handle_t *handle, uint16_t pins, pca9555_dir_t dir)
{
    /* Config register: bit=1 → input, bit=0 → output */
    return PCA9555_ModifyPortRegBits(handle, PCA9555_CONFIG_PORT0, pins,
                                     (kPCA9555_Output != dir) ? kPCA9555_SetRegBits : kPCA9555_ClearRegBits);
}

/*
 * brief Set PCA9555 pins output value to 1.
 *
 * param handle Pointer to the PCA9555 handle.
 * param pins The pins to set high.
 * return Return ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_SetPins(pca9555_handle_t *handle, uint16_t pins)
{
    return PCA9555_ModifyPortRegBits(handle, PCA9555_OUTPUT_PORT0, pins, kPCA9555_SetRegBits);
}

/*
 * brief Set PCA9555 pins output value to 0.
 *
 * param handle Pointer to the PCA9555 handle.
 * param pins The pins to set low.
 * return Return ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_ClearPins(pca9555_handle_t *handle, uint16_t pins)
{
    return PCA9555_ModifyPortRegBits(handle, PCA9555_OUTPUT_PORT0, pins, kPCA9555_ClearRegBits);
}

/*
 * brief Toggle PCA9555 pins output value.
 *
 * param handle Pointer to the PCA9555 handle.
 * param pins The pins to toggle.
 * return Return ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_TogglePins(pca9555_handle_t *handle, uint16_t pins)
{
    return PCA9555_ModifyPortRegBits(handle, PCA9555_OUTPUT_PORT0, pins, kPCA9555_ToggleRegBits);
}

/*
 * brief Read PCA9555 pins value.
 *
 * Reads the Input Port registers and updates prevPinStates so that polling
 * and interrupt modes do not diverge.
 *
 * param handle    Pointer to the PCA9555 handle.
 * param pinsValue Variable to store the read pin values.
 * return Return ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_ReadPins(pca9555_handle_t *handle, uint16_t *pinsValue)
{
    uint16_t pinStates = 0U;
    status_t status;

    if (NULL != handle->lockFunc)
    {
        status = handle->lockFunc(true);
        if (kStatus_Success != status)
        {
            return status;
        }
    }
    status = PCA9555_ReadPort(handle, PCA9555_INPUT_PORT0, &pinStates);
    if (kStatus_Success == status)
    {
        handle->prevPinStates = pinStates;
        *pinsValue            = pinStates;
    }
    if (NULL != handle->lockFunc)
    {
        (void)handle->lockFunc(false);
    }

    return status;
}

/*
 * brief Inverse PCA9555 pins input polarity.
 *
 * param handle  Pointer to the PCA9555 handle.
 * param pins    Bitmask of pins to change.
 * param inverse Use true to invert, false to restore original polarity.
 * return Return ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_InversePolarity(pca9555_handle_t *handle, uint16_t pins, bool inverse)
{
    return PCA9555_ModifyPortRegBits(handle, PCA9555_POLARITY_INV_PORT0, pins,
                                     inverse ? kPCA9555_SetRegBits : kPCA9555_ClearRegBits);
}

/*
 * brief PCA9555 interrupt handler.
 *
 * note This function performs I2C transactions and must NOT be called from
 * ISR context.
 *
 * param handle Pointer to the PCA9555 handle.
 * return Return ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_InterruptHandler(pca9555_handle_t *handle)
{
    uint16_t pinStates   = 0U;
    uint16_t changedPins = 0U;
    status_t status;

    /* Under lock: read current pin states (clears INT hardware), compute XOR
     * diff against the last known state, and update prevPinStates atomically. */
    if (NULL != handle->lockFunc)
    {
        status = handle->lockFunc(true);
        if (kStatus_Success != status)
        {
            return status;
        }
    }

    status = PCA9555_ReadPort(handle, PCA9555_INPUT_PORT0, &pinStates);
    if (kStatus_Success != status)
    {
        if (NULL != handle->lockFunc)
        {
            (void)handle->lockFunc(false);
        }
        return status;
    }

    changedPins           = pinStates ^ handle->prevPinStates;
    handle->prevPinStates = pinStates;

    if (NULL != handle->lockFunc)
    {
        (void)handle->lockFunc(false);
    }

    /* Dispatch callback(s) with the lock released so a callback is free
     * to call other driver APIs (SetPins, etc.) without deadlocking. */
#if PCA9555_CALLBACK_PER_PIN
    for (uint8_t pin = 0U; pin < PCA9555_PIN_COUNT; pin++)
    {
        uint16_t pinMask = (uint16_t)(1U << pin);

        if ((0U != (changedPins & pinMask)) && (NULL != handle->pinCallbacks[pin]))
        {
            handle->pinCallbacks[pin](pin, (0U != (pinStates & pinMask)), handle->pinUserData[pin]);
        }
    }
#else
    if ((0U != changedPins) && (NULL != handle->callback))
    {
        handle->callback(changedPins, pinStates, handle->userData);
    }
#endif

    return kStatus_Success;
}
