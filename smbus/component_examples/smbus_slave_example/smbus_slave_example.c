/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  SDK Included Files */
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "fsl_i2c_gpdma.h"
#include "app.h"
#include "fsl_smbus.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define EXAMPLE_I2C_SLAVE ((I2C_Type *)EXAMPLE_I2C_SLAVE_BASE)

#define I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
#define I2C_DATA_LENGTH            (34) /* MAX is 256 */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void i2s_slave_smbus_master(void);
void i2s_master_smbus_slave(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t cmpWriteByteBuff[2]       = {kSMBUS_WriteByte, 0x5A};
uint8_t cmpWriteWordBuff[5]       = {kSMBUS_WriteWord, 0x55, 0xAA, 0x5A, 0xA5};
uint8_t cmpWriteWord32Buff[5]     = {kSMBUS_Write32, 0x5A, 0xA5, 0x55, 0xAA};
uint8_t cmpWriteWord64Buff[9]     = {kSMBUS_Write64, 0x55, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5, 0x55, 0xAA};
uint8_t cmpWriteWordBlockBuff[18] = {kSMBUS_WriteBlock,
                                     0x10,
                                     0x00,
                                     0x01,
                                     0x02,
                                     0x03,
                                     0x04,
                                     0x05,
                                     0x06,
                                     0x07,
                                     0x08,
                                     0x09,
                                     0x0A,
                                     0x0B,
                                     0x0C,
                                     0x0D,
                                     0x0E,
                                     0x0F};
uint8_t txBlock[19]               = {0x12, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                     0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11};
uint8_t txProcessCall[9]          = {0, 1, 2, 3, 4, 5, 6, 7, 8};
uint8_t deviceID[18]              = {0x11,                                           /* Byte Count */
                                     0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
                                     0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, /* UID */
                                     0x7E /* Slave Address */};
uint8_t g_slave_rxbuff[I2C_DATA_LENGTH];
uint8_t g_slave_txbuff[I2C_DATA_LENGTH];
uint8_t g_master_rxbuff[I2C_DATA_LENGTH];
uint8_t g_master_txbuff[I2C_DATA_LENGTH];
i2c_slave_gpdma_handle_t g_s_gpdma_handle;
i2c_master_gpdma_handle_t g_m_gpdma_handle;
gpdma_handle_t g_gpdmaHandle;
volatile bool g_CompletionFlag = false;
i2c_slave_transfer_t slaveXfer;

/*******************************************************************************
 * Code
 ******************************************************************************/

int APP_WaitString(uint8_t *pBuf, uint16_t wSize)
{
    uint8_t r;
    uint16_t rBytes = 0;
    do
    {
        do
        {
            r = GETCHAR();
        } while (((r == 0x0A) || (r == 0x0D)) && (rBytes == 0));
        /* If Char is Enter break */
        if ((r == 0x0A) || (r == 0x0D))
        {
            break;
        }
        pBuf[rBytes++] = (uint8_t)r;
    } while (rBytes < wSize);
    return rBytes;
}
static void i2c_slave_callback(I2C_Type *base, volatile i2c_slave_transfer_t *slaveXfer, void *userData)
{
    switch (slaveXfer->event)
    {
        /*  Address match event */
        case kI2C_SlaveAddressMatchEvent:
            slaveXfer->rxData = NULL;
            slaveXfer->rxSize = 0U;
            break;
        /*  Receive request */
        case kI2C_SlaveReceiveEvent:
            slaveXfer->rxData = ((i2c_slave_transfer_t *)userData)->rxData;
            slaveXfer->rxSize = ((i2c_slave_transfer_t *)userData)->rxSize;
            break;

        /*  Transfer done */
        case kI2C_SlaveCompletionEvent:
            /*  Transmit request */
            g_CompletionFlag  = true;
            slaveXfer->rxData = NULL;
            slaveXfer->txData = NULL;
            break;

        /*  Transmit request */
        case kI2C_SlaveTransmitEvent:
            slaveXfer->txData = ((i2c_slave_transfer_t *)userData)->txData;
            slaveXfer->txSize = ((i2c_slave_transfer_t *)userData)->txSize;
            break;

        default:
            g_CompletionFlag = false;
            break;
    }
}

int main(void)
{
    BOARD_InitHardware();

    i2s_slave_smbus_master();

    i2s_master_smbus_slave();
    while (1)
    {
    }
}
static uint8_t generate_crc8(const uint8_t *data, uint8_t length)
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

static status_t verify_crc8(const uint8_t *data, uint8_t length, uint8_t targetCrc8)
{
    if (generate_crc8(data, length) != targetCrc8)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

static status_t slave_transferData(void *rxData, uint32_t rxSize, void *txData, uint32_t txSize)
{
    if (rxData != NULL)
    {
        g_CompletionFlag = false;

        slaveXfer.rxData = rxData;
        slaveXfer.rxSize = rxSize;
        I2C_SlaveTransferGPDMA(EXAMPLE_I2C_SLAVE, &g_s_gpdma_handle,
                               kI2C_SlaveCompletionEvent | kI2C_SlaveAddressMatchEvent);

        /* Wait for master receive completed.*/
        while (!g_CompletionFlag)
        {
        }
        g_CompletionFlag = false;
    }

    if (txData != NULL)
    {
        g_CompletionFlag = false;

        slaveXfer.txData = txData;
        slaveXfer.txSize = txSize;
        I2C_SlaveTransferGPDMA(EXAMPLE_I2C_SLAVE, &g_s_gpdma_handle,
                               kI2C_SlaveCompletionEvent | kI2C_SlaveAddressMatchEvent);

        /* Wait for master receive completed.*/
        while (!g_CompletionFlag)
        {
        }
        g_CompletionFlag = false;
    }

    return kStatus_Success;
}

void i2s_slave_smbus_master(void)
{
    i2c_slave_config_t slaveConfig;
    GPDMA_Init(DEMO_DMA);

    /*Set up i2c slave first*/
    /*
     * slaveConfig.addressingMode = kI2C_Address7bit;
     * slaveConfig.enableGeneralCall = false;
     * slaveConfig.enableWakeUp = false;
     * slaveConfig.enableBaudRateCtl = false;
     * slaveConfig.enableSlave = true;
     */
    I2C_SlaveGetDefaultConfig(&slaveConfig);

    /* Change the slave address */
    slaveConfig.address0.address = I2C_MASTER_SLAVE_ADDR_7BIT;

    /* Initialize the I2C slave peripheral */
    I2C_SlaveInit(EXAMPLE_I2C_SLAVE, &slaveConfig, I2C_SLAVE_CLOCK_FREQUENCY);

    memset(g_slave_rxbuff, 0, sizeof(g_slave_rxbuff));
    memset(&g_s_gpdma_handle, 0, sizeof(g_s_gpdma_handle));

    GPDMA_CreateHandle(&g_gpdmaHandle, DEMO_DMA, EXAMPLE_I2C_SLAVE_CHANNEL);
    GPDMA_SetChannelDestinationPeripheral(DEMO_DMA, EXAMPLE_I2C_SLAVE_CHANNEL, kGpdmaRequestMuxI2cMasterSlv);
    GPDMA_SetChannelSourcePeripheral(DEMO_DMA, EXAMPLE_I2C_SLAVE_CHANNEL, kGpdmaRequestMuxI2cMasterSlv);

    I2C_SlaveTransferCreateHandleGPDMA(EXAMPLE_I2C_SLAVE, &g_s_gpdma_handle, i2c_slave_callback, &slaveXfer,
                                       &g_gpdmaHandle);

    g_slave_rxbuff[0] = I2C_MASTER_SLAVE_ADDR_7BIT << 1U;
    g_slave_txbuff[0] = (I2C_MASTER_SLAVE_ADDR_7BIT << 1U) | 0x1U;

    /* receive byte */
    slave_transferData(&g_slave_rxbuff[1], 1U, NULL, 0U);
    if (g_slave_rxbuff[1] == 0xA5U)
    {
        PRINTF("Receive byte successfully .\r\n");
    }
    else
    {
        PRINTF("Receive byte failed .\r\n");
    }

    /* receive byte with crc */
    slave_transferData(&g_slave_rxbuff[1], 2U, NULL, 0U);
    if (verify_crc8(g_slave_rxbuff, 2U, g_slave_rxbuff[2]) == kStatus_Success)
    {
        PRINTF("Receive byte with CRC successfully .\r\n");
    }
    else
    {
        PRINTF("Receive byte with CRC failed .\r\n");
    }

    /* receive byte command */
    slave_transferData(&g_slave_rxbuff[1], 2U, NULL, 0U);
    if ((g_slave_rxbuff[1U] == kSMBUS_WriteByte) && (g_slave_rxbuff[2U] == 0x5A))
    {
        PRINTF("Receive byte command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive byte command failed .\r\n");
    }

    /* receive byte command with CRC */
    slave_transferData(&g_slave_rxbuff[1], 3U, NULL, 0U);
    if ((g_slave_rxbuff[1U] == kSMBUS_WriteByte) && (g_slave_rxbuff[2U] == 0x5A) &&
        (verify_crc8(g_slave_rxbuff, 3U, g_slave_rxbuff[3]) == kStatus_Success))
    {
        PRINTF("Receive byte command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive byte command with crc failed .\r\n");
    }

    /* receive WORD command */
    slave_transferData(&g_slave_rxbuff[1], 5U, NULL, 0U);
    if ((g_slave_rxbuff[1U] == kSMBUS_WriteWord) && (g_slave_rxbuff[2U] == 0x55) && (g_slave_rxbuff[3U] == 0xAA) &&
        (g_slave_rxbuff[4U] == 0x5A) && (g_slave_rxbuff[5U] == 0xA5))
    {
        PRINTF("Receive word command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive word command failed .\r\n");
    }

    /* receive WORD command with CRC */
    slave_transferData(&g_slave_rxbuff[1], 6U, NULL, 0U);
    if ((g_slave_rxbuff[1U] == kSMBUS_WriteWord) && (g_slave_rxbuff[2U] == 0x55) && (g_slave_rxbuff[3U] == 0xAA) &&
        (g_slave_rxbuff[4U] == 0x5A) && (g_slave_rxbuff[5U] == 0xA5) &&
        (verify_crc8(g_slave_rxbuff, 6U, g_slave_rxbuff[6]) == kStatus_Success))
    {
        PRINTF("Receive word command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive word command with crc failed .\r\n");
    }

    /* receive 32 command */
    slave_transferData(&g_slave_rxbuff[1], 5U, NULL, 0U);
    if ((g_slave_rxbuff[1U] == kSMBUS_Write32) && (g_slave_rxbuff[2U] == 0x5A) && (g_slave_rxbuff[3U] == 0xA5) &&
        (g_slave_rxbuff[4U] == 0x55) && (g_slave_rxbuff[5U] == 0xAA))
    {
        PRINTF("Receive 32 command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive 32 command failed .\r\n");
    }

    /* receive 32 command with CRC */
    slave_transferData(&g_slave_rxbuff[1], 6U, NULL, 0U);
    if ((g_slave_rxbuff[1U] == kSMBUS_Write32) && (g_slave_rxbuff[2U] == 0x5A) && (g_slave_rxbuff[3U] == 0xA5) &&
        (g_slave_rxbuff[4U] == 0x55) && (g_slave_rxbuff[5U] == 0xAA) &&
        (verify_crc8(g_slave_rxbuff, 6U, g_slave_rxbuff[6]) == kStatus_Success))
    {
        PRINTF("Receive 32 command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive 32 command with crc failed .\r\n");
    }

    /* receive 64 command */
    slave_transferData(&g_slave_rxbuff[1], 9U, NULL, 0U);
    if (memcmp(cmpWriteWord64Buff, &g_slave_rxbuff[1], 9U) == 0U)
    {
        PRINTF("Receive 64 command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive 64 command failed .\r\n");
    }

    /* receive 64 command with CRC */
    slave_transferData(&g_slave_rxbuff[1], 10U, NULL, 0U);
    if ((memcmp(cmpWriteWord64Buff, &g_slave_rxbuff[1], 9U) == 0U) &&
        (verify_crc8(g_slave_rxbuff, 10U, g_slave_rxbuff[10]) == kStatus_Success))
    {
        PRINTF("Receive 64 command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive 64 command with crc failed .\r\n");
    }

    /* receive BLOCK command */
    slave_transferData(&g_slave_rxbuff[1], 18U, NULL, 0U);
    if (memcmp(cmpWriteWordBlockBuff, &g_slave_rxbuff[1], 18U) == 0U)
    {
        PRINTF("Receive block command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive block command failed .\r\n");
    }

    /* receive block command with CRC */
    slave_transferData(&g_slave_rxbuff[1], 19U, NULL, 0U);
    if ((memcmp(cmpWriteWordBlockBuff, &g_slave_rxbuff[1], 18U) == 0U) &&
        (verify_crc8(g_slave_rxbuff, 19U, g_slave_rxbuff[19]) == kStatus_Success))
    {
        PRINTF("Receive block command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive block command with crc failed .\r\n");
    }

    /* send byte */
    g_slave_txbuff[1] = 0x5A;
    slave_transferData(NULL, 0U, &g_slave_txbuff[1], 1U);
    PRINTF("Send byte successfully.\r\n");

    g_slave_txbuff[1] = 0x5A;
    g_slave_txbuff[2] = generate_crc8(g_slave_txbuff, 2U);
    slave_transferData(NULL, 0U, &g_slave_txbuff[1], 2U);
    PRINTF("Send byte with crc successfully.\r\n");

    g_slave_txbuff[1] = 0x5A;
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 1U);
    if (g_slave_rxbuff[0] == kSMBUS_ReadByte)
    {
        PRINTF("Send byte cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send byte cmd failed.\r\n");
    }

    g_slave_txbuff[1] = 0x5A;
    g_slave_txbuff[2] = generate_crc8(g_slave_txbuff, 2U);
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 2U);
    if (g_slave_rxbuff[0] == kSMBUS_ReadByte)
    {
        PRINTF("Send byte cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send byte cmd with crc failed.\r\n");
    }

    g_slave_txbuff[1] = 0xAA;
    g_slave_txbuff[2] = 0x55;
    g_slave_txbuff[3] = 0xA5;
    g_slave_txbuff[4] = 0x5A;
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 4U);
    if (g_slave_rxbuff[0] == kSMBUS_ReadWord)
    {
        PRINTF("Send word cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send word cmd failed.\r\n");
    }

    g_slave_txbuff[1] = 0xAA;
    g_slave_txbuff[2] = 0x55;
    g_slave_txbuff[3] = 0xA5;
    g_slave_txbuff[4] = 0x5A;
    g_slave_txbuff[5] = generate_crc8(g_slave_txbuff, 5U);
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 5U);
    if (g_slave_rxbuff[0] == kSMBUS_ReadWord)
    {
        PRINTF("Send word cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send word cmd with crc failed.\r\n");
    }

    g_slave_txbuff[1] = 0xA5;
    g_slave_txbuff[2] = 0x5A;
    g_slave_txbuff[3] = 0xAA;
    g_slave_txbuff[4] = 0x55;
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 4U);
    if (g_slave_rxbuff[0] == kSMBUS_Read32)
    {
        PRINTF("Send 32 cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send 32 cmd failed.\r\n");
    }

    g_slave_txbuff[1] = 0xA5;
    g_slave_txbuff[2] = 0x5A;
    g_slave_txbuff[3] = 0xAA;
    g_slave_txbuff[4] = 0x55;
    g_slave_txbuff[5] = generate_crc8(g_slave_txbuff, 5U);
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 5U);
    if (g_slave_rxbuff[0] == kSMBUS_Read32)
    {
        PRINTF("Send 32 cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send 32 cmd with crc failed.\r\n");
    }

    g_slave_txbuff[1] = 0x55;
    g_slave_txbuff[2] = 0xAA;
    g_slave_txbuff[3] = 0x55;
    g_slave_txbuff[4] = 0xAA;
    g_slave_txbuff[5] = 0x5A;
    g_slave_txbuff[6] = 0xA5;
    g_slave_txbuff[7] = 0x5A;
    g_slave_txbuff[8] = 0xA5;
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 8U);
    if (g_slave_rxbuff[0] == kSMBUS_Read64)
    {
        PRINTF("Send 64 cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send 64 cmd failed.\r\n");
    }

    g_slave_txbuff[1] = 0x55;
    g_slave_txbuff[2] = 0xAA;
    g_slave_txbuff[3] = 0x55;
    g_slave_txbuff[4] = 0xAA;
    g_slave_txbuff[5] = 0x5A;
    g_slave_txbuff[6] = 0xA5;
    g_slave_txbuff[7] = 0x5A;
    g_slave_txbuff[8] = 0xA5;
    g_slave_txbuff[9] = generate_crc8(g_slave_txbuff, 9U);
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 9U);
    if (g_slave_rxbuff[0] == kSMBUS_Read64)
    {
        PRINTF("Send 64 cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send 64 cmd with crc failed.\r\n");
    }

    memcpy(&g_slave_txbuff[1], txBlock, 19U);
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 19U);
    if (g_slave_rxbuff[0] == kSMBUS_ReadBlock)
    {
        PRINTF("Send block cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send block cmd failed.\r\n");
    }

    memcpy(&g_slave_txbuff[1], txBlock, 19U);
    g_slave_txbuff[20] = generate_crc8(&g_slave_txbuff[0], 20U);
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 20U);
    if (g_slave_rxbuff[0] == kSMBUS_ReadBlock)
    {
        PRINTF("Send block cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send block cmd with crc failed.\r\n");
    }

    memcpy(&g_slave_txbuff[1], txProcessCall, 4U);
    slave_transferData(g_slave_rxbuff, 5U, &g_slave_txbuff[1], 4U);
    if ((g_slave_rxbuff[0] == kSMBUS_ProcessCall) && (memcmp(txProcessCall, &g_slave_rxbuff[1], 4U) == 0U))
    {
        PRINTF("Process call cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Process call cmd failed.\r\n");
    }

    memcpy(&g_slave_txbuff[1], txProcessCall, 4U);
    g_slave_txbuff[5] = generate_crc8(&g_slave_txbuff[0], 5U);
    slave_transferData(g_slave_rxbuff, 5U, &g_slave_txbuff[1], 5U);
    if ((g_slave_rxbuff[0] == kSMBUS_ProcessCall) && (memcmp(txProcessCall, &g_slave_rxbuff[1], 4U) == 0U))
    {
        PRINTF("Process call cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Process call cmd with crc failed.\r\n");
    }
    g_slave_txbuff[1] = 9;
    memcpy(&g_slave_txbuff[2], txProcessCall, 9U);
    slave_transferData(g_slave_rxbuff, 11U, &g_slave_txbuff[1], 10U);
    if ((g_slave_rxbuff[0] == kSMBUS_BlockWriteBlockRead) && (g_slave_rxbuff[1] == 9U) &&
        (memcmp(txProcessCall, &g_slave_rxbuff[2], 9U) == 0U))
    {
        PRINTF("Process call block read/write cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Process call block read/write cmd failed.\r\n");
    }

    memcpy(&g_slave_txbuff[2], txProcessCall, 9U);
    g_slave_txbuff[11] = generate_crc8(&g_slave_txbuff[0], 11U);
    slave_transferData(g_slave_rxbuff, 11U, &g_slave_txbuff[1], 11U);
    if ((g_slave_rxbuff[0] == kSMBUS_BlockWriteBlockRead) && (g_slave_rxbuff[1] == 9U) &&
        (memcmp(txProcessCall, &g_slave_rxbuff[2], 9U) == 0U))
    {
        PRINTF("Process call block read/write cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Process call block read/write cmd with crc failed.\r\n");
    }

    I2C_SlaveGetDefaultConfig(&slaveConfig);

    /* Change the slave address */
    slaveConfig.address0.address = 0x61U;

    /* Initialize the I2C slave peripheral */
    I2C_SlaveInit(EXAMPLE_I2C_SLAVE, &slaveConfig, I2C_SLAVE_CLOCK_FREQUENCY);

    memset(g_slave_rxbuff, 0, sizeof(g_slave_rxbuff));
    memset(&g_s_gpdma_handle, 0, sizeof(g_s_gpdma_handle));

    GPDMA_CreateHandle(&g_gpdmaHandle, DEMO_DMA, EXAMPLE_I2C_SLAVE_CHANNEL);
    GPDMA_SetChannelDestinationPeripheral(DEMO_DMA, EXAMPLE_I2C_SLAVE_CHANNEL, kGpdmaRequestMuxI2cMasterSlv);
    GPDMA_SetChannelSourcePeripheral(DEMO_DMA, EXAMPLE_I2C_SLAVE_CHANNEL, kGpdmaRequestMuxI2cMasterSlv);

    I2C_SlaveTransferCreateHandleGPDMA(EXAMPLE_I2C_SLAVE, &g_s_gpdma_handle, i2c_slave_callback, &slaveXfer,
                                       &g_gpdmaHandle);

    g_slave_txbuff[0] = (0x61U << 1U) | 0x1U;
    memcpy(&g_slave_txbuff[1], deviceID, 18U);
    g_slave_txbuff[19] = generate_crc8(&g_slave_txbuff[0], 19U);
    slave_transferData(g_slave_rxbuff, 1U, &g_slave_txbuff[1], 19U);
    if ((g_slave_rxbuff[0] == 3U))
    {
        PRINTF("nGet device ID cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Get device ID cmd with crc failed.\r\n");
    }
}

static void i2c_master_gpdma_callback(I2C_Type *base,
                                      i2c_master_gpdma_handle_t *handle,
                                      status_t status,
                                      void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        g_CompletionFlag = true;
    }
}

static status_t master_transferData(void *rxData, uint32_t rxSize, void *txData, uint32_t txSize)
{
    i2c_master_transfer_t masterXfer;
    uint8_t iChoice = 0;

    PRINTF("Provide any one character input and press enter \r\n");
    APP_WaitString(&iChoice, 1U);

    if (rxData != NULL)
    {
        masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
        masterXfer.direction      = kI2C_Read;
        masterXfer.subaddress     = 0U;
        masterXfer.subaddressSize = 0U;
        masterXfer.data           = rxData;
        masterXfer.dataSize       = rxSize;
        masterXfer.flags          = kI2C_TransferDefaultFlag;
        g_CompletionFlag          = false;

        I2C_MasterTransferGPDMA(EXAMPLE_I2C_MASTER, &g_m_gpdma_handle, &masterXfer);

        /* Wait for master receive completed.*/
        while (!g_CompletionFlag)
        {
        }
        g_CompletionFlag = false;
    }

    if (txData != NULL)
    {
        masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
        masterXfer.direction      = kI2C_Write;
        masterXfer.subaddress     = 0U;
        masterXfer.subaddressSize = 0U;
        masterXfer.data           = txData;
        masterXfer.dataSize       = txSize;
        masterXfer.flags          = kI2C_TransferDefaultFlag;
        g_CompletionFlag          = false;
        I2C_MasterTransferGPDMA(EXAMPLE_I2C_MASTER, &g_m_gpdma_handle, &masterXfer);

        /* Wait for master receive completed.*/
        while (!g_CompletionFlag)
        {
        }
        g_CompletionFlag = false;
    }

    return kStatus_Success;
}

void i2s_master_smbus_slave(void)
{
    i2c_master_config_t masterConfig;

    GPDMA_Init(DEMO_DMA);

    /*
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.enableMaster = true;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);
    g_master_rxbuff[0] = I2C_MASTER_SLAVE_ADDR_7BIT << 1U;
    g_master_txbuff[0] = (I2C_MASTER_SLAVE_ADDR_7BIT << 1U) | 0x1U;
    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = I2C_BAUDRATE;

    /* Initialize the I2C master peripheral */
    I2C_MasterInit(EXAMPLE_I2C_MASTER, &masterConfig, I2C_MASTER_CLOCK_FREQUENCY);

    GPDMA_CreateHandle(&g_gpdmaHandle, DEMO_DMA, EXAMPLE_I2C_MASTER_CHANNEL);

    I2C_MasterTransferCreateHandleGPDMA(EXAMPLE_I2C_MASTER, &g_m_gpdma_handle, i2c_master_gpdma_callback, NULL,
                                        &g_gpdmaHandle);

    GPDMA_SetChannelDestinationPeripheral(g_m_gpdma_handle.gpdmaHandle->base, g_m_gpdma_handle.gpdmaHandle->channel,
                                          kGpdmaRequestMuxI2cMasterMst);
    GPDMA_SetChannelSourcePeripheral(g_m_gpdma_handle.gpdmaHandle->base, g_m_gpdma_handle.gpdmaHandle->channel,
                                     kGpdmaRequestMuxI2cMasterMst);

    /* master send byte */
    master_transferData(&g_master_rxbuff[1], 1U, NULL, 0U);
    if (g_master_rxbuff[1] == 0xA5U)
    {
        PRINTF("Receive byte successfully .\r\n");
    }
    else
    {
        PRINTF("Receive byte failed .\r\n");
    }

    /* receive byte with crc */
    master_transferData(&g_master_rxbuff[1], 2U, NULL, 0U);
    if (verify_crc8(g_master_rxbuff, 2U, g_master_rxbuff[2]) == kStatus_Success)
    {
        PRINTF("Receive byte with CRC successfully .\r\n");
    }
    else
    {
        PRINTF("Receive byte with CRC failed .\r\n");
    }

    /* receive byte command */
    master_transferData(&g_master_rxbuff[1], 2U, NULL, 0U);
    if ((g_master_rxbuff[1U] == kSMBUS_WriteByte) && (g_master_rxbuff[2U] == 0x5A))
    {
        PRINTF("Receive byte command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive byte command failed .\r\n");
    }

    /* receive byte command with CRC */
    master_transferData(&g_master_rxbuff[1], 3U, NULL, 0U);
    if ((g_master_rxbuff[1U] == kSMBUS_WriteByte) && (g_master_rxbuff[2U] == 0x5A) &&
        (verify_crc8(g_master_rxbuff, 3U, g_master_rxbuff[3]) == kStatus_Success))
    {
        PRINTF("Receive byte command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive byte command with crc failed .\r\n");
    }

    /* receive WORD command */
    master_transferData(&g_master_rxbuff[1], 5U, NULL, 0U);
    if ((g_master_rxbuff[1U] == kSMBUS_WriteWord) && (g_master_rxbuff[2U] == 0x55) && (g_master_rxbuff[3U] == 0xAA) &&
        (g_master_rxbuff[4U] == 0x5A) && (g_master_rxbuff[5U] == 0xA5))
    {
        PRINTF("Receive word command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive word command failed .\r\n");
    }

    /* receive WORD command with CRC */
    master_transferData(&g_master_rxbuff[1], 6U, NULL, 0U);
    if ((g_master_rxbuff[1U] == kSMBUS_WriteWord) && (g_master_rxbuff[2U] == 0x55) && (g_master_rxbuff[3U] == 0xAA) &&
        (g_master_rxbuff[4U] == 0x5A) && (g_master_rxbuff[5U] == 0xA5) &&
        (verify_crc8(g_master_rxbuff, 6U, g_master_rxbuff[6]) == kStatus_Success))
    {
        PRINTF("Receive word command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive word command with crc failed .\r\n");
    }

    /* receive 32 command */
    master_transferData(&g_master_rxbuff[1], 5U, NULL, 0U);
    if ((g_master_rxbuff[1U] == kSMBUS_Write32) && (g_master_rxbuff[2U] == 0x5A) && (g_master_rxbuff[3U] == 0xA5) &&
        (g_master_rxbuff[4U] == 0x55) && (g_master_rxbuff[5U] == 0xAA))
    {
        PRINTF("Receive 32 command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive 32 command failed .\r\n");
    }

    /* receive 32 command with CRC */
    master_transferData(&g_master_rxbuff[1], 6U, NULL, 0U);
    if ((g_master_rxbuff[1U] == kSMBUS_Write32) && (g_master_rxbuff[2U] == 0x5A) && (g_master_rxbuff[3U] == 0xA5) &&
        (g_master_rxbuff[4U] == 0x55) && (g_master_rxbuff[5U] == 0xAA) &&
        (verify_crc8(g_master_rxbuff, 6U, g_master_rxbuff[6]) == kStatus_Success))
    {
        PRINTF("Receive 32 command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive 32 command with crc failed .\r\n");
    }

    /* receive 64 command */
    master_transferData(&g_master_rxbuff[1], 9U, NULL, 0U);
    if (memcmp(cmpWriteWord64Buff, &g_master_rxbuff[1], 9U) == 0U)
    {
        PRINTF("Receive 64 command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive 64 command failed .\r\n");
    }

    /* receive 64 command with CRC */
    master_transferData(&g_master_rxbuff[1], 10U, NULL, 0U);
    if ((memcmp(cmpWriteWord64Buff, &g_master_rxbuff[1], 9U) == 0U) &&
        (verify_crc8(g_master_rxbuff, 10U, g_master_rxbuff[10]) == kStatus_Success))
    {
        PRINTF("Receive 64 command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive 64 command with crc failed .\r\n");
    }

    /* receive BLOCK command */
    master_transferData(&g_master_rxbuff[1], 18U, NULL, 0U);
    if (memcmp(cmpWriteWordBlockBuff, &g_master_rxbuff[1], 18U) == 0U)
    {
        PRINTF("Receive block command successfully .\r\n");
    }
    else
    {
        PRINTF("Receive block command failed .\r\n");
    }

    /* receive block command with CRC */
    master_transferData(&g_master_rxbuff[1], 19U, NULL, 0U);
    if ((memcmp(cmpWriteWordBlockBuff, &g_master_rxbuff[1], 18U) == 0U) &&
        (verify_crc8(g_master_rxbuff, 19U, g_master_rxbuff[19]) == kStatus_Success))
    {
        PRINTF("Receive block command with crc successfully .\r\n");
    }
    else
    {
        PRINTF("Receive block command with crc failed .\r\n");
    }

    /* send byte */
    g_master_txbuff[1] = 0xA5;
    master_transferData(NULL, 0U, &g_master_txbuff[1], 1U);
    PRINTF("Send byte successfully.\r\n");

    g_master_txbuff[1] = 0xA5;
    g_master_txbuff[2] = generate_crc8(g_master_txbuff, 2U);
    master_transferData(NULL, 0U, &g_master_txbuff[1], 2U);
    PRINTF("Send byte with crc successfully.\r\n");

    g_master_txbuff[1] = 0x5A;
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 1U);
    if (g_master_rxbuff[0] == kSMBUS_ReadByte)
    {
        PRINTF("Send byte cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send byte cmd failed.\r\n");
    }

    g_master_txbuff[1] = 0x5A;
    g_master_txbuff[2] = generate_crc8(g_master_txbuff, 2U);
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 2U);
    if (g_master_rxbuff[0] == kSMBUS_ReadByte)
    {
        PRINTF("Send byte cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send byte cmd with crc failed.\r\n");
    }

    g_master_txbuff[1] = 0xAA;
    g_master_txbuff[2] = 0x55;
    g_master_txbuff[3] = 0xA5;
    g_master_txbuff[4] = 0x5A;
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 4U);
    if (g_master_rxbuff[0] == kSMBUS_ReadWord)
    {
        PRINTF("Send word cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send word cmd failed.\r\n");
    }

    g_master_txbuff[1] = 0xAA;
    g_master_txbuff[2] = 0x55;
    g_master_txbuff[3] = 0xA5;
    g_master_txbuff[4] = 0x5A;
    g_master_txbuff[5] = generate_crc8(g_master_txbuff, 5U);
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 5U);
    if (g_master_rxbuff[0] == kSMBUS_ReadWord)
    {
        PRINTF("Send word cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send word cmd with crc failed.\r\n");
    }

    g_master_txbuff[1] = 0xA5;
    g_master_txbuff[2] = 0x5A;
    g_master_txbuff[3] = 0xAA;
    g_master_txbuff[4] = 0x55;
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 4U);
    if (g_master_rxbuff[0] == kSMBUS_Read32)
    {
        PRINTF("Send 32 cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send 32 cmd failed.\r\n");
    }

    g_master_txbuff[1] = 0xA5;
    g_master_txbuff[2] = 0x5A;
    g_master_txbuff[3] = 0xAA;
    g_master_txbuff[4] = 0x55;
    g_master_txbuff[5] = generate_crc8(g_master_txbuff, 5U);
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 5U);
    if (g_master_rxbuff[0] == kSMBUS_Read32)
    {
        PRINTF("Send 32 cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send 32 cmd with crc failed.\r\n");
    }

    g_master_txbuff[1] = 0x55;
    g_master_txbuff[2] = 0xAA;
    g_master_txbuff[3] = 0x55;
    g_master_txbuff[4] = 0xAA;
    g_master_txbuff[5] = 0x5A;
    g_master_txbuff[6] = 0xA5;
    g_master_txbuff[7] = 0x5A;
    g_master_txbuff[8] = 0xA5;
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 8U);
    if (g_master_rxbuff[0] == kSMBUS_Read64)
    {
        PRINTF("Send 64 cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send 64 cmd failed.\r\n");
    }

    g_master_txbuff[1] = 0x55;
    g_master_txbuff[2] = 0xAA;
    g_master_txbuff[3] = 0x55;
    g_master_txbuff[4] = 0xAA;
    g_master_txbuff[5] = 0x5A;
    g_master_txbuff[6] = 0xA5;
    g_master_txbuff[7] = 0x5A;
    g_master_txbuff[8] = 0xA5;
    g_master_txbuff[9] = generate_crc8(g_master_txbuff, 9U);
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 9U);
    if (g_master_rxbuff[0] == kSMBUS_Read64)
    {
        PRINTF("Send 64 cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send 64 cmd with crc failed.\r\n");
    }

    memcpy(&g_master_txbuff[1], txBlock, 19U);
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 19U);
    if (g_master_rxbuff[0] == kSMBUS_ReadBlock)
    {
        PRINTF("Send block cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Send block cmd failed.\r\n");
    }

    memcpy(&g_master_txbuff[1], txBlock, 19U);
    g_master_txbuff[20] = generate_crc8(&g_master_txbuff[0], 20U);
    master_transferData(g_master_rxbuff, 1U, &g_master_txbuff[1], 20U);
    if (g_master_rxbuff[0] == kSMBUS_ReadBlock)
    {
        PRINTF("Send block cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Send block cmd with crc failed.\r\n");
    }

    memcpy(&g_master_txbuff[1], txProcessCall, 4U);
    master_transferData(g_master_rxbuff, 5U, &g_master_txbuff[1], 4U);
    if ((g_master_rxbuff[0] == kSMBUS_ProcessCall) && (memcmp(txProcessCall, &g_master_rxbuff[1], 4U) == 0U))
    {
        PRINTF("Process call cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Process call cmd failed.\r\n");
    }

    memcpy(&g_master_txbuff[1], txProcessCall, 4U);
    g_master_txbuff[5] = generate_crc8(&g_master_txbuff[0], 5U);
    master_transferData(g_master_rxbuff, 5U, &g_master_txbuff[1], 5U);
    if ((g_master_rxbuff[0] == kSMBUS_ProcessCall) && (memcmp(txProcessCall, &g_master_rxbuff[1], 4U) == 0U))
    {
        PRINTF("Process call cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Process call cmd with crc failed.\r\n");
    }

    g_master_txbuff[1] = 9U;
    memcpy(&g_master_txbuff[2], txProcessCall, 9U);
    master_transferData(g_master_rxbuff, 11U, &g_master_txbuff[1], 10U);
    if ((g_master_rxbuff[0] == kSMBUS_BlockWriteBlockRead) && (g_master_rxbuff[1] == 9U) &&
        (memcmp(txProcessCall, &g_master_rxbuff[2], 9U) == 0U))
    {
        PRINTF("Process call block read/write cmd successfully.\r\n");
    }
    else
    {
        PRINTF("Process call block read/write cmd failed.\r\n");
    }

    memcpy(&g_master_txbuff[2], txProcessCall, 9U);
    g_master_txbuff[11] = generate_crc8(&g_master_txbuff[0], 11U);
    master_transferData(g_master_rxbuff, 11U, &g_master_txbuff[1], 11U);
    if ((g_master_rxbuff[0] == kSMBUS_BlockWriteBlockRead) && (g_master_rxbuff[1] == 9U) &&
        (memcmp(txProcessCall, &g_master_rxbuff[2], 9U) == 0U))
    {
        PRINTF("Process call block read/write cmd with crc successfully.\r\n");
    }
    else
    {
        PRINTF("Process call block read/write cmd with crc failed.\r\n");
    }
}
