/*
 * Copyright 2022 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SMBUS_PORT_H_
#define _FSL_SMBUS_PORT_H_

/*!
 * @addtogroup SMBUS_Adapter
 * @{
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"
#include "fsl_i2c.h"
#include "fsl_i2c_gpdma.h"
#include "fsl_smbus_event.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*@brief smbus transfer flag */
enum _smbus_port_transfer_flag
{
    kSMBUS_PORT_DefaultFlag         = kI2C_TransferDefaultFlag,
    kSMBUS_PORT_ReadRepeatStartFlag = kI2C_TransferRepeatedStartFlag,
    kSMBUS_PORT_WriteNoStopFlag     = kI2C_TransferNoStopFlag,
};

/*@brief smbus port config */
typedef struct _smbus_port_config
{
    void *i2cBase;        /*!< i2c base */
    uint32_t srcClock_Hz; /*!< i2c source clock frequency */

    void *dmaBase;        /*!< i2c dma base */
    uint32_t dmaChannel;  /*!< dma channel for i2c */
} smbus_port_config_t;

/*@brief smbus port handle */
typedef struct _smbus_master_port_handle
{
    gpdma_handle_t gpdmaHandle;
    void *i2cBase; /*!< i2c base */
    i2c_master_gpdma_handle_t i2cMasterDmaHandle;
    smbus_event_t event;
} smbus_master_port_handle_t;

/*@brief smbus port handle */
typedef struct _smbus_slave_port_handle
{
    gpdma_handle_t gpdmaHandle;
    i2c_slave_gpdma_handle_t i2cSlaveDmaHandle;
    void *i2cBase; /*!< i2c base */
    smbus_event_t event;

    uint8_t *data;
    uint32_t dataSize;
} smbus_slave_port_handle_t;

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * APIs
 ******************************************************************************/
/*!
 * @brief SMBUS master port init.
 * @param handle smbus port handler
 * @param baudRate_Bps smbus port baudrate
 * @param config smbus port configuration pointer
 *
 */
void SMBUS_Master_Port_Init(smbus_master_port_handle_t *handle,
                            uint32_t baudRate_Bps,
                            const smbus_port_config_t *config);

/*!
 * @brief SMBUS master port deinit.
 * @param handle smbus port handler
 */
void SMBUS_Master_Port_Deinit(smbus_master_port_handle_t *handle);

/*!
 * @brief SMBUS master port write.
 * @param handle smbus port handler
 * @param slaveAddress smbus port baudrate
 * @param buffer buffer address contain the data to write
 * @param writeSize data size to write
 * @param timeoutMS transfer timeout
 * @param flag smbus port transfer flag
 *
 */
status_t SMBUS_Master_Port_Write(smbus_master_port_handle_t *handle,
                                 uint8_t slaveAddress,
                                 uint8_t *buffer,
                                 uint32_t writeSize,
                                 uint32_t timeoutMS,
                                 uint32_t flag);

/*!
 * @brief SMBUS master port read.
 * @param handle smbus port handler
 * @param slaveAddress smbus port baudrate
 * @param buffer buffer address contain the data to write
 * @param readSize data size to read
 * @param timeoutMS transfer timeout
 * @param flag smbus port transfer flag
 *
 */
status_t SMBUS_Master_Port_Read(smbus_master_port_handle_t *handle,
                                uint8_t slaveAddress,
                                uint8_t *buffer,
                                uint32_t readSize,
                                uint32_t timeoutMS,
                                uint32_t flag);

/*!
 * @brief SMBUS slave port init.
 * @param handle smbus port handler
 * @param config smbus port configuration pointer
 * @param slaveAddress smbus slave address
 *
 */
void SMBUS_Slave_Port_Init(smbus_slave_port_handle_t *handle, const smbus_port_config_t *config, uint8_t slaveAddress);

/*!
 * @brief SMBUS slave port deinit.
 * @param handle smbus port handler
 */
void SMBUS_Slave_Port_Deinit(smbus_slave_port_handle_t *handle);

/*!
 * @brief SMBUS slave port write.
 * @param handle smbus port handler
 * @param buffer buffer address contain the data to write
 * @param writeSize data size to write
 * @param timeoutMS transfer timeout
 *
 */
status_t SMBUS_Slave_Port_Write(smbus_slave_port_handle_t *handle,
                                uint8_t *buffer,
                                uint32_t writeSize,
                                uint32_t timeoutMS);

/*!
 * @brief SMBUS slave port read.
 * @param handle smbus port handler
 * @param buffer buffer address contain the data to write
 * @param readSize data size to read
 * @param timeoutMS transfer timeout
 *
 */
status_t SMBUS_Slave_Port_Read(smbus_slave_port_handle_t *handle,
                               uint8_t *buffer,
                               uint32_t readSize,
                               uint32_t timeoutMS);
/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */
#endif /* FSL_SMBUS_H_ */
