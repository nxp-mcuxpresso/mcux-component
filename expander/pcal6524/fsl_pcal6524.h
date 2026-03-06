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
 *     - Add PCAL6524 I/O expander functions and register definitions
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

/*! @brief Define the Register Memory Map of PCAL6524. */
#define PCAL6524_INPUT_PORT0          (0x00U)
#define PCAL6524_INPUT_PORT1          (0x01U)
#define PCAL6524_INPUT_PORT2          (0x02U)
#define PCAL6524_OUTPUT_PORT0         (0x04U)
#define PCAL6524_OUTPUT_PORT1         (0x05U)
#define PCAL6524_OUTPUT_PORT2         (0x06U)
#define PCAL6524_POLARITY_INV_PORT0   (0x08U)
#define PCAL6524_POLARITY_INV_PORT1   (0x09U)
#define PCAL6524_POLARITY_INV_PORT2   (0x0AU)
#define PCAL6524_CONFIG_PORT0         (0x0CU)
#define PCAL6524_CONFIG_PORT1         (0x0DU)
#define PCAL6524_CONFIG_PORT2         (0x0EU)
#define PCAL6524_INPUT_LATCH_PORT0    (0x48U)
#define PCAL6524_INPUT_LATCH_PORT1    (0x49U)
#define PCAL6524_INPUT_LATCH_PORT2    (0x4AU)
#define PCAL6524_INT_MASK_PORT0       (0x54U)
#define PCAL6524_INT_MASK_PORT1       (0x55U)
#define PCAL6524_INT_MASK_PORT2       (0x56U)
#define PCAL6524_INT_STATUS_PORT0     (0x58U)
#define PCAL6524_INT_STATUS_PORT1     (0x59U)
#define PCAL6524_INT_STATUS_PORT2     (0x5AU)

/*! @brief PCAL6524 IO direction. */
typedef enum _pcal6524_dir
{
    kPCAL6524_Input = 0U, /*!< Set pin as input. */
    kPCAL6524_Output,     /*!< Set pin as output. */
} pcal6524_dir_t;

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
} pcal6524_handle_t;

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
 * @brief Initializes the PCAL6524 driver handle.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param config Pointer to the PCAL6524 configuration structure.
 */
void PCAL6524_Init(pcal6524_handle_t *handle, const pcal6524_config_t *config);

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
 * @param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_TogglePins(pcal6524_handle_t *handle, uint32_t pins);

/*!
 * @brief Inverse PCAL6524 pins polarity.
 *
 * This function changes multiple pins polarity, the pins to modify are passed in
 * as a bit OR'ed value.
 *
 * For example, the following code set pin2 and pin3 to NOT inverse:
 *
 * @code
   PCAL6524_InversePolarity(handle, (1<<2) | (1<<3), false);
   @endcode
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * @param inverse Use true to inverse, false to not inverse.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_InversePolarity(pcal6524_handle_t *handle, uint32_t pins, bool inverse);

/*!
 * @brief Read PCAL6524 pins value.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pinsValue Variable to save the read out pin values.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ReadPins(pcal6524_handle_t *handle, uint32_t *pinsValue);

/*!
 * @brief Set PCAL6524 interrupt mask.
 *
 * This function configures which pins can generate interrupts.
 * In the interrupt mask register, a bit value of 0 enables interrupt, a bit value of 1 disables interrupt.
 *
 * For example, the following code enables interrupt on pin2 and pin3:
 *
 * @code
   PCAL6524_SetInterruptMask(handle, (1<<2) | (1<<3), true);
   @endcode
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * @param enable Use true to enable interrupt, false to disable interrupt.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetInterruptMask(pcal6524_handle_t *handle, uint32_t pins, bool enable);

/*!
 * @brief Get PCAL6524 interrupt status.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pinsStatus Variable to save the interrupt status.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_GetInterruptStatus(pcal6524_handle_t *handle, uint32_t *pinsStatus);

/*!
 * @brief Clear PCAL6524 interrupt.
 *
 * This function clears the interrupt by reading the input port registers.
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pinsValue Variable to save the read out pin values.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_ClearInterrupt(pcal6524_handle_t *handle, uint32_t *pinsValue);

/*!
 * @brief Set PCAL6524 input latch.
 *
 * This function configures whether input ports are latched or not.
 *
 * For example, the following code enables latch on pin2 and pin3:
 *
 * @code
   PCAL6524_SetInputLatch(handle, (1<<2) | (1<<3), true);
   @endcode
 *
 * @param handle Pointer to the PCAL6524 handle.
 * @param pins The pins to change, for example: (1<<2) | (1<<3) means pin 2 and pin 3.
 * @param enable Use true to enable latch, false to disable latch.
 * @return Return @ref kStatus_Success if successed, otherwise returns error code.
 */
status_t PCAL6524_SetInputLatch(pcal6524_handle_t *handle, uint32_t pins, bool enable);

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* _FSL_PCAL6524_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
