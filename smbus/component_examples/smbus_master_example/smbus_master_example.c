/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  SDK Included Files */
#include "board.h"
#include "app.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "fsl_i2c_gpdma.h"
#include "fsl_smbus.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int SMBUS_MASTER_SEND_BYTE(void);
int SMBUS_MASTER_RECIEVE_BYTE(void);
int SMBUS_MASTER_WRITE_BYTE(void);
int SMBUS_MASTER_READ_BYTE(void);
int SMBUS_MASTER_WRITE_WORD(void);
int SMBUS_MASTER_READ_WORD(void);
int SMBUS_MASTER_WRITE_32(void);
int SMBUS_MASTER_READ_32(void);
int SMBUS_MASTER_WRITE_64(void);
int SMBUS_MASTER_READ_64(void);
int SMBUS_MASTER_WRITE_BLOCK(void);
int SMBUS_MASTER_READ_BLOCK(void);
int SMBUS_MASTER_PROCESS_CALL(void);
int SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL(void);
int SMBUS_MASTER_SEND_BYTE_WITH_PEC(void);
int SMBUS_MASTER_RECIEVE_BYTE_WITH_PEC(void);
int SMBUS_MASTER_WRITE_BYTE_WITH_PEC(void);
int SMBUS_MASTER_READ_BYTE_WITH_PEC(void);
int SMBUS_MASTER_WRITE_WORD_WITH_PEC(void);
int SMBUS_MASTER_READ_WORD_WITH_PEC(void);
int SMBUS_MASTER_WRITE_32_WITH_PEC(void);
int SMBUS_MASTER_READ_32_WITH_PEC(void);
int SMBUS_MASTER_WRITE_64_WITH_PEC(void);
int SMBUS_MASTER_READ_64_WITH_PEC(void);
int SMBUS_MASTER_WRITE_BLOCK_WITH_PEC(void);
int SMBUS_MASTER_READ_BLOCK_WITH_PEC(void);
int SMBUS_MASTER_PROCESS_CALL_WITH_PEC(void);
int SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC(void);
int SMBUS_MASTER_GET_DEVICE_ID(void);
int SMBUS_SLAVE_SEND_BYTE(void);
int SMBUS_SLAVE_RECIEVE_BYTE(void);
int SMBUS_SLAVE_WRITE_BYTE(void);
int SMBUS_SLAVE_READ_BYTE(void);
int SMBUS_SLAVE_WRITE_WORD(void);
int SMBUS_SLAVE_READ_WORD(void);
int SMBUS_SLAVE_WRITE_32(void);
int SMBUS_SLAVE_READ_32(void);
int SMBUS_SLAVE_WRITE_64(void);
int SMBUS_SLAVE_READ_64(void);
int SMBUS_SLAVE_WRITE_BLOCK(void);
int SMBUS_SLAVE_READ_BLOCK(void);
int SMBUS_SLAVE_PROCESS_CALL(void);
int SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL(void);
int SMBUS_SLAVE_SEND_BYTE_WITH_PEC(void);
int SMBUS_SLAVE_RECIEVE_BYTE_WITH_PEC(void);
int SMBUS_SLAVE_WRITE_BYTE_WITH_PEC(void);
int SMBUS_SLAVE_READ_BYTE_WITH_PEC(void);
int SMBUS_SLAVE_WRITE_WORD_WITH_PEC(void);
int SMBUS_SLAVE_READ_WORD_WITH_PEC(void);
int SMBUS_SLAVE_WRITE_32_WITH_PEC(void);
int SMBUS_SLAVE_READ_32_WITH_PEC(void);
int SMBUS_SLAVE_WRITE_64_WITH_PEC(void);
int SMBUS_SLAVE_READ_64_WITH_PEC(void);
int SMBUS_SLAVE_WRITE_BLOCK_WITH_PEC(void);
int SMBUS_SLAVE_READ_BLOCK_WITH_PEC(void);
int SMBUS_SLAVE_PROCESS_CALL_WITH_PEC(void);
int SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC(void);
static void SMBUS_MASTER_SEND_RECEIVE(void);
static void SMBUS_SLAVE_SEND_RECEIVE(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
smbus_master_config_t smbusMasterConfig;
smbus_master_handle_t smbusMasterHandle;
smbus_slave_config_t smbusSlaveConfig;
smbus_slave_handle_t smbusSlaveHandle;
uint8_t iChoice = 0;
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
int main(void)
{
    BOARD_InitHardware();

    SMBUS_MASTER_SEND_RECEIVE();
    SMBUS_SLAVE_SEND_RECEIVE();

    while (1)
    {
    }
}

static void SMBUS_MASTER_SEND_RECEIVE(void)
{
    int errors = 0;

    smbusMasterConfig.portConfig.i2cBase     = EXAMPLE_I2C_MASTER;
    smbusMasterConfig.portConfig.srcClock_Hz = I2C_MASTER_CLOCK_FREQUENCY;
    smbusMasterConfig.portConfig.dmaBase     = DEMO_DMA;
    smbusMasterConfig.portConfig.dmaChannel  = EXAMPLE_I2C_MASTER_CHANNEL;
    smbusMasterConfig.baudRate_Bps           = I2C_BAUDRATE;
    smbusMasterConfig.timeoutMS              = DEMO_SMBUS_MASTER_TRANSFER_TIMEOUT;
    SMBUS_Master_Init(&smbusMasterHandle, &smbusMasterConfig);

    errors += SMBUS_MASTER_SEND_BYTE();
    errors += SMBUS_MASTER_SEND_BYTE_WITH_PEC();
    errors += SMBUS_MASTER_WRITE_BYTE();
    errors += SMBUS_MASTER_WRITE_BYTE_WITH_PEC();
    errors += SMBUS_MASTER_WRITE_WORD();
    errors += SMBUS_MASTER_WRITE_WORD_WITH_PEC();
    errors += SMBUS_MASTER_WRITE_32();
    errors += SMBUS_MASTER_WRITE_32_WITH_PEC();
    errors += SMBUS_MASTER_WRITE_64();
    errors += SMBUS_MASTER_WRITE_64_WITH_PEC();
    errors += SMBUS_MASTER_WRITE_BLOCK();
    errors += SMBUS_MASTER_WRITE_BLOCK_WITH_PEC();
    errors += SMBUS_MASTER_RECIEVE_BYTE();
    errors += SMBUS_MASTER_RECIEVE_BYTE_WITH_PEC();
    errors += SMBUS_MASTER_READ_BYTE();
    errors += SMBUS_MASTER_READ_BYTE_WITH_PEC();
    errors += SMBUS_MASTER_READ_WORD();
    errors += SMBUS_MASTER_READ_WORD_WITH_PEC();
    errors += SMBUS_MASTER_READ_32();
    errors += SMBUS_MASTER_READ_32_WITH_PEC();
    errors += SMBUS_MASTER_READ_64();
    errors += SMBUS_MASTER_READ_64_WITH_PEC();
    errors += SMBUS_MASTER_READ_BLOCK();
    errors += SMBUS_MASTER_READ_BLOCK_WITH_PEC();

    errors += SMBUS_MASTER_PROCESS_CALL();
    errors += SMBUS_MASTER_PROCESS_CALL_WITH_PEC();

    errors += SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL();
    errors += SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC();

    errors += SMBUS_MASTER_GET_DEVICE_ID();

    SMBUS_Master_Deinit(&smbusMasterHandle);

    if (errors > 0)
    {
        PRINTF("\r\nProject failed with %d errors\r\n", errors);
    }
    else
    {
        PRINTF("\r\nProject success\r\n");
    }
}

static void SMBUS_SLAVE_SEND_RECEIVE(void)
{
    int errors = 0;

    smbusSlaveConfig.slaveAddress           = I2C_MASTER_SLAVE_ADDR_7BIT;
    smbusSlaveConfig.portConfig.i2cBase     = EXAMPLE_I2C_SLAVE;
    smbusSlaveConfig.portConfig.srcClock_Hz = I2C_SLAVE_CLOCK_FREQUENCY;
    smbusSlaveConfig.portConfig.dmaBase     = DEMO_DMA;
    smbusSlaveConfig.portConfig.dmaChannel  = EXAMPLE_I2C_SLAVE_CHANNEL;
    smbusSlaveConfig.timeoutMS              = DEMO_SMBUS_SLAVE_TRANSFER_TIMEOUT;

    SMBUS_Slave_Init(&smbusSlaveHandle, &smbusSlaveConfig);

    errors += SMBUS_SLAVE_SEND_BYTE();
    errors += SMBUS_SLAVE_SEND_BYTE_WITH_PEC();
    errors += SMBUS_SLAVE_WRITE_BYTE();
    errors += SMBUS_SLAVE_WRITE_BYTE_WITH_PEC();
    errors += SMBUS_SLAVE_WRITE_WORD();
    errors += SMBUS_SLAVE_WRITE_WORD_WITH_PEC();
    errors += SMBUS_SLAVE_WRITE_32();
    errors += SMBUS_SLAVE_WRITE_32_WITH_PEC();
    errors += SMBUS_SLAVE_WRITE_64();
    errors += SMBUS_SLAVE_WRITE_64_WITH_PEC();
    errors += SMBUS_SLAVE_WRITE_BLOCK();
    errors += SMBUS_SLAVE_WRITE_BLOCK_WITH_PEC();
    errors += SMBUS_SLAVE_RECIEVE_BYTE();
    errors += SMBUS_SLAVE_RECIEVE_BYTE_WITH_PEC();
    errors += SMBUS_SLAVE_READ_BYTE();
    errors += SMBUS_SLAVE_READ_BYTE_WITH_PEC();
    errors += SMBUS_SLAVE_READ_WORD();
    errors += SMBUS_SLAVE_READ_WORD_WITH_PEC();
    errors += SMBUS_SLAVE_READ_32();
    errors += SMBUS_SLAVE_READ_32_WITH_PEC();
    errors += SMBUS_SLAVE_READ_64();
    errors += SMBUS_SLAVE_READ_64_WITH_PEC();
    errors += SMBUS_SLAVE_READ_BLOCK();
    errors += SMBUS_SLAVE_READ_BLOCK_WITH_PEC();
    errors += SMBUS_SLAVE_PROCESS_CALL();
    errors += SMBUS_SLAVE_PROCESS_CALL_WITH_PEC();
    errors += SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL();
    errors += SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC();

    SMBUS_Slave_Deinit(&smbusSlaveHandle);

    if (errors > 0)
    {
        PRINTF("Project failed with %d errors\r\n", errors);
    }
    else
    {
        PRINTF("Project success\r\n");
    }
}

int SMBUS_MASTER_SEND_BYTE(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite = 0xA5;

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 1U;
        transfer.writeBuffer      = &byteToWrite;

        if (kStatus_Success == SMBUS_Master_WriteByte(&smbusMasterHandle, &transfer))
        {
            PRINTF("SMBUS_MASTER_SEND_BYTE PASS!\r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_SEND_BYTE FAIL!\r\n");
        }
    } while (0);

    return ret;
}

int SMBUS_MASTER_RECIEVE_BYTE(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToReceive;

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);
    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 1U;
        transfer.readBuffer       = &byteToReceive;

        if (kStatus_Success == SMBUS_Master_ReadByte(&smbusMasterHandle, &transfer))
        {
            if (byteToReceive == 0x5A)
            {
                PRINTF("SMBUS_MASTER_RECIEVE_BYTE PASS!\r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_RECIEVE_BYTE FAIL!\r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_RECIEVE_BYTE FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_BYTE(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite = 0x5A;

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = &byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 1U;

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_WriteByte))
        {
            PRINTF("SMBUS_MASTER_WRITE_BYTE PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_BYTE FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_BYTE(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead;

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = &byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 1U;

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_ReadByte))
        {
            if (byteToRead == 0x5A)
            {
                PRINTF("SMBUS_MASTER_READ_BYTE PASS!\r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_BYTE FAIL!\r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_BYTE FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_WORD(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x55, 0xAA, 0x5A, 0xA5};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);
    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_WriteWord))
        {
            PRINTF("SMBUS_MASTER_WRITE_WORD PASS!\r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_WORD FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_WORD(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead[4]    = {0};
    uint8_t byteToReadCmp[4] = {0xAA, 0x55, 0xA5, 0x5A};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_ReadWord))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_MASTER_READ_WORD PASS!\r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_WORD FAIL!\r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_WORD FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_32(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x5A, 0xA5, 0x55, 0xAA};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_Write32))
        {
            PRINTF("SMBUS_MASTER_WRITE_32 PASS!\r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_32 FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_32(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead[4]    = {0};
    uint8_t byteToReadCmp[4] = {0xA5, 0x5A, 0xAA, 0x55};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_Read32))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_MASTER_READ_32 PASS!\r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_32 FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_32 FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_64(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[8] = {0x55, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5, 0x55, 0xAA};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 8U;

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_Write64))
        {
            PRINTF("SMBUS_MASTER_WRITE_64 PASS!\r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_64 FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_64(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead[8]    = {0};
    uint8_t byteToReadCmp[8] = {0x55, 0xAA, 0x55, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 8U;

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_Read64))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_MASTER_READ_64 PASS!\r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_64 FAIL!\r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_64 FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_BLOCK(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_WriteBlock))
        {
            PRINTF("SMBUS_MASTER_WRITE_BLOCK PASS!\r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_BLOCK FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_BLOCK(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead[18]    = {0};
    uint8_t byteToReadCmp[18] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = sizeof(byteToRead);

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_ReadBlock))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_MASTER_READ_BLOCK PASS!\r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_BLOCK FAIL!\r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_BLOCK FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_PROCESS_CALL(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x00, 0x01, 0x02, 0x03};
    uint8_t byteToRead[4]  = {0};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Master_ProcessCall(&smbusMasterHandle, &transfer, kSMBUS_ProcessCall))
        {
            if (!(memcmp(byteToWrite, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_MASTER_PROCESS_CALL PASS!\r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_PROCESS_CALL FAIL!\r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_PROCESS_CALL FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[9] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t byteToRead[10] = {0};

    PRINTF("Provide any one character input and press enter\r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Master_ProcessCall(&smbusMasterHandle, &transfer, kSMBUS_BlockWriteBlockRead))
        {
            if (!(memcmp(byteToWrite, &byteToRead[0], sizeof(byteToWrite))))
            {
                PRINTF("SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL PASS!\r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL FAIL!\r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL FAIL!\r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_SEND_BYTE_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToSend = 0xA5U;
    ;

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 1U;
        transfer.writeBuffer      = &byteToSend;
        if (kStatus_Success == SMBUS_Master_WriteByte(&smbusMasterHandle, &transfer))
        {
            PRINTF("SMBUS_MASTER_SEND_BYTE_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_SEND_BYTE_WITH_PEC FAIL! \r\n");
        }
    } while (0);

    return ret;
}
int SMBUS_MASTER_RECIEVE_BYTE_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToReceive;

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);
    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.needPEC          = 0x01; /* With PEC Byte*/
        transfer.transferByteSize = 1U;
        transfer.readBuffer       = &byteToReceive;

        if (kStatus_Success == SMBUS_Master_ReadByte(&smbusMasterHandle, &transfer))
        {
            if (byteToReceive == 0x5A)
            {
                PRINTF("SMBUS_MASTER_RECIEVE_BYTE_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_RECIEVE_BYTE_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_RECIEVE_BYT_WITH_PECE FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_BYTE_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite = 0x5A;

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = &byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 1U;

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_WriteByte))
        {
            PRINTF("SMBUS_MASTER_WRITE_BYTE_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_BYTE_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_BYTE_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead;

    PRINTF(" Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = &byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 1U;

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_ReadByte))
        {
            if (byteToRead == 0x5A)
            {
                PRINTF("SMBUS_MASTER_READ_BYTE_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_BYTE_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_BYTE_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_WORD_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x55, 0xAA, 0x5A, 0xA5};

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);
    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_WriteWord))
        {
            PRINTF("SMBUS_MASTER_WRITE_WORD_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_WORD_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_WORD_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead[4]    = {0};
    uint8_t byteToReadCmp[4] = {0xAA, 0x55, 0xA5, 0x5A};

    PRINTF(" Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_ReadWord))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_MASTER_READ_WORD_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_WORD_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_WORD_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_32_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x5A, 0xA5, 0x55, 0xAA};

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_Write32))
        {
            PRINTF("SMBUS_MASTER_WRITE_32_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_32_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_32_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead[4]    = {0};
    uint8_t byteToReadCmp[4] = {0xA5, 0x5A, 0xAA, 0x55};

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_Read32))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_MASTER_READ_32_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_32_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_32_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_64_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[8] = {0x55, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5, 0x55, 0xAA};

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 8U;

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_Write64))
        {
            PRINTF("SMBUS_MASTER_WRITE_64_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_64_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_64_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead[8]    = {0};
    uint8_t byteToReadCmp[8] = {0x55, 0xAA, 0x55, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5};

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 8U;

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_Read64))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_MASTER_READ_64_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_64_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_64_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_WRITE_BLOCK_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Master_Write(&smbusMasterHandle, &transfer, kSMBUS_WriteBlock))
        {
            PRINTF("SMBUS_MASTER_WRITE_BLOCK_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_MASTER_WRITE_BLOCK_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_READ_BLOCK_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToRead[18]    = {0};
    uint8_t byteToReadCmp[18] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11};

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = sizeof(byteToRead);

        if (kStatus_Success == SMBUS_Master_Read(&smbusMasterHandle, &transfer, kSMBUS_ReadBlock))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_MASTER_READ_BLOCK_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_READ_BLOCK_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_READ_BLOCK_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_PROCESS_CALL_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x00, 0x01, 0x02, 0x03};
    uint8_t byteToRead[5]  = {0};

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Master_ProcessCall(&smbusMasterHandle, &transfer, kSMBUS_ProcessCall))
        {
            if (!(memcmp(byteToWrite, byteToRead, sizeof(byteToWrite))))
            {
                PRINTF("SMBUS_MASTER_PROCESS_CALL_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("MBUS_MASTER_PROCESS_CALL_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_PROCESS_CALL_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC(void)
{
    int ret = 1;
    smbus_master_transfer_t transfer;
    uint8_t byteToWrite[9] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t byteToRead[11] = {0};

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        transfer.slaveAddress     = I2C_MASTER_SLAVE_ADDR_7BIT;
        transfer.writeBuffer      = byteToWrite;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Master_ProcessCall(&smbusMasterHandle, &transfer, kSMBUS_BlockWriteBlockRead))
        {
            if (!(memcmp(byteToWrite, &byteToRead[0], sizeof(byteToWrite))))
            {
                PRINTF("SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_MASTER_GET_DEVICE_ID(void)
{
    int ret                 = 1;
    uint8_t deviceID[18]    = {0x00};
    uint8_t ByteToCheck[16] = {
        0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, /* UID */
    };

    PRINTF("Provide any one character input and press enter \r\n");

    APP_WaitString(&iChoice, 1U);

    do
    {
        if (kStatus_Success == SMBUS_Master_GetDeviceID(&smbusMasterHandle, deviceID))
        {
            if (!(memcmp(ByteToCheck, deviceID, sizeof(ByteToCheck))))
            {
                PRINTF("SMBUS_MASTER_GET_DEVICE_ID PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_MASTER_GET_DEVICE_ID FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_MASTER_GET_DEVICE_ID FAIL! \r\n");
        }
    } while (0);
    return ret;
}

int SMBUS_SLAVE_SEND_BYTE(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite = 0xA5;

    do
    {
        transfer.needPEC          = false;
        transfer.writeBuffer      = &byteToWrite;
        transfer.transferByteSize = 1U;
        if (kStatus_Success == SMBUS_Slave_WriteByte(&smbusSlaveHandle, &transfer))
        {
            PRINTF("SMBUS_SLAVE_SEND_BYTE PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_SEND_BYTE FAIL! \r\n");
        }
    } while (0);

    return ret;
}

int SMBUS_SLAVE_RECIEVE_BYTE(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToReceive = 0U;

    do
    {
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.readBuffer       = &byteToReceive;
        transfer.transferByteSize = 1U;
        if (kStatus_Success == SMBUS_Slave_ReadByte(&smbusSlaveHandle, &transfer))
        {
            if (byteToReceive == 0xA5)
            {
                PRINTF("SMBUS_SLAVE_RECIEVE_BYTE PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_RECIEVE_BYTE FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_RECIEVE_BYTE FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_WRITE_BYTE(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite = 0x5A;

    do
    {
        transfer.writeBuffer      = &byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 1U;
        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_WriteByte))
        {
            PRINTF("SMBUS_SLAVE_WRITE_BYTE PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_BYTE FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_READ_BYTE(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead;

    do
    {
        transfer.readBuffer       = &byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 1U;
        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_ReadByte))
        {
            if (byteToRead == 0x5A)
            {
                PRINTF("SMBUS_SLAVE_READ_BYTE PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_BYTE FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_BYTE FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_WRITE_WORD(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x55, 0xAA, 0x5A, 0xA5};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 4U;
        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_WriteWord))
        {
            PRINTF("SMBUS_SLAVE_WRITE_WORD PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_WORD FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_READ_WORD(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead[4]    = {0};
    uint8_t byteToReadCmp[4] = {0xAA, 0x55, 0xA5, 0x5A};

    do
    {
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_ReadWord))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_SLAVE_READ_WORD PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_WORD FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_WORD FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_WRITE_32(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x5A, 0xA5, 0x55, 0xAA};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_Write32))
        {
            PRINTF("SMBUS_SLAVE_WRITE_32 PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_32 FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_READ_32(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead[4]    = {0};
    uint8_t byteToReadCmp[4] = {0xA5, 0x5A, 0xAA, 0x55};

    do
    {
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_Read32))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_SLAVE_READ_32 PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_32 FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_32 FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_WRITE_64(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[8] = {0x55, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5, 0x55, 0xAA};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 8U;

        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_Write64))
        {
            PRINTF("SMBUS_SLAVE_WRITE_64 PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_64 FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_READ_64(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead[8]    = {0};
    uint8_t byteToReadCmp[8] = {0x55, 0xAA, 0x55, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5};

    do
    {
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = 8U;

        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_Read64))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_SLAVE_READ_64 PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_64 FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_64 FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_WRITE_BLOCK(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_WriteBlock))
        {
            PRINTF("SMBUS_SLAVE_WRITE_BLOCK PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_BLOCK FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_READ_BLOCK(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead[18]    = {0};
    uint8_t byteToReadCmp[18] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11};

    do
    {
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = sizeof(byteToRead);

        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_ReadBlock))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_SLAVE_READ_BLOCK PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_BLOCK FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_BLOCK FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_PROCESS_CALL(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x00, 0x01, 0x02, 0x03};
    uint8_t byteToRead[4]  = {0};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Slave_ProcessCall(&smbusSlaveHandle, &transfer, kSMBUS_ProcessCall))
        {
            if (!(memcmp(byteToWrite, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_SLAVE_PROCESS_CALL PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_PROCESS_CALL FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_PROCESS_CALL FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[9] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t byteToRead[10] = {0};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = false; /* Without PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Slave_ProcessCall(&smbusSlaveHandle, &transfer, kSMBUS_BlockWriteBlockRead))
        {
            if (!(memcmp(byteToWrite, &byteToRead[0], sizeof(byteToWrite))))
            {
                PRINTF("SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL FAIL! \r\n");
        }
    } while (0);

    return ret;
}

int SMBUS_SLAVE_SEND_BYTE_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToSend = 0xA5;

    do
    {
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 1U;
        transfer.writeBuffer      = &byteToSend;

        if (kStatus_Success == SMBUS_Slave_WriteByte(&smbusSlaveHandle, &transfer))
        {
            PRINTF("SMBUS_SLAVE_SEND_BYTE_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_SEND_BYTE_WITH_PEC FAIL! \r\n");
        }
    } while (0);
    return ret;
}
int SMBUS_SLAVE_RECIEVE_BYTE_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToReceive;

    do
    {
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 1U;
        transfer.readBuffer       = &byteToReceive;
        if (kStatus_Success == SMBUS_Slave_ReadByte(&smbusSlaveHandle, &transfer))
        {
            if (byteToReceive == 0xA5)
            {
                PRINTF("SMBUS_SLAVE_RECIEVE_BYTE_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_RECIEVE_BYTE_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_RECIEVE_BYTE_WITH_PEC FAIL! \r\n");
        }
    } while (0);
    return ret;
}
int SMBUS_SLAVE_WRITE_BYTE_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite = 0x5A;

    do
    {
        transfer.writeBuffer      = &byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 1U;

        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_WriteByte))
        {
            PRINTF("SMBUS_SLAVE_WRITE_BYTE_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_BYTE_WITH_PEC FAIL! \r\n");
        }
    } while (0);
    return ret;
}
int SMBUS_SLAVE_READ_BYTE_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead[2] = {0}; /* Extra byte for PEC*/

    do
    {
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 1U;

        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_ReadByte))
        {
            if (byteToRead[0] == 0x5A)
            {
                PRINTF("SMBUS_SLAVE_READ_BYTE_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_BYTE_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_BYTE_WITH_PEC FAIL! \r\n");
        }
    } while (0);
    return ret;
}
int SMBUS_SLAVE_WRITE_WORD_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x55, 0xAA, 0x5A, 0xA5};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_WriteWord))
        {
            PRINTF("SMBUS_SLAVE_WRITE_WORD_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_WORD_WITH_PEC FAIL! \r\n");
        }
    } while (0);
    return ret;
}
int SMBUS_SLAVE_READ_WORD_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead[4]    = {0};
    uint8_t byteToReadCmp[5] = {0xAA, 0x55, 0xA5, 0x5A}; /* Extra byte for PEC*/

    do
    {
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_ReadWord))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_SLAVE_READ_WORD_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_WORD_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_WORD_WITH_PEC FAIL! \r\n");
        }
    } while (0);
    return ret;
}
int SMBUS_SLAVE_WRITE_32_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x5A, 0xA5, 0x55, 0xAA};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_Write32))
        {
            PRINTF("SMBUS_SLAVE_WRITE_32_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_32_WITH_PEC FAIL! \r\n");
        }
    } while (0);
    return ret;
}
int SMBUS_SLAVE_READ_32_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead[4]    = {0};
    uint8_t byteToReadCmp[5] = {0xA5, 0x5A, 0xAA, 0x55}; /* Extra byte for PEC*/

    do
    {
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 4U;

        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_Read32))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_SLAVE_READ_32_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_32_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_32_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_WRITE_64_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[8] = {0x55, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5, 0x55, 0xAA};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 8U;

        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_Write64))
        {
            PRINTF("SMBUS_SLAVE_WRITE_64_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_64_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_READ_64_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead[8]    = {0};
    uint8_t byteToReadCmp[9] = {0x55, 0xAA, 0x55, 0xAA, 0x5A, 0xA5, 0x5A, 0xA5}; /* Extra byte for PEC*/

    do
    {
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = 8U;

        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_Read64))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_SLAVE_READ_64_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_64_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_64_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_WRITE_BLOCK_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Slave_Write(&smbusSlaveHandle, &transfer, kSMBUS_WriteBlock))
        {
            PRINTF("SMBUS_SLAVE_WRITE_BLOCK_WITH_PEC PASS! \r\n");
            ret = 0;
        }
        else
        {
            PRINTF("SMBUS_SLAVE_WRITE_BLOCK_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_READ_BLOCK_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToRead[18]    = {0};
    uint8_t byteToReadCmp[20] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11}; /* Extra byte for PEC*/

    do
    {
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = sizeof(byteToRead);

        if (kStatus_Success == SMBUS_Slave_Read(&smbusSlaveHandle, &transfer, kSMBUS_ReadBlock))
        {
            if (!(memcmp(byteToReadCmp, byteToRead, sizeof(byteToRead))))
            {
                PRINTF("SMBUS_SLAVE_READ_BLOCK_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_READ_BLOCK_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_READ_BLOCK_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_PROCESS_CALL_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[4] = {0x00, 0x01, 0x02, 0x03};
    uint8_t byteToRead[5]  = {0}; /* Read one more byte with PEC*/

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Slave_ProcessCall(&smbusSlaveHandle, &transfer, kSMBUS_ProcessCall))
        {
            if (!(memcmp(byteToWrite, byteToRead, sizeof(byteToWrite))))
            {
                PRINTF("SMBUS_SLAVE_PROCESS_CALL_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_PROCESS_CALL_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_PROCESS_CALL_WITH_PEC FAIL! \r\n");
        }

    } while (0);

    return ret;
}

int SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC(void)
{
    int ret = 1;
    smbus_slave_transfer_t transfer;
    uint8_t byteToWrite[9] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t byteToRead[10] = {0}; /* Extra byte for PEC*/

    do
    {
        transfer.writeBuffer      = byteToWrite;
        transfer.readBuffer       = byteToRead;
        transfer.needPEC          = true; /* With PEC Byte*/
        transfer.transferByteSize = sizeof(byteToWrite);

        if (kStatus_Success == SMBUS_Slave_ProcessCall(&smbusSlaveHandle, &transfer, kSMBUS_BlockWriteBlockRead))
        {
            if (!(memcmp(byteToWrite, &byteToRead[0], sizeof(byteToWrite))))
            {
                PRINTF("SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC PASS! \r\n");
                ret = 0;
            }
            else
            {
                PRINTF("SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC FAIL! \r\n");
            }
        }
        else
        {
            PRINTF("SMBUS_SLAVE_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL_WITH_PEC FAIL! \r\n");
        }
    } while (0);

    return ret;
}
