/*
 * Copyright 2022 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SMBUS_H_
#define _FSL_SMBUS_H_

/*!
 * @addtogroup SMBUS_Adapter
 * @{
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"
#include "fsl_smbus_port.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief smbus bus protocol */
typedef enum _smbus_bus_protocol
{
    kSMBUS_WriteByte           = 0x11U, /*!< write byte */
    kSMBUS_ReadByte            = 0x12U, /*!< read byte */
    kSMBUS_WriteWord           = 0x13U, /*!< write word */
    kSMBUS_ReadWord            = 0x14U, /*!< read word */
    kSMBUS_Write32             = 0x15U, /*!< write 32bits */
    kSMBUS_Read32              = 0x16U, /*!< read 32bits */
    kSMBUS_Write64             = 0x17U, /*!< write 64bits */
    kSMBUS_Read64              = 0x18U, /*!< read 64bits */
    kSMBUS_WriteBlock          = 0x19U, /*!< write block */
    kSMBUS_ReadBlock           = 0x1AU, /*!< read block */
    kSMBUS_ProcessCall         = 0x1BU, /*!< process call */
    kSMBUS_BlockWriteBlockRead = 0x1CU, /*!< block write - block read process call*/
} smbus_bus_protocol_t;

/*@brief smbus master transfer */
typedef struct _smbus_master_transfer
{
    uint8_t slaveAddress;       /*!< slave address shall be set when master read/write */
    const uint8_t *writeBuffer; /*!< write buffer address  */
    uint8_t *readBuffer;        /*!< read buffer address */
    uint32_t transferByteSize;  /*!< buffer byte size */
    bool needPEC;               /*!< true will amend PEC, otherwise no PEC */
} smbus_master_transfer_t;

/*@brief smbus master config */
typedef struct _smbus_master_config
{
    uint32_t baudRate_Bps;          /*!< smbus baudrate */
    uint32_t timeoutMS;             /*!< smbus transfer timeout */
    smbus_port_config_t portConfig; /*!< smbus port config */
} smbus_master_config_t;

/*@brief smbus master handle */
typedef struct _smbus_master_handle
{
    smbus_master_port_handle_t port; /*!< smbus port handler */
    uint8_t packetBuffer[256U];      /*!< smbus packet buffer for internal use */
    uint8_t crcBuffer[257U];         /*!< smbus crc buffer for internal use */
    uint32_t timeoutMS;              /*!< smbus transfer timeout */
} smbus_master_handle_t;

/*@brief smbus slave transfer */
typedef struct _smbus_slave_transfer
{
    const uint8_t *writeBuffer; /*!< write buffer address  */
    uint8_t *readBuffer;        /*!< read buffer address */
    uint32_t transferByteSize;  /*!< buffer byte size */
    bool needPEC;               /*!< true will amend PEC, otherwise no PEC */
} smbus_slave_transfer_t;

/*@brief smbus slave config */
typedef struct _smbus_slave_config
{
    uint8_t slaveAddress;           /*!< slave address shall be set when act as slave */
    smbus_port_config_t portConfig; /*!< smbus port config */
    uint32_t timeoutMS;             /*!< smbus transfer timeout */
} smbus_slave_config_t;

/*@brief smbus slave handle */
typedef struct _smbus_slave_handle
{
    smbus_slave_port_handle_t port; /*!< smbus port handler */
    uint8_t slaveAddress;           /*!< slave address */
    uint8_t packetBuffer[256U];     /*!< smbus packet buffer for internal use */
    uint8_t crcBuffer[257U];        /*!< smbus crc buffer for internal use */
    uint32_t timeoutMS;             /*!< smbus transfer timeout */
} smbus_slave_handle_t;

#if defined(__cplusplus)
extern "C" {
#endif
/*******************************************************************************
 * APIs
 ******************************************************************************/
/*!
 * @brief SMBUS master init.
 * @param handle smbus handler
 * @param config smbus configuration pointer
 *
 */
void SMBUS_Master_Init(smbus_master_handle_t *handle, smbus_master_config_t *config);

/*!
 * @brief SMBUS master deinit.
 * @param handle smbus handler
 *
 */
void SMBUS_Master_Deinit(smbus_master_handle_t *handle);

/*!
 * @brief SMBUS master send byte.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Master_WriteByte(smbus_master_handle_t *handle, smbus_master_transfer_t *transfer);

/*!
 * @brief SMBUS master receive byte.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Master_ReadByte(smbus_master_handle_t *handle, smbus_master_transfer_t *transfer);

/*!
 * @brief SMBUS master write.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 * @param protocol smbus transfer protocol
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Master_Write(smbus_master_handle_t *handle,
                            smbus_master_transfer_t *transfer,
                            smbus_bus_protocol_t protocol);

/*!
 * @brief SMBUS master read.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 * @param protocol smbus transfer protocol
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Master_Read(smbus_master_handle_t *handle,
                           smbus_master_transfer_t *transfer,
                           smbus_bus_protocol_t protocol);

/*!
 * @brief SMBUS master process call.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 * @param protocol smbus transfer protocol
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Master_ProcessCall(smbus_master_handle_t *handle,
                                  smbus_master_transfer_t *transfer,
                                  smbus_bus_protocol_t protocol);

/*!
 * @brief SMBUS master send byte.
 * @param handle smbus handler
 * @param deviceID smbus master deviceID
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Master_GetDeviceID(smbus_master_handle_t *handle, uint8_t *deviceID);

/*!
 * @brief SMBUS slave init.
 * @param handle smbus handler
 * @param config smbus configuration pointer
 *
 */
void SMBUS_Slave_Init(smbus_slave_handle_t *handle, smbus_slave_config_t *config);

/*!
 * @brief SMBUS slave deinit.
 * @param handle smbus handler
 *
 */
void SMBUS_Slave_Deinit(smbus_slave_handle_t *handle);

/*!
 * @brief SMBUS slave send byte.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Slave_WriteByte(smbus_slave_handle_t *handle, smbus_slave_transfer_t *transfer);

/*!
 * @brief SMBUS slave receive byte.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Slave_ReadByte(smbus_slave_handle_t *handle, smbus_slave_transfer_t *transfer);

/*!
 * @brief SMBUS slave write.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 * @param protocol smbus transfer protocol
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Slave_Write(smbus_slave_handle_t *handle,
                           smbus_slave_transfer_t *transfer,
                           smbus_bus_protocol_t protocol);

/*!
 * @brief SMBUS slave read.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 * @param protocol smbus transfer protocol
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Slave_Read(smbus_slave_handle_t *handle,
                          smbus_slave_transfer_t *transfer,
                          smbus_bus_protocol_t protocol);

/*!
 * @brief SMBUS slave process call.
 * @param handle smbus handler
 * @param transfer smbus transfer configuration pointer
 * @param protocol smbus transfer protocol
 *
 * @retval kStatus_Success if no error, otherwise failed.
 */
status_t SMBUS_Slave_ProcessCall(smbus_slave_handle_t *handle,
                                 smbus_slave_transfer_t *transfer,
                                 smbus_bus_protocol_t protocol);

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */
#endif /* _FSL_SMBUS_H_ */
