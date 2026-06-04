/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FSL_XSPI_NAND_FLASH_H_
#define FSL_XSPI_NAND_FLASH_H_

#include "fsl_xspi.h"

/*!
 * @addtogroup xspi_nand_flash
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Total LUT slot count. */
#define NAND_XSPI_LUT_SLOT_COUNT 9U
/*! @brief LUT words per sequence. */
#define NAND_XSPI_LUT_WORDS_PER_SEQ 5U

/*!@brief XSPI NAND memory config. */
typedef struct _xspi_mem_nand_config
{
    xspi_device_config_t deviceConfig;                                           /*!< XSPI device configuration. */
    xspi_target_group_t targetGroup;                                             /*!< Target group. */
    uint32_t lookupTable[NAND_XSPI_LUT_SLOT_COUNT][NAND_XSPI_LUT_WORDS_PER_SEQ]; /*!< LUT Table */
    uint32_t ambaBase;                                                           /*!< XSPI AHB base */
} xspi_mem_nand_config_t;

/*! @} */

/*!
 * @brief Read JEDEC ID from the SPI NAND device.
 *
 * @param handle   Initialized nand_handle_t.
 * @param mfr      Output: manufacturer ID byte.
 * @param deviceId Output: 16-bit device ID.
 * @return         kStatus_Success on success, else a transfer error (IDs are 0 on failure).
 */
status_t Nand_Flash_ReadJedecId(nand_handle_t *handle, uint8_t *mfr, uint16_t *deviceId);

/*!
 * @brief Read an SPI NAND feature/status register via GET FEATURE (0x0F).
 *
 * @param handle       Initialized nand_handle_t.
 * @param featureAddr  Register address: 0xA0=SR1(Protection), 0xB0=SR2(Config), 0xC0=SR3(Status).
 * @param value        Output: register value (lower 8 bits).
 * @return             kStatus_Success on success, else a transfer error.
 */
status_t Nand_Flash_GetFeature(nand_handle_t *handle, uint8_t featureAddr, uint8_t *value);

/*!
 * @brief Write an SPI NAND feature/status register via SET FEATURE (0x1F).
 *
 * @param handle       Initialized nand_handle_t.
 * @param featureAddr  Register address: 0xA0=SR1(Protection), 0xB0=SR2(Config).
 * @param value        Value to write (lower 8 bits).
 * @return             kStatus_Success on success, else a transfer error.
 */
status_t Nand_Flash_SetFeature(nand_handle_t *handle, uint8_t featureAddr, uint8_t value);

#endif /* FSL_XSPI_NAND_FLASH_H_ */
