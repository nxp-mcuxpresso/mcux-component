/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FSL_PCA9555_H_
#define FSL_PCA9555_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"

/*
 * Change log:
 *
 *   1.0.0
 *     - Initial version
 */

/*!
 * @addtogroup pca9555
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief PCA9555 register memory map.
 *
 * Pin numbering: bits[7:0]  = Port 0 (P0_0~P0_7),
 *                bits[15:8] = Port 1 (P1_0~P1_7).
 * For example, pin 8 = P1_0, pin 15 = P1_7.
 *
 * All register pairs are consecutive; the I2C auto-increment feature allows
 * both ports to be read or written in a single 2-byte transfer starting at
 * the Port 0 register address.
 */
/*! Input port (reading clears the INT output). */
#define PCA9555_INPUT_PORT0        (0x00U)
#define PCA9555_INPUT_PORT1        (0x01U)
/*! Output port (default 0xFF after reset). */
#define PCA9555_OUTPUT_PORT0       (0x02U)
#define PCA9555_OUTPUT_PORT1       (0x03U)
/*! Polarity inversion: 1 = inverted, 0 = original. */
#define PCA9555_POLARITY_INV_PORT0 (0x04U)
#define PCA9555_POLARITY_INV_PORT1 (0x05U)
/*! Configuration (direction): 1 = input (default), 0 = output. */
#define PCA9555_CONFIG_PORT0       (0x06U)
#define PCA9555_CONFIG_PORT1       (0x07U)

/*! @brief PCA9555 total number of I/O pins (P0_0~P0_7, P1_0~P1_7). */
#define PCA9555_PIN_COUNT          (16U)

/*! @brief PCA9555 IO direction. */
typedef enum _pca9555_dir
{
    kPCA9555_Input = 0U, /*!< Set pin as input. */
    kPCA9555_Output,     /*!< Set pin as output. */
} pca9555_dir_t;

/*!
 * @brief Callback dispatch mode. Set to 1 to store a per-pin callback table
 *        in the handle; 0 stores a single global callback.
 *
 * Controlled via Kconfig: set @c CONFIG_PCA9555_CALLBACK_PER_PIN to enable
 * per-pin mode. Can also be defined directly at build time to override.
 */
#ifndef PCA9555_CALLBACK_PER_PIN
#ifdef CONFIG_PCA9555_CALLBACK_PER_PIN
#define PCA9555_CALLBACK_PER_PIN CONFIG_PCA9555_CALLBACK_PER_PIN
#else
#define PCA9555_CALLBACK_PER_PIN 0
#endif
#endif

#if PCA9555_CALLBACK_PER_PIN
/*!
 * @brief PCA9555 per-pin interrupt callback function.
 *
 * Invoked by @ref PCA9555_InterruptHandler for each pin whose state has
 * changed since the previous read.
 *
 * @param pin       The pin that changed state (0~15).
 * @param pinState  Current level of this pin (true = high, false = low).
 * @param userData  User-defined context for this pin's handler.
 */
typedef void (*pca9555_pin_callback_t)(uint8_t pin, bool pinState, void *userData);
#else
/*!
 * @brief PCA9555 interrupt callback function.
 *
 * Called by @ref PCA9555_InterruptHandler when an interrupt is detected.
 * The changed-pin set is computed by software XOR of the current read against
 * the previously stored pin states.
 *
 * @param changedPins  Bitmask of pins whose state changed since the last read.
 * @param pinStates    Bitmask of current pin levels (read from Input Port registers).
 * @param userData     User-defined context pointer passed during callback installation.
 */
typedef void (*pca9555_int_callback_t)(uint16_t changedPins, uint16_t pinStates, void *userData);
#endif

/*!
 * @brief PCA9555 lock/unlock function.
 *
 * Optional critical-section hook stored in the driver handle. When non-NULL,
 * the driver calls this before and after every read-modify-write sequence and
 * the @ref PCA9555_InterruptHandler snapshot.
 *
 * Lock function implementation:
 * - For bare-metal project:
 *   - If the driver will not be called in ISR, the lock function can be NULL.
 *   - If the driver will be called in ISR, the lock function should disable/enable interrupts.
 * - For RTOS-based project:
 *   - If the driver will not be called in ISR, the lock function should acquire/release a mutex.
 *   - If the driver will be called in ISR, the lock function should
 *     use a mutex, and also disable/enable interrupts.
 *
 * @param lock  true  = acquire the lock (enter critical section).
 *              false = release the lock (exit critical section).
 * @return @ref kStatus_Success if the lock operation succeeded; any other value
 *         causes the driver to abort the current operation (only checked on
 *         the acquire call; the release return value is ignored).
 */
typedef status_t (*pca9555_lock_func_t)(bool lock);

/*! @brief PCA9555 I2C receive function. */
typedef status_t (*pca9555_i2c_receive_func_t)(void *base,
                                               uint8_t deviceAddress,
                                               uint32_t subAddress,
                                               uint8_t subaddressSize,
                                               uint8_t *rxBuff,
                                               uint8_t rxBuffSize,
                                               uint32_t flags);

/*! @brief PCA9555 I2C send function. */
typedef status_t (*pca9555_i2c_send_func_t)(void *base,
                                            uint8_t deviceAddress,
                                            uint32_t subAddress,
                                            uint8_t subaddressSize,
                                            const uint8_t *txBuff,
                                            uint8_t txBuffSize,
                                            uint32_t flags);

/*! @brief PCA9555 configure structure. */
typedef struct _pca9555_config
{
    void *i2cBase;                              /*!< I2C instance base address. */
    uint8_t i2cAddr;                            /*!< I2C device address (0x20~0x27). */
    pca9555_i2c_send_func_t I2C_SendFunc;       /*!< Function to send I2C data. */
    pca9555_i2c_receive_func_t I2C_ReceiveFunc; /*!< Function to receive I2C data. */
    pca9555_lock_func_t lockFunc;               /*!< Optional lock/unlock hook. NULL = no critical section. */
} pca9555_config_t;

/*! @brief PCA9555 driver handle. */
typedef struct _pca9555_handle
{
    void *i2cBase;                              /*!< I2C instance base address. */
    uint8_t i2cAddr;                            /*!< I2C device address. */
    pca9555_i2c_send_func_t I2C_SendFunc;       /*!< Function to send I2C data. */
    pca9555_i2c_receive_func_t I2C_ReceiveFunc; /*!< Function to receive I2C data. */
    pca9555_lock_func_t lockFunc;               /*!< Optional lock/unlock hook. NULL = no critical section. */
    uint16_t prevPinStates;                     /*!< Pin states from the last Input Port read; used by
                                                     @ref PCA9555_InterruptHandler to compute the XOR diff. */
#if PCA9555_CALLBACK_PER_PIN
    pca9555_pin_callback_t pinCallbacks[PCA9555_PIN_COUNT]; /*!< Per-pin callbacks, NULL to skip. */
    void *pinUserData[PCA9555_PIN_COUNT];                   /*!< Per-pin user context. */
#else
    pca9555_int_callback_t callback;            /*!< Interrupt callback, NULL if not installed. */
    void *userData;                             /*!< User context passed to callback. */
#endif
} pca9555_handle_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name Initialization
 * @{
 */

/*!
 * @brief Initializes the PCA9555 driver handle.
 *
 * Copies configuration into the handle, clears all callbacks, and primes
 * @c prevPinStates by performing an initial read of the Input Port registers.
 * This initial read also deasserts the INT pin if it was asserted at power-on.
 *
 * @param handle Pointer to the PCA9555 handle.
 * @param config Pointer to the PCA9555 configuration structure.
 * @return Return @ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_Init(pca9555_handle_t *handle, const pca9555_config_t *config);

#if PCA9555_CALLBACK_PER_PIN
/*!
 * @brief Install PCA9555 per-pin interrupt callback.
 *
 * The installed callback is invoked by @ref PCA9555_InterruptHandler for each
 * pin whose state has changed since the previous read.
 *
 * @param handle   Pointer to the PCA9555 handle.
 * @param pin      The pin number (0~15).
 * @param callback Interrupt callback function, or NULL to uninstall.
 * @param userData User-defined context pointer passed to the callback.
 * @return Return @ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_InstallPinCallback(pca9555_handle_t *handle, uint8_t pin,
                                    pca9555_pin_callback_t callback, void *userData);
#else
/*!
 * @brief Install PCA9555 interrupt callback.
 *
 * The installed callback is invoked by @ref PCA9555_InterruptHandler when
 * an interrupt event is detected.
 *
 * @param handle   Pointer to the PCA9555 handle.
 * @param callback Interrupt callback function, or NULL to uninstall.
 * @param userData User-defined context pointer passed to the callback.
 */
void PCA9555_InstallCallback(pca9555_handle_t *handle, pca9555_int_callback_t callback, void *userData);
#endif

/*! @} */

/*!
 * @name GPIO Read/Write
 * @{
 */

/*!
 * @brief Set PCA9555 pins direction.
 *
 * This function sets multiple pins direction, the pins to modify are passed in
 * as a bit OR'ed value.
 *
 * For example, the following code sets pin 2 and pin 3 to output:
 *
 * @code
   PCA9555_SetDirection(handle, (1U<<2) | (1U<<3), kPCA9555_Output);
   @endcode
 *
 * @param handle Pointer to the PCA9555 handle.
 * @param pins   The pins to change, for example: (1U<<2)|(1U<<3) means pin 2 and pin 3.
 * @param dir    Pin direction.
 * @return Return @ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_SetDirection(pca9555_handle_t *handle, uint16_t pins, pca9555_dir_t dir);

/*!
 * @brief Set PCA9555 pins output value to 1.
 *
 * @param handle Pointer to the PCA9555 handle.
 * @param pins   The pins to set high.
 * @return Return @ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_SetPins(pca9555_handle_t *handle, uint16_t pins);

/*!
 * @brief Set PCA9555 pins output value to 0.
 *
 * @param handle Pointer to the PCA9555 handle.
 * @param pins   The pins to set low.
 * @return Return @ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_ClearPins(pca9555_handle_t *handle, uint16_t pins);

/*!
 * @brief Toggle PCA9555 pins output value.
 *
 * @param handle Pointer to the PCA9555 handle.
 * @param pins   The pins to toggle.
 * @return Return @ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_TogglePins(pca9555_handle_t *handle, uint16_t pins);

/*!
 * @brief Read PCA9555 pins value.
 *
 * Reads the Input Port registers (00h~01h), which also clears the INT output
 * as a hardware side-effect. Also updates @c prevPinStates in the handle so
 * that polling and interrupt modes remain in sync.
 *
 * @param handle     Pointer to the PCA9555 handle.
 * @param pinsValue  Variable to store the read pin values.
 * @return Return @ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_ReadPins(pca9555_handle_t *handle, uint16_t *pinsValue);

/*!
 * @brief Inverse PCA9555 pins input polarity.
 *
 * When a pin's polarity-inversion bit is set, the value read from the Input
 * Port register is inverted relative to the actual pin level.
 *
 * @param handle   Pointer to the PCA9555 handle.
 * @param pins     Bitmask of pins to change.
 * @param inverse  Use true to invert, false to restore original polarity.
 * @return Return @ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_InversePolarity(pca9555_handle_t *handle, uint16_t pins, bool inverse);

/*! @} */

/*!
 * @name Interrupt
 * @{
 */

/*!
 * @brief PCA9555 interrupt handler.
 *
 * Call this from task context when the MCU GPIO ISR detects a falling edge on
 * the PCA9555 INT pin. A single-pass approach is used: the Input Port registers
 * are read once (which also clears the INT hardware output), a software XOR
 * diff is computed against @c prevPinStates, and the appropriate callback(s)
 * are invoked.
 *
 * Because reading the Input Port registers is the only mechanism to clear the
 * INT pin on this device, re-triggering naturally occurs if the INT pin is
 * still asserted after this function returns (i.e. a new edge occurred during
 * callback execution). The MCU GPIO ISR will then queue another call.
 *
 * @note This function performs I2C transactions. The suggested way is don't
 * call it directly in ISR context, use a flag or deferred work mechanism to
 * call from task context.
 *
 * @param handle Pointer to the PCA9555 handle.
 * @return Return @ref kStatus_Success if succeeded, otherwise returns error code.
 */
status_t PCA9555_InterruptHandler(pca9555_handle_t *handle);

/*! @} */


#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* FSL_PCA9555_H_ */
