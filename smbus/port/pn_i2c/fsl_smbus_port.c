/*
 * Copyright 2022 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_smbus_port.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void i2c_master_callback(I2C_Type *base, i2c_master_gpdma_handle_t *handle, status_t status, void *userData);
static void i2c_slave_callback(I2C_Type *base, volatile i2c_slave_transfer_t *xfer, void *userData);
/*******************************************************************************
 * Code
 ******************************************************************************/
static void i2c_master_callback(I2C_Type *base, i2c_master_gpdma_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        SMBUS_EventSet(&((smbus_master_port_handle_t *)userData)->event, SMBUS_EVENT_TRANSFER_MASTER_COMPLETE);
    }
    else
    {
        SMBUS_EventSet(&((smbus_master_port_handle_t *)userData)->event, SMBUS_EVENT_TRANSFER_MASTER_ERROR);
    }
}

static void i2c_slave_callback(I2C_Type *base, volatile i2c_slave_transfer_t *xfer, void *userData)
{
    assert(userData != NULL);

    switch (xfer->event)
    {
        /*  Address match event */
        case kI2C_SlaveAddressMatchEvent:
            xfer->rxData = NULL;
            xfer->rxSize = 0;
            break;
        /*  Receive request */
        case kI2C_SlaveReceiveEvent:
            /*  Update information for received process */
            xfer->rxData = ((smbus_slave_port_handle_t *)userData)->data;
            xfer->rxSize = ((smbus_slave_port_handle_t *)userData)->dataSize;
            break;

        /*  Transfer done */
        case kI2C_SlaveCompletionEvent:
            /*  Transmit request */
            SMBUS_EventSet(&((smbus_slave_port_handle_t *)userData)->event, SMBUS_EVENT_TRANSFER_SLAVE_COMPLETE);
            xfer->rxData = NULL;
            xfer->txData = NULL;
            break;

        /*  Transmit request */
        case kI2C_SlaveTransmitEvent:
            /*  Update information for transmit process */
            xfer->txData = ((smbus_slave_port_handle_t *)userData)->data;
            xfer->txSize = ((smbus_slave_port_handle_t *)userData)->dataSize;
            break;

        default:
            break;
    }
}

void SMBUS_Master_Port_Init(smbus_master_port_handle_t *handle,
                            uint32_t baudRate_Bps,
                            const smbus_port_config_t *config)
{
    i2c_master_config_t masterConfig;

    /* Initialize GPDMA */
    GPDMA_Init(config->dmaBase);

    /*
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.enableMaster = true;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = baudRate_Bps;
    handle->i2cBase           = config->i2cBase;

    /* Initialize the I2C master peripheral */
    I2C_MasterInit(config->i2cBase, &masterConfig, config->srcClock_Hz);

    /* Create GPDMA Channel and I2C GPDMA handle for the transfer */
    GPDMA_CreateHandle(&handle->gpdmaHandle, config->dmaBase, config->dmaChannel);

    I2C_MasterTransferCreateHandleGPDMA(config->i2cBase, &handle->i2cMasterDmaHandle, i2c_master_callback, handle,
                                        &handle->gpdmaHandle);
    GPDMA_SetChannelDestinationPeripheral(config->dmaBase, config->dmaChannel, kGpdmaRequestMuxI2cMasterMst);
    GPDMA_SetChannelSourcePeripheral(config->dmaBase, config->dmaChannel, kGpdmaRequestMuxI2cMasterMst);
    /* create master event */
    SMBUS_EventCreate(&handle->event);
}

void SMBUS_Master_Port_Deinit(smbus_master_port_handle_t *handle)
{
    I2C_MasterDeinit(handle->i2cBase);
    (void)SMBUS_EventDestroy(&handle->event);
}

status_t SMBUS_Master_Port_Write(smbus_master_port_handle_t *handle,
                                 uint8_t slaveAddress,
                                 uint8_t *buffer,
                                 uint32_t writeSize,
                                 uint32_t timeoutMS,
                                 uint32_t flag)
{
    i2c_master_transfer_t masterXfer = {0};
    status_t retVal                  = kStatus_Success;
    uint32_t event                   = 0U;

    masterXfer.slaveAddress   = slaveAddress;
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddressSize = 0;
    masterXfer.data           = buffer;
    masterXfer.dataSize       = writeSize;
    masterXfer.flags          = flag;

    /* clear transfer event */
    SMBUS_EventClear(&handle->event, SMBUS_EVENT_TRANSFER_MASTER_COMPLETE | SMBUS_EVENT_TRANSFER_MASTER_ERROR);

    /* Send Data to slave */
    retVal = I2C_MasterTransferGPDMA(handle->i2cBase, &handle->i2cMasterDmaHandle, &masterXfer);
    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }
    else
    {
        if ((kStatus_Fail ==
             SMBUS_EventWait(&handle->event, SMBUS_EVENT_TRANSFER_MASTER_COMPLETE, timeoutMS, &event)) ||
            ((event & SMBUS_EVENT_TRANSFER_MASTER_ERROR) != 0U))
        {
            retVal = kStatus_Fail;
        }
    }

    return retVal;
}

status_t SMBUS_Master_Port_Read(smbus_master_port_handle_t *handle,
                                uint8_t slaveAddress,
                                uint8_t *buffer,
                                uint32_t readSize,
                                uint32_t timeoutMS,
                                uint32_t flag)
{
    i2c_master_transfer_t masterXfer = {0};
    status_t retVal                  = kStatus_Success;
    uint32_t event                   = 0U;

    masterXfer.slaveAddress   = slaveAddress;
    masterXfer.direction      = kI2C_Read;
    masterXfer.subaddressSize = 0;
    masterXfer.data           = buffer;
    masterXfer.dataSize       = readSize;
    masterXfer.flags          = flag;

    /* clear transfer event */
    SMBUS_EventClear(&handle->event, SMBUS_EVENT_TRANSFER_MASTER_COMPLETE | SMBUS_EVENT_TRANSFER_MASTER_ERROR);

    /* Send Data to slave */
    retVal = I2C_MasterTransferGPDMA(handle->i2cBase, &handle->i2cMasterDmaHandle, &masterXfer);
    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }
    else
    {
        if ((kStatus_Fail ==
             SMBUS_EventWait(&handle->event, SMBUS_EVENT_TRANSFER_MASTER_COMPLETE, timeoutMS, &event)) ||
            ((event & SMBUS_EVENT_TRANSFER_MASTER_ERROR) != 0U))
        {
            retVal = kStatus_Fail;
        }
    }

    return retVal;
}

void SMBUS_Slave_Port_Init(smbus_slave_port_handle_t *handle, const smbus_port_config_t *config, uint8_t slaveAddress)
{
    i2c_slave_config_t slaveConfig;

    /* Initialize GPDMA */
    GPDMA_Init(config->dmaBase);

    /* Set up i2c slave */
    I2C_SlaveGetDefaultConfig(&slaveConfig);

    slaveConfig.address0.address = slaveAddress;

    /* Initialize the I2C slave peripheral */
    I2C_SlaveInit(config->i2cBase, &slaveConfig, config->srcClock_Hz);

    handle->i2cBase = config->i2cBase;

    GPDMA_CreateHandle(&handle->gpdmaHandle, config->dmaBase, config->dmaChannel);

    I2C_SlaveTransferCreateHandleGPDMA(handle->i2cBase, &handle->i2cSlaveDmaHandle, i2c_slave_callback, handle,
                                       &handle->gpdmaHandle);
    /* Create the I2C handle for the non-blocking transfer */
    GPDMA_SetChannelDestinationPeripheral(config->dmaBase, config->dmaChannel, kGpdmaRequestMuxI2cMasterSlv);
    GPDMA_SetChannelSourcePeripheral(config->dmaBase, config->dmaChannel, kGpdmaRequestMuxI2cMasterSlv);

    /* create slave event */
    SMBUS_EventCreate(&handle->event);
}

void SMBUS_Slave_Port_Deinit(smbus_slave_port_handle_t *handle)
{
    I2C_SlaveDeinit(handle->i2cBase);
    (void)SMBUS_EventDestroy(&handle->event);
}

status_t SMBUS_Slave_Port_Write(smbus_slave_port_handle_t *handle,
                                uint8_t *buffer,
                                uint32_t writeSize,
                                uint32_t timeoutMS)
{
    status_t retVal = kStatus_Success;
    uint32_t event  = 0U;

    handle->data     = buffer;
    handle->dataSize = writeSize;

    /* clear transfer event */
    SMBUS_EventClear(&handle->event, SMBUS_EVENT_TRANSFER_SLAVE_COMPLETE);

    /* Start accepting I2C transfers on the I2C slave peripheral */
    retVal = I2C_SlaveTransferGPDMA(handle->i2cBase, &handle->i2cSlaveDmaHandle,
                                    kI2C_SlaveAddressMatchEvent | kI2C_SlaveCompletionEvent);
    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }
    else
    {
        if (kStatus_Fail == SMBUS_EventWait(&handle->event, SMBUS_EVENT_TRANSFER_SLAVE_COMPLETE, timeoutMS, &event))
        {
            retVal = kStatus_Fail;
        }
    }

    return retVal;
}

status_t SMBUS_Slave_Port_Read(smbus_slave_port_handle_t *handle,
                               uint8_t *buffer,
                               uint32_t readSize,
                               uint32_t timeoutMS)
{
    status_t retVal = kStatus_Success;
    uint32_t event  = 0U;

    handle->data     = buffer;
    handle->dataSize = readSize;

    /* clear transfer event */
    SMBUS_EventClear(&handle->event, SMBUS_EVENT_TRANSFER_SLAVE_COMPLETE);

    /* Start accepting I2C transfers on the I2C slave peripheral */
    retVal = I2C_SlaveTransferGPDMA(handle->i2cBase, &handle->i2cSlaveDmaHandle,
                                    kI2C_SlaveAddressMatchEvent | kI2C_SlaveCompletionEvent);
    if (retVal != kStatus_Success)
    {
        retVal = kStatus_Fail;
    }
    else
    {
        if (kStatus_Fail == SMBUS_EventWait(&handle->event, SMBUS_EVENT_TRANSFER_SLAVE_COMPLETE, timeoutMS, &event))
        {
            retVal = kStatus_Fail;
        }
    }

    return retVal;
}
