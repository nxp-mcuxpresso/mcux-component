/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_smbus.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SMBUS_CMD_GET_DEVICE_ID             (0x03U)
#define SMBUS_CMD_GET_DEVICE_ID_PACKET_SIZE (19U)

#define SMBUS_DEVICE_DEFAULT_ADDRESS (0x61U)
#define SMBUS_DEVICE_UDID_SIZE       (16U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static uint8_t SMBUS_CRC8(const uint8_t *data, uint8_t length);
/*******************************************************************************
 * Code
 ******************************************************************************/
static uint8_t SMBUS_CRC8(const uint8_t *data, uint8_t length)
{
    unsigned crc = 0;
    int i, j;
    for (j = length; j; j--, data++)
    {
        crc ^= (*data << 8);
        for (i = 8; i; i--)
        {
            if (crc & 0x8000)
                crc ^= (0x1070 << 3);
            crc <<= 1;
        }
    }
    return (uint8_t)(crc >> 8);
}

void SMBUS_Master_Init(smbus_master_handle_t *handle, smbus_master_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);

    handle->timeoutMS = config->timeoutMS;
    SMBUS_Master_Port_Init(&handle->port, config->baudRate_Bps, &config->portConfig);
}

void SMBUS_Master_Deinit(smbus_master_handle_t *handle)
{
    SMBUS_Master_Port_Deinit(&handle->port);
}

status_t SMBUS_Master_WriteByte(smbus_master_handle_t *handle, smbus_master_transfer_t *transfer)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal    = kStatus_Success;
    uint32_t writeSize = 1U;

    handle->packetBuffer[0] = transfer->writeBuffer[0U];
    if (transfer->needPEC)
    {
        handle->crcBuffer[0]    = transfer->slaveAddress << 1;
        handle->crcBuffer[1]    = transfer->writeBuffer[0U];
        handle->packetBuffer[1] = SMBUS_CRC8(handle->crcBuffer, 2);
        writeSize++;
    }

    /* Send Data to slave */
    retVal = SMBUS_Master_Port_Write(&handle->port, transfer->slaveAddress, handle->packetBuffer, writeSize,
                                     handle->timeoutMS, kSMBUS_PORT_DefaultFlag);

    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }

    return retVal;
}

status_t SMBUS_Master_ReadByte(smbus_master_handle_t *handle, smbus_master_transfer_t *transfer)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal   = kStatus_Success;
    uint32_t readSize = 1U;
    uint8_t tempCRC   = 0U;

    if (transfer->needPEC)
    {
        readSize++;
    }

    /* Read Data From Slave*/
    retVal = SMBUS_Master_Port_Read(&handle->port, transfer->slaveAddress, handle->packetBuffer, readSize,
                                    handle->timeoutMS, kSMBUS_PORT_DefaultFlag);
    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }
    else
    {
        if (transfer->needPEC)
        {
            handle->crcBuffer[0] = (transfer->slaveAddress << 1U) | 0x01U;
            handle->crcBuffer[1] = handle->packetBuffer[0];
            tempCRC              = SMBUS_CRC8(handle->crcBuffer, 2);
            if (tempCRC != handle->packetBuffer[1])
            {
                retVal = kStatus_Fail;
            }
        }

        if (retVal == kStatus_Success)
        {
            transfer->readBuffer[0U] = handle->packetBuffer[0];
        }
    }

    return retVal;
}

status_t SMBUS_Master_Write(smbus_master_handle_t *handle,
                            smbus_master_transfer_t *transfer,
                            smbus_bus_protocol_t protocol)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal    = kStatus_Success;
    uint32_t writeSize = 0U;

    switch (protocol)
    {
        case kSMBUS_WriteByte:
            handle->packetBuffer[0U] = (uint32_t)protocol;
            handle->packetBuffer[1U] = transfer->writeBuffer[0];
            writeSize                = 2U;
            break;
        case kSMBUS_WriteWord:
        case kSMBUS_Write32:
            handle->packetBuffer[0U] = (uint32_t)protocol;
            handle->packetBuffer[1U] = transfer->writeBuffer[0];
            handle->packetBuffer[2U] = transfer->writeBuffer[1];
            handle->packetBuffer[3U] = transfer->writeBuffer[2];
            handle->packetBuffer[4U] = transfer->writeBuffer[3];
            writeSize                = 5U;
            break;
        case kSMBUS_Write64:
            handle->packetBuffer[0U] = (uint32_t)protocol;
            handle->packetBuffer[1U] = transfer->writeBuffer[0];
            handle->packetBuffer[2U] = transfer->writeBuffer[1];
            handle->packetBuffer[3U] = transfer->writeBuffer[2];
            handle->packetBuffer[4U] = transfer->writeBuffer[3];
            handle->packetBuffer[5U] = transfer->writeBuffer[4];
            handle->packetBuffer[6U] = transfer->writeBuffer[5];
            handle->packetBuffer[7U] = transfer->writeBuffer[6];
            handle->packetBuffer[8U] = transfer->writeBuffer[7];
            writeSize                = 9U;
            break;
        case kSMBUS_WriteBlock:
            handle->packetBuffer[0U] = (uint32_t)protocol;
            handle->packetBuffer[1U] = transfer->transferByteSize;
            for (uint8_t i = 0U; i < transfer->transferByteSize; i++)
            {
                handle->packetBuffer[i + 2U] = transfer->writeBuffer[i];
            }
            writeSize = transfer->transferByteSize + 2U;
            break;
        default:
            retVal = kStatus_InvalidArgument;
            break;
    }

    if (kStatus_Success == retVal)
    {
        if (transfer->needPEC)
        {
            handle->crcBuffer[0] = transfer->slaveAddress << 1U;
            for (uint8_t i = 1U; i <= writeSize; i++)
            {
                handle->crcBuffer[i] = handle->packetBuffer[i - 1U];
            }
            handle->packetBuffer[writeSize] = SMBUS_CRC8(handle->crcBuffer, writeSize + 1);
            writeSize++;
        }

        retVal = SMBUS_Master_Port_Write(&handle->port, transfer->slaveAddress, handle->packetBuffer, writeSize,
                                         handle->timeoutMS, kSMBUS_PORT_DefaultFlag);

        if (retVal != kStatus_Success)
        {
            retVal = kStatus_Fail;
        }
    }

    return retVal;
}

status_t SMBUS_Master_Read(smbus_master_handle_t *handle,
                           smbus_master_transfer_t *transfer,
                           smbus_bus_protocol_t protocol)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal   = kStatus_Success;
    uint32_t readSize = 0U;
    uint8_t tempCRC   = 0U;

    switch (protocol)
    {
        case kSMBUS_ReadByte:
            handle->packetBuffer[0U] = (uint32_t)protocol;
            readSize                 = 1U;
            break;
        case kSMBUS_ReadWord:
        case kSMBUS_Read32:
            handle->packetBuffer[0U] = (uint32_t)protocol;
            readSize                 = 4U;
            break;
        case kSMBUS_Read64:
            handle->packetBuffer[0U] = (uint32_t)protocol;
            readSize                 = 8U;
            break;
        case kSMBUS_ReadBlock:
            handle->packetBuffer[0U] = (uint32_t)protocol;
            readSize                 = transfer->transferByteSize + 1U;
            break;
        default:
            retVal = kStatus_InvalidArgument;
            break;
    }

    if (kStatus_Success == retVal)
    {
        retVal = SMBUS_Master_Port_Write(&handle->port, transfer->slaveAddress, handle->packetBuffer, 1U,
                                         handle->timeoutMS, kSMBUS_PORT_WriteNoStopFlag);

        if (retVal != kStatus_Success)
        {
            retVal = kStatus_Fail;
        }
        else
        {
            if (transfer->needPEC)
            {
                readSize++;
            }

            /* Send master non-blocking data to slave */
            retVal = SMBUS_Master_Port_Read(&handle->port, transfer->slaveAddress, handle->packetBuffer, readSize,
                                            handle->timeoutMS, kSMBUS_PORT_DefaultFlag);
            if (retVal != kStatus_Success)
            {
                retVal = kStatus_Fail;
            }
            else
            {
                if (transfer->needPEC)
                {
                    handle->crcBuffer[0U] = (transfer->slaveAddress << 1) | 0x01U;
                    for (uint8_t i = 1U; i < readSize; i++)
                    {
                        handle->crcBuffer[i] = handle->packetBuffer[i - 1U];
                    }
                    tempCRC = SMBUS_CRC8(handle->crcBuffer, readSize);
                    if (tempCRC != handle->packetBuffer[readSize - 1U])
                    {
                        retVal = kStatus_Fail;
                    }
                }

                if (retVal == kStatus_Success)
                {
                    if (protocol == kSMBUS_ReadBlock)
                    {
                        memcpy(transfer->readBuffer, &handle->packetBuffer[1U], transfer->transferByteSize);
                    }
                    else
                    {
                        memcpy(transfer->readBuffer, &handle->packetBuffer[0U], transfer->transferByteSize);
                    }
                }
            }
        }
    }

    return retVal;
}

status_t SMBUS_Master_ProcessCall(smbus_master_handle_t *handle,
                                  smbus_master_transfer_t *transfer,
                                  smbus_bus_protocol_t protocol)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal   = kStatus_Success;
    uint32_t tempSize = 0U;
    uint8_t tempCRC   = 0U;

    if (protocol == kSMBUS_BlockWriteBlockRead)
    {
        handle->packetBuffer[0] = (uint8_t)kSMBUS_BlockWriteBlockRead;
        handle->packetBuffer[1] = transfer->transferByteSize;
        for (uint8_t i = 0; i < transfer->transferByteSize; i++)
        {
            handle->packetBuffer[i + 2U] = transfer->writeBuffer[i];
        }
        tempSize = transfer->transferByteSize + 2U;
    }
    else if (protocol == kSMBUS_ProcessCall)
    {
        handle->packetBuffer[0] = (uint8_t)kSMBUS_ProcessCall;
        handle->packetBuffer[1] = transfer->writeBuffer[0];
        handle->packetBuffer[2] = transfer->writeBuffer[1];
        handle->packetBuffer[3] = transfer->writeBuffer[2];
        handle->packetBuffer[4] = transfer->writeBuffer[3];
        tempSize                = 5U;
    }
    else
    {
        retVal = kStatus_InvalidArgument;
    }

    if (retVal == kStatus_Success)
    {
        retVal = SMBUS_Master_Port_Write(&handle->port, transfer->slaveAddress, handle->packetBuffer, tempSize,
                                         handle->timeoutMS, kSMBUS_PORT_WriteNoStopFlag);
        if (retVal != kStatus_Success)
        {
            retVal = kStatus_Fail;
        }
        else
        {
            if (protocol == kSMBUS_BlockWriteBlockRead)
            {
                tempSize = transfer->transferByteSize + 1;
            }
            else
            {
                tempSize = 4;
            }
            if (transfer->needPEC)
            {
                tempSize++;
            }
            retVal = SMBUS_Master_Port_Read(&handle->port, transfer->slaveAddress, handle->packetBuffer, tempSize,
                                            handle->timeoutMS, kSMBUS_PORT_ReadRepeatStartFlag);
            if (retVal != kStatus_Success)
            {
                retVal = kStatus_Fail;
            }
            else
            {
                if (transfer->needPEC)
                {
                    handle->crcBuffer[0] = (transfer->slaveAddress << 1) | 0x01U;
                    for (uint8_t i = 1; i < tempSize; i++)
                    {
                        handle->crcBuffer[i] = handle->packetBuffer[i - 1];
                    }
                    tempCRC = SMBUS_CRC8(handle->crcBuffer, tempSize);
                    if (tempCRC != handle->packetBuffer[tempSize - 1])
                    {
                        retVal = kStatus_Fail;
                    }
                }

                if (retVal == kStatus_Success)
                {
                    if (protocol == kSMBUS_BlockWriteBlockRead)
                    {
                        memcpy(transfer->readBuffer, &handle->packetBuffer[1], transfer->transferByteSize);
                    }
                    else
                    {
                        memcpy(transfer->readBuffer, &handle->packetBuffer[0], transfer->transferByteSize);
                    }
                }
            }
        }
    }

    return retVal;
}

status_t SMBUS_Master_GetDeviceID(smbus_master_handle_t *handle, uint8_t *deviceID)
{
    assert(handle != NULL);

    status_t retVal = kStatus_Success;
    uint8_t tempCRC = 0U;

    handle->packetBuffer[0] = SMBUS_CMD_GET_DEVICE_ID;
    retVal = SMBUS_Master_Port_Write(&handle->port, SMBUS_DEVICE_DEFAULT_ADDRESS, handle->packetBuffer, 1U,
                                     handle->timeoutMS, kSMBUS_PORT_WriteNoStopFlag);
    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }
    else
    {
        /* read data from master to slave */
        retVal = SMBUS_Master_Port_Read(&handle->port, SMBUS_DEVICE_DEFAULT_ADDRESS, handle->packetBuffer,
                                        SMBUS_CMD_GET_DEVICE_ID_PACKET_SIZE, handle->timeoutMS,
                                        kSMBUS_PORT_ReadRepeatStartFlag);
        if (retVal != kStatus_Success)
        {
            retVal = kStatus_Fail;
        }
        else
        {
            handle->crcBuffer[0] = (SMBUS_DEVICE_DEFAULT_ADDRESS << 1U) | 0x01U;
            for (uint8_t i = 1; i <= SMBUS_CMD_GET_DEVICE_ID_PACKET_SIZE; i++)
            {
                handle->crcBuffer[i] = handle->packetBuffer[i - 1];
            }
            tempCRC = SMBUS_CRC8(handle->crcBuffer, SMBUS_CMD_GET_DEVICE_ID_PACKET_SIZE);
            if (tempCRC != handle->packetBuffer[SMBUS_CMD_GET_DEVICE_ID_PACKET_SIZE - 1U])
            {
                retVal = kStatus_Fail;
            }
            else
            {
                memcpy(deviceID, &handle->packetBuffer[1U], SMBUS_DEVICE_UDID_SIZE);
            }
        }
    }
    return retVal;
}

void SMBUS_Slave_Init(smbus_slave_handle_t *handle, smbus_slave_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);

    handle->slaveAddress = config->slaveAddress;
    handle->timeoutMS    = config->timeoutMS;

    SMBUS_Slave_Port_Init(&handle->port, &config->portConfig, config->slaveAddress);
}

void SMBUS_Slave_Deinit(smbus_slave_handle_t *handle)
{
    assert(handle != NULL);

    SMBUS_Slave_Port_Deinit(&handle->port);
}

status_t SMBUS_Slave_WriteByte(smbus_slave_handle_t *handle, smbus_slave_transfer_t *transfer)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal         = kStatus_Fail;
    uint32_t writeSize      = 1U;
    handle->packetBuffer[0] = transfer->writeBuffer[0U];

    if (transfer->needPEC)
    {
        handle->crcBuffer[0]    = handle->slaveAddress << 1U;
        handle->crcBuffer[1]    = transfer->writeBuffer[0];
        handle->packetBuffer[1] = SMBUS_CRC8(handle->crcBuffer, 2);
        writeSize++;
    }

    retVal = SMBUS_Slave_Port_Write(&handle->port, handle->packetBuffer, writeSize, handle->timeoutMS);
    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }

    return retVal;
}

status_t SMBUS_Slave_ReadByte(smbus_slave_handle_t *handle, smbus_slave_transfer_t *transfer)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal   = kStatus_Fail;
    uint32_t readSize = 1U;
    uint8_t tempCRC;

    if (transfer->needPEC)
    {
        readSize++;
    }

    retVal = SMBUS_Slave_Port_Read(&handle->port, handle->packetBuffer, readSize, handle->timeoutMS);
    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }

    if (transfer->needPEC)
    {
        handle->crcBuffer[0] = (handle->slaveAddress << 1) | 0x01U;
        handle->crcBuffer[1] = handle->packetBuffer[0];
        tempCRC              = SMBUS_CRC8(handle->crcBuffer, 2);
        if (tempCRC != handle->packetBuffer[1])
        {
            retVal = kStatus_Fail;
        }
    }

    if (retVal == kStatus_Success)
    {
        transfer->readBuffer[0U] = handle->packetBuffer[0];
    }

    return retVal;
}

status_t SMBUS_Slave_Write(smbus_slave_handle_t *handle,
                           smbus_slave_transfer_t *transfer,
                           smbus_bus_protocol_t protocol)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal    = kStatus_Fail;
    uint32_t writeSize = 0U;

    switch (protocol)
    {
        case kSMBUS_WriteByte:
            handle->packetBuffer[0] = (uint8_t)protocol;
            handle->packetBuffer[1] = transfer->writeBuffer[0];
            writeSize               = 2U;
            break;
        case kSMBUS_WriteWord:
        case kSMBUS_Write32:
            handle->packetBuffer[0] = (uint8_t)protocol;
            handle->packetBuffer[1] = transfer->writeBuffer[0];
            handle->packetBuffer[2] = transfer->writeBuffer[1];
            handle->packetBuffer[3] = transfer->writeBuffer[2];
            handle->packetBuffer[4] = transfer->writeBuffer[3];
            writeSize               = 5U;
            break;
        case kSMBUS_Write64:
            handle->packetBuffer[0] = (uint8_t)protocol;
            handle->packetBuffer[1] = transfer->writeBuffer[0];
            handle->packetBuffer[2] = transfer->writeBuffer[1];
            handle->packetBuffer[3] = transfer->writeBuffer[2];
            handle->packetBuffer[4] = transfer->writeBuffer[3];
            handle->packetBuffer[5] = transfer->writeBuffer[4];
            handle->packetBuffer[6] = transfer->writeBuffer[5];
            handle->packetBuffer[7] = transfer->writeBuffer[6];
            handle->packetBuffer[8] = transfer->writeBuffer[7];
            writeSize               = 9U;
            break;
        case kSMBUS_WriteBlock:
            handle->packetBuffer[0] = (uint8_t)protocol;
            handle->packetBuffer[1] = transfer->transferByteSize;
            for (uint8_t i = 0; i < transfer->transferByteSize; i++)
            {
                handle->packetBuffer[i + 2U] = transfer->writeBuffer[i];
            }
            writeSize = transfer->transferByteSize + 2U;
            break;
        default:
            return kStatus_InvalidArgument;
    }

    if (transfer->needPEC)
    {
        handle->crcBuffer[0] = handle->slaveAddress << 1U;
        for (uint8_t i = 1; i <= writeSize; i++)
        {
            handle->crcBuffer[i] = handle->packetBuffer[i - 1U];
        }
        handle->packetBuffer[writeSize] = SMBUS_CRC8(handle->crcBuffer, writeSize + 1U);
        writeSize++;
    }

    retVal = SMBUS_Slave_Port_Write(&handle->port, handle->packetBuffer, writeSize, handle->timeoutMS);
    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }

    return retVal;
}

status_t SMBUS_Slave_Read(smbus_slave_handle_t *handle, smbus_slave_transfer_t *transfer, smbus_bus_protocol_t protocol)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal   = kStatus_Success;
    uint32_t readSize = 0U;
    uint8_t tempCRC;

    switch (protocol)
    {
        case kSMBUS_ReadByte:
            handle->packetBuffer[0] = (uint8_t)protocol;
            readSize                = 1;
            break;
        case kSMBUS_ReadWord:
        case kSMBUS_Read32:
            handle->packetBuffer[0] = (uint8_t)protocol;
            readSize                = 4;
            break;
        case kSMBUS_Read64:
            handle->packetBuffer[0] = (uint8_t)protocol;
            readSize                = 8;
            break;
        case kSMBUS_ReadBlock:
            handle->packetBuffer[0] = (uint8_t)protocol;
            readSize                = transfer->transferByteSize + 1U;
            break;
        default:
            retVal = kStatus_InvalidArgument;
            break;
    }

    if (retVal == kStatus_Success)
    {
        retVal = SMBUS_Slave_Port_Write(&handle->port, handle->packetBuffer, 1U, handle->timeoutMS);
        if (retVal != kStatus_Success)
        {
            retVal = kStatus_Fail;
        }
        else
        {
            if (transfer->needPEC)
            {
                readSize++;
            }

            retVal = SMBUS_Slave_Port_Read(&handle->port, handle->packetBuffer, readSize, handle->timeoutMS);
            if (retVal != kStatus_Success)
            {
                retVal = kStatus_Fail;
            }
            else
            {
                if (transfer->needPEC)
                {
                    handle->crcBuffer[0] = (handle->slaveAddress << 1U) | 0x01;
                    for (uint8_t i = 1U; i < readSize; i++)
                    {
                        handle->crcBuffer[i] = handle->packetBuffer[i - 1U];
                    }
                    tempCRC = SMBUS_CRC8(handle->crcBuffer, readSize);
                    if (tempCRC != handle->packetBuffer[readSize - 1U])
                    {
                        retVal = kStatus_Fail;
                    }
                }

                if (retVal == kStatus_Success)
                {
                    if (protocol == kSMBUS_ReadBlock)
                    {
                        memcpy(transfer->readBuffer, &handle->packetBuffer[1], transfer->transferByteSize);
                    }
                    else
                    {
                        memcpy(transfer->readBuffer, &handle->packetBuffer[0], transfer->transferByteSize);
                    }
                }
            }
        }
    }

    return retVal;
}

status_t SMBUS_Slave_ProcessCall(smbus_slave_handle_t *handle,
                                 smbus_slave_transfer_t *transfer,
                                 smbus_bus_protocol_t protocol)
{
    assert(handle != NULL);
    assert(transfer != NULL);

    status_t retVal   = kStatus_Success;
    uint8_t tempCRC   = 0;
    uint32_t tempSize = {0};

    if (protocol == kSMBUS_BlockWriteBlockRead)
    {
        handle->packetBuffer[0] = (uint8_t)kSMBUS_BlockWriteBlockRead;
        handle->packetBuffer[1] = transfer->transferByteSize;
        for (uint8_t i = 0; i < transfer->transferByteSize; i++)
        {
            handle->packetBuffer[i + 2U] = transfer->writeBuffer[i];
        }
        tempSize = transfer->transferByteSize + 2U;
    }
    else if (protocol == kSMBUS_ProcessCall)
    {
        handle->packetBuffer[0] = (uint8_t)kSMBUS_ProcessCall;
        handle->packetBuffer[1] = transfer->writeBuffer[0];
        handle->packetBuffer[2] = transfer->writeBuffer[1];
        handle->packetBuffer[3] = transfer->writeBuffer[2];
        handle->packetBuffer[4] = transfer->writeBuffer[3];
        tempSize                = 5U;
    }
    else
    {
        retVal = kStatus_InvalidArgument;
    }

    if (retVal == kStatus_Success)
    {
        retVal = SMBUS_Slave_Port_Write(&handle->port, handle->packetBuffer, tempSize, handle->timeoutMS);
        if (retVal != kStatus_Success)
        {
            retVal = kStatus_Fail;
        }
        else
        {
            if (protocol == kSMBUS_BlockWriteBlockRead)
            {
                tempSize = transfer->transferByteSize + 1U;
            }
            else
            {
                tempSize = 4U;
            }

            if (transfer->needPEC)
            {
                tempSize++;
            }

            retVal = SMBUS_Slave_Port_Read(&handle->port, handle->packetBuffer, tempSize, handle->timeoutMS);
            if (retVal != kStatus_Success)
            {
                retVal = kStatus_Fail;
            }
            else
            {
                if (transfer->needPEC)
                {
                    handle->crcBuffer[0] = (handle->slaveAddress << 1U) | 0x01U;
                    for (uint8_t i = 1; i < tempSize; i++)
                    {
                        handle->crcBuffer[i] = handle->packetBuffer[i - 1];
                    }
                    tempCRC = SMBUS_CRC8(handle->crcBuffer, tempSize);
                    if (tempCRC != handle->packetBuffer[tempSize - 1])
                    {
                        retVal = kStatus_Fail;
                    }
                }

                if (protocol == kSMBUS_BlockWriteBlockRead)
                {
                    memcpy(transfer->readBuffer, &handle->packetBuffer[1], transfer->transferByteSize);
                }
                else
                {
                    memcpy(transfer->readBuffer, &handle->packetBuffer[0], transfer->transferByteSize);
                }
            }
        }
    }

    return retVal;
}
