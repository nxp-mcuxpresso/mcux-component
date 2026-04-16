/*
 * Copyright 2022, 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pcal6524.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _pcal6524_reg_ops
{
    kPCAL6524_SetRegBits = 0U,
    kPCAL6524_ClearRegBits,
    kPCAL6524_ToggleRegBits,
} pcal6524_reg_ops_t;

/* Weak no-op default; user code may provide a strong override. See header. */
__attribute__((weak)) void PCAL6524_EnterCritical(pcal6524_handle_t *handle)
{
    (void)handle;
}

__attribute__((weak)) void PCAL6524_ExitCritical(pcal6524_handle_t *handle)
{
    (void)handle;
}

/*******************************************************************************
 * Code
 ******************************************************************************/

/*! @brief Read 3 consecutive port registers (port 0/1/2) into a uint32_t. */
static status_t PCAL6524_ReadPort(pcal6524_handle_t *handle, uint8_t startReg, uint32_t *value)
{
    *value = 0U;
    return handle->I2C_ReceiveFunc(handle->i2cBase, handle->i2cAddr, startReg, 1U, (uint8_t *)value, 3U, 0U);
}

/*! @brief Write 3 consecutive port registers (port 0/1/2) from a uint32_t. */
static status_t PCAL6524_WritePort(pcal6524_handle_t *handle, uint8_t startReg, uint32_t value)
{
    return handle->I2C_SendFunc(handle->i2cBase, handle->i2cAddr, startReg, 1U, (uint8_t *)&value, 3U, 0U);
}

/*! @brief Read a single register. */
static status_t PCAL6524_ReadReg(pcal6524_handle_t *handle, uint8_t reg, uint8_t *value)
{
    return handle->I2C_ReceiveFunc(handle->i2cBase, handle->i2cAddr, reg, 1U, value, 1U, 0U);
}

/*! @brief Write a single register. */
static status_t PCAL6524_WriteReg(pcal6524_handle_t *handle, uint8_t reg, uint8_t value)
{
    return handle->I2C_SendFunc(handle->i2cBase, handle->i2cAddr, reg, 1U, &value, 1U, 0U);
}

/*! @brief Read-modify-write on a 3-register port group. */
static status_t PCAL6524_ModifyPortRegBits(pcal6524_handle_t *handle,
                                           uint8_t startReg,
                                           uint32_t bits,
                                           pcal6524_reg_ops_t ops)
{
    uint32_t regValue = 0U;
    status_t status;

    PCAL6524_EnterCritical(handle);

    status = PCAL6524_ReadPort(handle, startReg, &regValue);

    if (kStatus_Success == status)
    {
        if (kPCAL6524_SetRegBits == ops)
        {
            regValue |= bits;
        }
        else if (kPCAL6524_ClearRegBits == ops)
        {
            regValue &= ~bits;
        }
        else if (kPCAL6524_ToggleRegBits == ops)
        {
            regValue ^= bits;
        }
        else
        {
            /* Add for MISRA 15.7 */
        }

        status = PCAL6524_WritePort(handle, startReg, regValue);
    }

    PCAL6524_ExitCritical(handle);
    return status;
}

/*
 * brief Initializes the PCAL6524 driver handle.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param config Pointer to the PCAL6524 configuration structure.
 */
void PCAL6524_Init(pcal6524_handle_t *handle, const pcal6524_config_t *config)
{
    assert(NULL != handle);
    assert(NULL != config);

    handle->i2cBase         = config->i2cBase;
    handle->i2cAddr         = config->i2cAddr;
    handle->I2C_SendFunc    = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc = config->I2C_ReceiveFunc;

#if PCAL6524_CALLBACK_PER_PIN
    for (uint8_t i = 0U; i < PCAL6524_PIN_COUNT; i++)
    {
        handle->pinCallbacks[i] = NULL;
        handle->pinUserData[i]  = NULL;
    }
#else
    handle->callback = NULL;
    handle->userData = NULL;
#endif
}

#if PCAL6524_CALLBACK_PER_PIN
/*
 * brief Install PCAL6524 per-pin interrupt callback.
 *
 * param handle   Pointer to the PCAL6524 handle.
 * param pin      The pin number (0~23).
 * param callback Interrupt callback function, or NULL to uninstall.
 * param userData User-defined context pointer passed to the callback.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_InstallPinCallback(pcal6524_handle_t *handle, uint8_t pin,
                                     pcal6524_pin_callback_t callback, void *userData)
{
    assert(NULL != handle);

    if (pin >= PCAL6524_PIN_COUNT)
    {
        return kStatus_InvalidArgument;
    }

    handle->pinCallbacks[pin] = callback;
    handle->pinUserData[pin]  = userData;
    return kStatus_Success;
}
#else
/*
 * brief Install PCAL6524 interrupt callback.
 *
 * param handle   Pointer to the PCAL6524 handle.
 * param callback Interrupt callback function, or NULL to uninstall.
 * param userData User-defined context pointer passed to the callback.
 */
void PCAL6524_InstallCallback(pcal6524_handle_t *handle, pcal6524_int_callback_t callback, void *userData)
{
    assert(NULL != handle);

    handle->callback = callback;
    handle->userData = userData;
}
#endif

/*
 * brief Set PCAL6524 pins direction.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * param dir Pin direction.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetDirection(pcal6524_handle_t *handle, uint32_t pins, pcal6524_dir_t dir)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_CONFIG_PORT0, pins,
                                      (kPCAL6524_Output != dir) ? kPCAL6524_SetRegBits : kPCAL6524_ClearRegBits);
}

/*
 * brief Set PCAL6524 pins output value to 1.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPins(pcal6524_handle_t *handle, uint32_t pins)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_OUTPUT_PORT0, pins, kPCAL6524_SetRegBits);
}

/*
 * brief Set PCAL6524 pins output value to 0.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ClearPins(pcal6524_handle_t *handle, uint32_t pins)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_OUTPUT_PORT0, pins, kPCAL6524_ClearRegBits);
}

/*
 * brief Toggle PCAL6524 pins output value.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins The pins to toggle, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_TogglePins(pcal6524_handle_t *handle, uint32_t pins)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_OUTPUT_PORT0, pins, kPCAL6524_ToggleRegBits);
}

/*
 * brief Read PCAL6524 pins value.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pinsValue Variable to save the read out pin values.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ReadPins(pcal6524_handle_t *handle, uint32_t *pinsValue)
{
    return PCAL6524_ReadPort(handle, PCAL6524_INPUT_STATUS_PORT0, pinsValue);
}

/*
 * brief Read PCAL6524 pins value and clear all interrupts.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pinsValue Variable to save the read out pin values.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ReadPinsAndClearInterrupt(pcal6524_handle_t *handle, uint32_t *pinsValue)
{
    return PCAL6524_ReadPort(handle, PCAL6524_INPUT_PORT0, pinsValue);
}

/*
 * brief Inverse PCAL6524 pins input polarity.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins Bitmask of pins to change.
 * param inverse Use true to inverse, false to not inverse.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_InversePolarity(pcal6524_handle_t *handle, uint32_t pins, bool inverse)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_POLARITY_INV_PORT0, pins,
                                      inverse ? kPCAL6524_SetRegBits : kPCAL6524_ClearRegBits);
}

/*
 * brief Set PCAL6524 individual pin output configuration (push-pull or open-drain).
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins Bitmask of pins to change.
 * param openDrain Use true for open-drain, false for push-pull.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPinOutputConfig(pcal6524_handle_t *handle, uint32_t pins, bool openDrain)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_INDIV_PIN_OUT_PORT0, pins,
                                      openDrain ? kPCAL6524_SetRegBits : kPCAL6524_ClearRegBits);
}

/*
 * brief Set PCAL6524 output port configuration (push-pull or open-drain).
 *
 * param handle Pointer to the PCAL6524 handle.
 * param portMask The ports to change (OR of PCAL6524_PORTx_MASK values).
 * param openDrain Use true for open-drain, false for push-pull.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetOutputPortConfig(pcal6524_handle_t *handle, uint8_t portMask, bool openDrain)
{
    uint8_t regValue;
    status_t status;

    PCAL6524_EnterCritical(handle);

    status = PCAL6524_ReadReg(handle, PCAL6524_OUTPUT_PORT_CONFIG, &regValue);

    if (kStatus_Success == status)
    {
        if (openDrain)
        {
            regValue |= (portMask & 0x07U);
        }
        else
        {
            regValue &= ~(portMask & 0x07U);
        }
        status = PCAL6524_WriteReg(handle, PCAL6524_OUTPUT_PORT_CONFIG, regValue);
    }

    PCAL6524_ExitCritical(handle);
    return status;
}

/*
 * brief Set PCAL6524 output drive strength for a specific pin.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pin The pin number (0~23). Pins 0~7 are port 0, 8~15 are port 1, 16~23 are port 2.
 * param strength The drive strength level.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetOutputDriveStrength(pcal6524_handle_t *handle,
                                         uint8_t pin,
                                         pcal6524_drive_strength_t strength)
{
    uint8_t reg;
    uint8_t shift;
    uint8_t regValue;
    status_t status;

    if (pin >= PCAL6524_PIN_COUNT)
    {
        return kStatus_InvalidArgument;
    }

    reg   = PCAL6524_OUT_DRIVE_PORT0A + (pin / 4U);
    shift = (pin % 4U) * 2U;

    PCAL6524_EnterCritical(handle);

    status = PCAL6524_ReadReg(handle, reg, &regValue);

    if (kStatus_Success == status)
    {
        regValue &= ~((uint8_t)0x03U << shift);
        regValue |= ((uint8_t)strength & 0x03U) << shift;
        status = PCAL6524_WriteReg(handle, reg, regValue);
    }

    PCAL6524_ExitCritical(handle);
    return status;
}

/*
 * brief Enable or disable PCAL6524 pull-up/pull-down resistors.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins Bitmask of pins to change.
 * param enable Use true to enable pull resistor, false to disable.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPullEnable(pcal6524_handle_t *handle, uint32_t pins, bool enable)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_PULL_ENABLE_PORT0, pins,
                                      enable ? kPCAL6524_SetRegBits : kPCAL6524_ClearRegBits);
}

/*
 * brief Select PCAL6524 pull-up or pull-down resistor.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins Bitmask of pins to change.
 * param pullUp Use true to select 100 kohm pull-up, false for pull-down.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPullSelection(pcal6524_handle_t *handle, uint32_t pins, bool pullUp)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_PULL_SELECT_PORT0, pins,
                                      pullUp ? kPCAL6524_SetRegBits : kPCAL6524_ClearRegBits);
}

/*
 * brief Set PCAL6524 input latch.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins Bitmask of pins to change.
 * param enable Use true to enable latch, false to disable latch.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetInputLatch(pcal6524_handle_t *handle, uint32_t pins, bool enable)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_INPUT_LATCH_PORT0, pins,
                                      enable ? kPCAL6524_SetRegBits : kPCAL6524_ClearRegBits);
}

/*
 * brief Configure PCAL6524 pin interrupt in one call.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pin The pin number (0~23).
 * param edge The interrupt edge type. Use kPCAL6524_IntEdgeLevel to disable.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPinInterruptConfig(pcal6524_handle_t *handle, uint8_t pin, pcal6524_int_edge_t edge)
{
    status_t status;
    uint32_t pinMask;

    if (pin >= PCAL6524_PIN_COUNT)
    {
        return kStatus_InvalidArgument;
    }

    pinMask = (1U << pin);

    /* Step 1: Mask first, so edge/latch reconfiguration cannot fire spurious
     * interrupts while in progress. */
    status = PCAL6524_DisableInterrupts(handle, pinMask);
    if (kStatus_Success != status)
    {
        return status;
    }

    /* Step 2: Configure edge type. */
    status = PCAL6524_SetInterruptEdge(handle, pin, edge);
    if (kStatus_Success != status)
    {
        return status;
    }

    if (kPCAL6524_IntEdgeLevel == edge)
    {
        /* Caller asked to disable: drop latch too and stay masked. */
        return PCAL6524_SetInputLatch(handle, pinMask, false);
    }

    /* Step 3: Enable latch so a fast transient after unmask is not lost. */
    status = PCAL6524_SetInputLatch(handle, pinMask, true);
    if (kStatus_Success != status)
    {
        return status;
    }

    /* Step 4: Clear anything that latched during reconfig. */
    status = PCAL6524_ClearInterruptPins(handle, pinMask);
    if (kStatus_Success != status)
    {
        return status;
    }

    /* Step 5: Unmask. */
    return PCAL6524_EnableInterrupts(handle, pinMask);
}

/*
 * brief Enable PCAL6524 interrupt for specific pins.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins Bitmask of pins to enable interrupt on.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_EnableInterrupts(pcal6524_handle_t *handle, uint32_t pins)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_INT_MASK_PORT0, pins, kPCAL6524_ClearRegBits);
}

/*
 * brief Disable PCAL6524 interrupt for specific pins.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins Bitmask of pins to disable interrupt on.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_DisableInterrupts(pcal6524_handle_t *handle, uint32_t pins)
{
    return PCAL6524_ModifyPortRegBits(handle, PCAL6524_INT_MASK_PORT0, pins, kPCAL6524_SetRegBits);
}

/*
 * brief Set PCAL6524 interrupt edge type for a specific pin.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pin The pin number (0~23).
 * param edge The interrupt edge type.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetInterruptEdge(pcal6524_handle_t *handle, uint8_t pin, pcal6524_int_edge_t edge)
{
    uint8_t reg;
    uint8_t shift;
    uint8_t regValue;
    status_t status;

    if (pin >= PCAL6524_PIN_COUNT)
    {
        return kStatus_InvalidArgument;
    }

    reg   = PCAL6524_INT_EDGE_PORT0A + (pin / 4U);
    shift = (pin % 4U) * 2U;

    PCAL6524_EnterCritical(handle);

    status = PCAL6524_ReadReg(handle, reg, &regValue);

    if (kStatus_Success == status)
    {
        regValue &= ~((uint8_t)0x03U << shift);
        regValue |= ((uint8_t)edge & 0x03U) << shift;
        status = PCAL6524_WriteReg(handle, reg, regValue);
    }

    PCAL6524_ExitCritical(handle);
    return status;
}

/*
 * brief Get PCAL6524 interrupt status.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pinsStatus Variable to save the interrupt status.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_GetInterruptStatus(pcal6524_handle_t *handle, uint32_t *pinsStatus)
{
    return PCAL6524_ReadPort(handle, PCAL6524_INT_STATUS_PORT0, pinsStatus);
}

/*
 * brief Clear PCAL6524 interrupt for specific pins.
 *
 * param handle Pointer to the PCAL6524 handle.
 * param pins Bitmask of pins to clear interrupt.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ClearInterruptPins(pcal6524_handle_t *handle, uint32_t pins)
{
    return PCAL6524_WritePort(handle, PCAL6524_INT_CLEAR_PORT0, pins);
}

/*
 * brief PCAL6524 interrupt handler.
 *
 * note This function performs I2C transactions and must NOT be called from
 * ISR context.
 *
 * param handle Pointer to the PCAL6524 handle.
 * return Return ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_InterruptHandler(pcal6524_handle_t *handle)
{
    uint32_t intStatus = 0U;
    uint32_t pinStates = 0U;
    status_t status;

    for (;;)
    {
        /* Under lock: read the pending-interrupt snapshot and clear it so
         * this handler owns those events. If another task (or a re-entry
         * from within a callback) calls us concurrently, only one of them
         * sees and clears each event. */
        PCAL6524_EnterCritical(handle);

        status = PCAL6524_ReadPort(handle, PCAL6524_INT_STATUS_PORT0, &intStatus);
        if (kStatus_Success != status)
        {
            PCAL6524_ExitCritical(handle);
            return status;
        }

        /* No (more) interrupts pending, we're done. */
        if (0U == (intStatus & 0x00FFFFFFU))
        {
            PCAL6524_ExitCritical(handle);
            return kStatus_Success;
        }

        status = PCAL6524_ReadPort(handle, PCAL6524_INPUT_STATUS_PORT0, &pinStates);
        if (kStatus_Success != status)
        {
            PCAL6524_ExitCritical(handle);
            return status;
        }

        /* Clear the interrupts we are about to dispatch. Any new edge that
         * fires during callback execution will be latched again in the chip
         * and picked up by the next loop iteration. */
        status = PCAL6524_WritePort(handle, PCAL6524_INT_CLEAR_PORT0, intStatus);
        if (kStatus_Success != status)
        {
            PCAL6524_ExitCritical(handle);
            return status;
        }

        PCAL6524_ExitCritical(handle);

        /* Dispatch callback(s) with the lock released so a callback is free
         * to call other driver APIs (SetPins, etc.) without deadlocking. An
         * edge-triggered MCU GPIO ISR cannot retrigger while INT stays
         * asserted, so we loop back to drain any new events. */
#if PCAL6524_CALLBACK_PER_PIN
        for (uint8_t pin = 0U; pin < PCAL6524_PIN_COUNT; pin++)
        {
            uint32_t pinMask = (1U << pin);

            if ((0U != (intStatus & pinMask)) && (NULL != handle->pinCallbacks[pin]))
            {
                handle->pinCallbacks[pin](pin, (0U != (pinStates & pinMask)), handle->pinUserData[pin]);
            }
        }
#else
        if (NULL != handle->callback)
        {
            handle->callback(intStatus, pinStates, handle->userData);
        }
#endif
    }
}
