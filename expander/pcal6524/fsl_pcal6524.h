/*
 * Copyright 2021, 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_PCAL6524_H_
#define _FSL_PCAL6524_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"

/*
 * Change log:
 *
 *   1.1.0
 *     - Add pin state read APIs (non-destructive and interrupt-clearing variants)
 *     - Add edge-triggered interrupt configuration and callback-based handler
 *     - Add pull-up/pull-down, output drive strength, open-drain, and polarity inversion configuration
 *     - Add weak PCAL6524_EnterCritical / PCAL6524_ExitCritical hooks wrapped
 *       around read-modify-write sequences; user code may provide a strong
 *       definition to take a mutex and/or disable the relevant interrupt
 *   1.0.0
 *     - Initial version
 */

/*!
 * @addtogroup pcal6524
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief PCAL6524 register memory map.
 *
 * Pin numbering: bits[0:7]   = Port 0 (P0_0~P0_7),
 *                bits[8:15]  = Port 1 (P1_0~P1_7),
 *                bits[16:23] = Port 2 (P2_0~P2_7).
 * For example, pin 10 = P1_2, pin 18 = P2_2.
 */
/*!< Input port (read clears interrupts). */
#define PCAL6524_INPUT_PORT0          (0x00U)
#define PCAL6524_INPUT_PORT1          (0x01U)
#define PCAL6524_INPUT_PORT2          (0x02U)
/*!< Output port. */
#define PCAL6524_OUTPUT_PORT0         (0x04U)
#define PCAL6524_OUTPUT_PORT1         (0x05U)
#define PCAL6524_OUTPUT_PORT2         (0x06U)
/*!< Polarity inversion. */
#define PCAL6524_POLARITY_INV_PORT0   (0x08U)
#define PCAL6524_POLARITY_INV_PORT1   (0x09U)
#define PCAL6524_POLARITY_INV_PORT2   (0x0AU)
/*!< Configuration (direction): 0 = output, 1 = input. */
#define PCAL6524_CONFIG_PORT0         (0x0CU)
#define PCAL6524_CONFIG_PORT1         (0x0DU)
#define PCAL6524_CONFIG_PORT2         (0x0EU)
/*!< Output drive strength (2 bits per pin). */
#define PCAL6524_OUT_DRIVE_PORT0A     (0x40U)
#define PCAL6524_OUT_DRIVE_PORT0B     (0x41U)
#define PCAL6524_OUT_DRIVE_PORT1A     (0x42U)
#define PCAL6524_OUT_DRIVE_PORT1B     (0x43U)
#define PCAL6524_OUT_DRIVE_PORT2A     (0x44U)
#define PCAL6524_OUT_DRIVE_PORT2B     (0x45U)
/*!< Input latch. */
#define PCAL6524_INPUT_LATCH_PORT0    (0x48U)
#define PCAL6524_INPUT_LATCH_PORT1    (0x49U)
#define PCAL6524_INPUT_LATCH_PORT2    (0x4AU)
/*!< Pull-up/pull-down enable. */
#define PCAL6524_PULL_ENABLE_PORT0    (0x4CU)
#define PCAL6524_PULL_ENABLE_PORT1    (0x4DU)
#define PCAL6524_PULL_ENABLE_PORT2    (0x4EU)
/*!< Pull-up/pull-down selection: 1 = pull-up, 0 = pull-down. */
#define PCAL6524_PULL_SELECT_PORT0    (0x50U)
#define PCAL6524_PULL_SELECT_PORT1    (0x51U)
#define PCAL6524_PULL_SELECT_PORT2    (0x52U)
/*!< Interrupt mask: 0 = enabled, 1 = masked. */
#define PCAL6524_INT_MASK_PORT0       (0x54U)
#define PCAL6524_INT_MASK_PORT1       (0x55U)
#define PCAL6524_INT_MASK_PORT2       (0x56U)
/*!< Interrupt status (read-only). */
#define PCAL6524_INT_STATUS_PORT0     (0x58U)
#define PCAL6524_INT_STATUS_PORT1     (0x59U)
#define PCAL6524_INT_STATUS_PORT2     (0x5AU)
/*!< Port-level push-pull/open-drain (single register). */
#define PCAL6524_OUTPUT_PORT_CONFIG   (0x5CU)
/*!< Interrupt edge configuration (2 bits per pin). */
#define PCAL6524_INT_EDGE_PORT0A      (0x60U)
#define PCAL6524_INT_EDGE_PORT0B      (0x61U)
#define PCAL6524_INT_EDGE_PORT1A      (0x62U)
#define PCAL6524_INT_EDGE_PORT1B      (0x63U)
#define PCAL6524_INT_EDGE_PORT2A      (0x64U)
#define PCAL6524_INT_EDGE_PORT2B      (0x65U)
/*!< Interrupt clear (write 1 to clear). */
#define PCAL6524_INT_CLEAR_PORT0      (0x68U)
#define PCAL6524_INT_CLEAR_PORT1      (0x69U)
#define PCAL6524_INT_CLEAR_PORT2      (0x6AU)
/*!< Input status (read does NOT clear interrupts). */
#define PCAL6524_INPUT_STATUS_PORT0   (0x6CU)
#define PCAL6524_INPUT_STATUS_PORT1   (0x6DU)
#define PCAL6524_INPUT_STATUS_PORT2   (0x6EU)
/*!< Per-pin push-pull/open-drain override. */
#define PCAL6524_INDIV_PIN_OUT_PORT0  (0x70U)
#define PCAL6524_INDIV_PIN_OUT_PORT1  (0x71U)
#define PCAL6524_INDIV_PIN_OUT_PORT2  (0x72U)

/*! @brief PCAL6524 total number of I/O pins (P0_0~P0_7, P1_0~P1_7, P2_0~P2_7). */
#define PCAL6524_PIN_COUNT            (24U)

/*! @brief PCAL6524 port masks for @ref PCAL6524_SetOutputPortConfig. */
#define PCAL6524_PORT0_MASK           (0x01U)
#define PCAL6524_PORT1_MASK           (0x02U)
#define PCAL6524_PORT2_MASK           (0x04U)

/*! @brief PCAL6524 IO direction. */
typedef enum _pcal6524_dir
{
    kPCAL6524_Input = 0U, /*!< Set pin as input. */
    kPCAL6524_Output,     /*!< Set pin as output. */
} pcal6524_dir_t;

/*! @brief PCAL6524 interrupt edge type. */
typedef enum _pcal6524_int_edge
{
    kPCAL6524_IntEdgeLevel   = 0x00U, /*!< Level-triggered interrupt. */
    kPCAL6524_IntEdgeRising  = 0x01U, /*!< Rising edge triggered interrupt. */
    kPCAL6524_IntEdgeFalling = 0x02U, /*!< Falling edge triggered interrupt. */
    kPCAL6524_IntEdgeAny     = 0x03U, /*!< Any edge triggered interrupt. */
} pcal6524_int_edge_t;

/*! @brief PCAL6524 output drive strength. */
typedef enum _pcal6524_drive_strength
{
    kPCAL6524_DriveStrength025x = 0x00U, /*!< 0.25x drive capability. */
    kPCAL6524_DriveStrength050x = 0x01U, /*!< 0.5x drive capability. */
    kPCAL6524_DriveStrength075x = 0x02U, /*!< 0.75x drive capability. */
    kPCAL6524_DriveStrength100x = 0x03U, /*!< 1x drive capability (default). */
} pcal6524_drive_strength_t;

/*!
 * @brief Callback dispatch mode. Set to 1 to store a per-pin callback table
 *        in the handle; 0 stores a single global callback.
 *
 * Controlled via Kconfig: set @c CONFIG_PCAL6524_CALLBACK_PER_PIN to enable
 * per-pin mode. Can also be defined directly at build time to override.
 */
#ifndef PCAL6524_CALLBACK_PER_PIN
#ifdef CONFIG_PCAL6524_CALLBACK_PER_PIN
#define PCAL6524_CALLBACK_PER_PIN CONFIG_PCAL6524_CALLBACK_PER_PIN
#else
#define PCAL6524_CALLBACK_PER_PIN 0
#endif
#endif

#if PCAL6524_CALLBACK_PER_PIN
/*!
 * @brief PCAL6524 per-pin interrupt callback function.
 *
 * Invoked by @ref PCAL6524_InterruptHandler for each pin whose bit is set in
 * the Interrupt Status register.
 *
 * @param pin       The pin that triggered the interrupt (0~23).
 * @param pinState  Current level of this pin (true = high, false = low).
 * @param userData  User-defined context for this pin's handler.
 */
typedef void (*pcal6524_pin_callback_t)(uint8_t pin, bool pinState, void *userData);
#else
/*!
 * @brief PCAL6524 interrupt callback function.
 *
 * Called by @ref PCAL6524_InterruptHandler when an interrupt is detected.
 *
 * @param intPins   Bitmask of pins that triggered the interrupt (from Interrupt Status register).
 * @param pinStates Bitmask of current pin levels (from Input Status register, non-destructive).
 * @param userData  User-defined context pointer passed during callback installation.
 */
typedef void (*pcal6524_int_callback_t)(uint32_t intPins, uint32_t pinStates, void *userData);
#endif

/*! @brief PCAL6524 I2C receive function. */
typedef status_t (*pcal6524_i2c_receive_func_t)(void *base,
                                                uint8_t deviceAddress,
                                                uint32_t subAddress,
                                                uint8_t subaddressSize,
                                                uint8_t *rxBuff,
                                                uint8_t rxBuffSize,
                                                uint32_t flags);

/*! @brief PCAL6524 I2C send function. */
typedef status_t (*pcal6524_i2c_send_func_t)(void *base,
                                             uint8_t deviceAddress,
                                             uint32_t subAddress,
                                             uint8_t subaddressSize,
                                             const uint8_t *txBuff,
                                             uint8_t txBuffSize,
                                             uint32_t flags);

/*! @brief PCAL6524 configure structure.*/
typedef struct _pcal6524_config
{
    void *i2cBase;                               /*!< I2C instance base address. */
    uint8_t i2cAddr;                             /*!< I2C address. */
    pcal6524_i2c_send_func_t I2C_SendFunc;       /*!< Function to send I2C data. */
    pcal6524_i2c_receive_func_t I2C_ReceiveFunc; /*!< Function to receive I2C data. */
} pcal6524_config_t;

/*! @brief PCAL6524 driver handle. */
typedef struct _pcal6524_handle
{
    void *i2cBase;                               /*!< I2C instance base address. */
    uint8_t i2cAddr;                             /*!< I2C address. */
    pcal6524_i2c_send_func_t I2C_SendFunc;       /*!< Function to send I2C data. */
    pcal6524_i2c_receive_func_t I2C_ReceiveFunc; /*!< Function to receive I2C data. */
#if PCAL6524_CALLBACK_PER_PIN
    pcal6524_pin_callback_t pinCallbacks[PCAL6524_PIN_COUNT]; /*!< Per-pin callbacks, NULL to skip. */
    void *pinUserData[PCAL6524_PIN_COUNT];                    /*!< Per-pin user context. */
#else
    pcal6524_int_callback_t callback;            /*!< Interrupt callback, NULL if not installed. */
    void *userData;                              /*!< User context passed to callback. */
#endif
} pcal6524_handle_t;

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
 * @brief Initializes the PCAL6524 driver handle.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param config Pointer to the PCAL6524 configuration structure.
 */
void PCAL6524_Init(pcal6524_handle_t *handle, const pcal6524_config_t *config);

#if PCAL6524_CALLBACK_PER_PIN
/*!
 * @brief Install PCAL6524 per-pin interrupt callback.
 *
 * The installed callback is invoked by @ref PCAL6524_InterruptHandler when an
 * interrupt on @p pin is detected.
 *
 * @param handle   Pointer to the PCAL6524 handle.
 * @param pin      The pin number (0~23).
 * @param callback Interrupt callback function, or NULL to uninstall.
 * @param userData User-defined context pointer passed to the callback.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_InstallPinCallback(pcal6524_handle_t *handle, uint8_t pin,
                                     pcal6524_pin_callback_t callback, void *userData);
#else
/*!
 * @brief Install PCAL6524 interrupt callback.
 *
 * The installed callback is invoked by @ref PCAL6524_InterruptHandler when
 * an interrupt event is detected.
 *
 * @param handle   Pointer to the PCAL6524 handle.
 * @param callback Interrupt callback function, or NULL to uninstall.
 * @param userData User-defined context pointer passed to the callback.
 */
void PCAL6524_InstallCallback(pcal6524_handle_t *handle, pcal6524_int_callback_t callback, void *userData);
#endif

/*! @} */

/*!
 * @name GPIO Read/Write
 * @{
 */

/*!
 * @brief Set PCAL6524 pins direction.
 *
 * This function sets multiple pins direction, the pins to modify are passed in
 * as a bit OR'ed value.
 *
 * For example, the following code set pin2 and pin3 to output:
 *
 * @code
   PCAL6524_SetDirection(handle, (1<<2) | (1<<3), kPCAL6524_Output);
   @endcode
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * @param dir Pin direction.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetDirection(pcal6524_handle_t *handle, uint32_t pins, pcal6524_dir_t dir);

/*!
 * @brief Set PCAL6524 pins output value to 1.
 *
 * This function changes multiple pins, the pins to modify are passed in as
 * a bit OR'ed value.
 *
 * For example, the following code set pin2 and pin3 output value to 1.
 *
 * @code
   PCAL6524_SetPins(handle, (1<<2) | (1<<3));
   @endcode
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPins(pcal6524_handle_t *handle, uint32_t pins);

/*!
 * @brief Set PCAL6524 pins output value to 0.
 *
 * This function changes multiple pins, the pins to modify are passed in as
 * a bit OR'ed value.
 *
 * For example, the following code set pin2 and pin3 output value to 0.
 *
 * @code
   PCAL6524_ClearPins(handle, (1<<2) | (1<<3));
   @endcode
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ClearPins(pcal6524_handle_t *handle, uint32_t pins);

/*!
 * @brief Toggle PCAL6524 pins output value.
 *
 * This function changes multiple pins, the pins to modify are passed in as
 * a bit OR'ed value.
 *
 * For example, the following code toggle pin2 and pin3 output value.
 *
 * @code
   PCAL6524_TogglePins(handle, (1<<2) | (1<<3));
   @endcode
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins The pins to toggle, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_TogglePins(pcal6524_handle_t *handle, uint32_t pins);

/*!
 * @brief Read PCAL6524 pins value.
 *
 * Reads the Input Status registers (6Ch~6Eh) which reflect the actual pin
 * state without clearing pending interrupts.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pinsValue Variable to save the read out pin values.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ReadPins(pcal6524_handle_t *handle, uint32_t *pinsValue);

/*!
 * @brief Read PCAL6524 pins value and clear all interrupts.
 *
 * Reads the Input Port registers (00h~02h). This returns pin values and clears
 * all pending interrupts as a side effect. Use this for power-on initialization
 * or when you intentionally want to acknowledge all interrupts at once.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pinsValue Variable to save the read out pin values.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ReadPinsAndClearInterrupt(pcal6524_handle_t *handle, uint32_t *pinsValue);

/*!
 * @brief Inverse PCAL6524 pins input polarity.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins Bitmask of pins to change.
 * @param inverse Use true to inverse, false to not inverse.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_InversePolarity(pcal6524_handle_t *handle, uint32_t pins, bool inverse);

/*! @} */

/*!
 * @name Pin Configuration
 * @{
 */

/*!
 * @brief Set PCAL6524 individual pin output configuration (push-pull or open-drain).
 *
 * Uses the individual pin output configuration registers (70h~72h) which override
 * the port-level setting in register 5Ch on a per-pin basis.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins Bitmask of pins to change.
 * @param openDrain Use true for open-drain, false for push-pull.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPinOutputConfig(pcal6524_handle_t *handle, uint32_t pins, bool openDrain);

/*!
 * @brief Set PCAL6524 output port configuration (push-pull or open-drain).
 *
 * Configures the output stage at port granularity. For per-pin control, use
 * @ref PCAL6524_SetPinOutputConfig instead.
 *
 * @code
   PCAL6524_SetOutputPortConfig(handle, PCAL6524_PORT0_MASK | PCAL6524_PORT1_MASK, true);
   @endcode
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param portMask The ports to change (OR of PCAL6524_PORTx_MASK values).
 * @param openDrain Use true for open-drain, false for push-pull.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetOutputPortConfig(pcal6524_handle_t *handle, uint8_t portMask, bool openDrain);

/*!
 * @brief Set PCAL6524 output drive strength for a specific pin.
 *
 * Each pin can be independently configured to one of four drive strength levels.
 * Reducing drive strength can lower simultaneous switching noise (SSN).
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pin The pin number (0~23). Pins 0~7 are port 0, 8~15 are port 1, 16~23 are port 2.
 * @param strength The drive strength level.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetOutputDriveStrength(pcal6524_handle_t *handle, uint8_t pin, pcal6524_drive_strength_t strength);

/*!
 * @brief Enable or disable PCAL6524 pull-up/pull-down resistors.
 *
 * Use @ref PCAL6524_SetPullSelection to choose between pull-up and pull-down.
 * Selection should be set before enabling.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins Bitmask of pins to change.
 * @param enable Use true to enable pull resistor, false to disable.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPullEnable(pcal6524_handle_t *handle, uint32_t pins, bool enable);

/*!
 * @brief Select PCAL6524 pull-up or pull-down resistor.
 *
 * The resistor must also be enabled via @ref PCAL6524_SetPullEnable.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins Bitmask of pins to change.
 * @param pullUp Use true to select 100 kohm pull-up, false for pull-down.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPullSelection(pcal6524_handle_t *handle, uint32_t pins, bool pullUp);

/*!
 * @brief Set PCAL6524 input latch.
 *
 * When latched, the input pin state that triggered an interrupt is held until
 * the input port register is read. This prevents missing fast transients.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins Bitmask of pins to change.
 * @param enable Use true to enable latch, false to disable latch.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetInputLatch(pcal6524_handle_t *handle, uint32_t pins, bool enable);

/*! @} */

/*!
 * @name Interrupt
 * @{
 */

/*!
 * @brief Configure PCAL6524 pin interrupt in one call.
 *
 * Performs disable -> configure -> enable so that no spurious interrupt can
 * fire while the edge or latch is being changed.
 *
 * To disable interrupt on a pin, pass @ref kPCAL6524_IntEdgeLevel as edge type.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pin The pin number (0~23).
 * @param edge The interrupt edge type. Use kPCAL6524_IntEdgeLevel to disable.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetPinInterruptConfig(pcal6524_handle_t *handle, uint8_t pin, pcal6524_int_edge_t edge);

/*!
 * @brief Enable PCAL6524 interrupt for specific pins.
 *
 * Clears the interrupt mask bits (hardware register bit=0 enables interrupt).
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins Bitmask of pins to enable interrupt on.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_EnableInterrupts(pcal6524_handle_t *handle, uint32_t pins);

/*!
 * @brief Disable PCAL6524 interrupt for specific pins.
 *
 * Sets the interrupt mask bits (hardware register bit=1 disables interrupt).
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins Bitmask of pins to disable interrupt on.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_DisableInterrupts(pcal6524_handle_t *handle, uint32_t pins);

/*!
 * @brief Set PCAL6524 interrupt edge type for a specific pin.
 *
 * Each pin uses 2 bits in the interrupt edge registers (60h~65h).
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pin The pin number (0~23).
 * @param edge The interrupt edge type.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetInterruptEdge(pcal6524_handle_t *handle, uint8_t pin, pcal6524_int_edge_t edge);

/*!
 * @brief Get PCAL6524 interrupt status.
 *
 * Reads the interrupt status registers (58h~5Ah). A bit set to 1 indicates
 * the corresponding pin was the source of an interrupt.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pinsStatus Variable to save the interrupt status.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_GetInterruptStatus(pcal6524_handle_t *handle, uint32_t *pinsStatus);

/*!
 * @brief Clear PCAL6524 interrupt for specific pins.
 *
 * Writes to the interrupt clear registers (68h~6Ah). Writing a 1 to a bit
 * clears that pin's interrupt source without affecting other pins.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins Bitmask of pins to clear interrupt.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ClearInterruptPins(pcal6524_handle_t *handle, uint32_t pins);

/*!
 * @brief PCAL6524 interrupt handler.
 *
 * Call this from task context when the MCU GPIO ISR detects a falling edge on
 * the PCAL6524 INT pin. It loops until the chip's Interrupt Status register
 * is empty, performing each iteration:
 *   1. Read Interrupt Status (58h) to identify source pins.
 *   2. Read Input Status (6Ch) to get current pin levels (non-destructive).
 *   3. Invoke the user callback(s) for pins in this snapshot.
 *   4. Clear the handled interrupt sources via Interrupt Clear (68h).
 *
 * The loop is required because an edge-triggered MCU GPIO ISR cannot retrigger
 * while the PCAL6524 INT pin stays asserted, so any interrupt that arrives
 * during callback execution must be drained within this call.
 *
 * @note This function performs I2C transactions and must NOT be called from
 * ISR context. Use a flag or deferred work mechanism to call from task context.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_InterruptHandler(pcal6524_handle_t *handle);

/*! @} */

/*!
 * @name Critical-section hooks
 *
 * The driver wraps every internal read-modify-write sequence with these
 * functions. Both have weak no-op default implementations in fsl_pcal6524.c.
 * To serialize driver state across multiple RTOS tasks, or to protect against
 * ISR re-entry, override either or both by providing a strong definition with
 * the same signature in user code (linker resolves strong over weak).
 *
 * The driver invokes them in a non-nested fashion on a given handle, and runs
 * user callbacks from @ref PCAL6524_InterruptHandler with the critical
 * section released so callbacks may call other driver APIs.
 *
 * Example: serialize across RTOS tasks with an OSA mutex
 * @code
   void PCAL6524_EnterCritical(pcal6524_handle_t *handle)
   {
       (void)handle;
       (void)OSA_MutexLock(g_expanderMutex, osaWaitForever_c);
   }
   void PCAL6524_ExitCritical(pcal6524_handle_t *handle)
   {
       (void)handle;
       (void)OSA_MutexUnlock(g_expanderMutex);
   }
   @endcode
 *
 * Example: also block ISR re-entry by masking the MCU GPIO IRQ that is wired
 * to the expander INT pin (mixed task / ISR access)
 * @code
   void PCAL6524_EnterCritical(pcal6524_handle_t *handle)
   {
       (void)handle;
       DisableIRQ(BOARD_EXPANDER_INT_IRQ);
       (void)OSA_MutexLock(g_expanderMutex, osaWaitForever_c);
   }
   void PCAL6524_ExitCritical(pcal6524_handle_t *handle)
   {
       (void)handle;
       (void)OSA_MutexUnlock(g_expanderMutex);
       EnableIRQ(BOARD_EXPANDER_INT_IRQ);
   }
   @endcode
 *
 * @{
 */
void PCAL6524_EnterCritical(pcal6524_handle_t *handle);
void PCAL6524_ExitCritical(pcal6524_handle_t *handle);
/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* _FSL_PCAL6524_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
