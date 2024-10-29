/*
 * Copyright 2019, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "app.h"
#include "fsl_nor_flash.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* To avoid build error in case of some device do not define following macros. */
#ifndef EXAMPLE_DISABLE_CACHE_OF_FLASH
#define EXAMPLE_DISABLE_CACHE_OF_FLASH
#endif

#ifndef EXAMPLE_ENABLE_CACHE_OF_FLASH
#define EXAMPLE_ENABLE_CACHE_OF_FLASH
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * variables
 ******************************************************************************/

uint8_t mem_writeBuffer[FLASH_PAGE_SIZE];
uint8_t mem_readBuffer[FLASH_PAGE_SIZE] = {0};
extern nor_config_t norConfig;
nor_handle_t norHandle = {NULL};

extern status_t Nor_Flash_Initialization(nor_config_t *config, nor_handle_t *handle);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*Error trap function*/
void ErrorTrap(void)
{
    PRINTF("\n\rError Occured. Please check the configurations.\n\r");
    while (1)
    {
        ;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    status_t status;

    BOARD_InitHardware();

    status = Nor_Flash_Initialization(&norConfig, &norHandle);
    if (status != kStatus_Success)
    {
        PRINTF("\r\n***NOR Flash Initialization Failed!***\r\n");
        ErrorTrap();
    }

    PRINTF("\r\n***NOR Flash Component Demo Start!***\r\n");

#if !(defined(XIP_EXTERNAL_FLASH))
    /* Erase whole chip */
    PRINTF("\r\n***NOR Flash Erase Chip Start!***\r\n");
    status = Nor_Flash_Erase_Chip(&norHandle);
    if (status != kStatus_Success)
    {
        PRINTF("\r\n***NOR Flash Erase Chip Failed!***\r\n");
        ErrorTrap();
    }
#endif

    for (uint32_t pageIndex = 0; pageIndex < (norHandle.bytesInSectorSize / norHandle.bytesInPageSize); pageIndex++)
    {
        uint32_t address = NOR_FLASH_START_ADDRESS + norHandle.bytesInPageSize * pageIndex;

        /* Erase Sector */
        EXAMPLE_DISABLE_CACHE_OF_FLASH;
        status = Nor_Flash_Erase_Sector(&norHandle, address);
        EXAMPLE_ENABLE_CACHE_OF_FLASH;
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Erase Sector Failed!***\r\n");
            ErrorTrap();
        }

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
        DCACHE_InvalidateByRange(EXAMPLE_FLEXSPI_AMBA_BASE + address, norHandle.bytesInPageSize);
#endif

        status = Nor_Flash_Read(&norHandle, address, mem_readBuffer, norHandle.bytesInPageSize);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Read Page Failed!***\r\n");
            ErrorTrap();
        }

        for (uint32_t bytesIndex = 0; bytesIndex < norHandle.bytesInPageSize; bytesIndex++)
        {
            if (mem_readBuffer[bytesIndex] != 0xFF)
            {
                PRINTF("\r\n***NOR Flash Erase Chip Failed!***\r\n");
                ErrorTrap();
            }
        }

        /* Program the page data. */
        /* Initialize the write buffers. */
        for (uint32_t i = 0; i < norHandle.bytesInPageSize; i++)
        {
            mem_writeBuffer[i] = i;
        }

        PRINTF("\r\n");

        EXAMPLE_DISABLE_CACHE_OF_FLASH;
        status = Nor_Flash_Page_Program(&norHandle, address, mem_writeBuffer);
        EXAMPLE_ENABLE_CACHE_OF_FLASH;
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Page %d Program Failed!***\r\n", pageIndex);
            ErrorTrap();
        }

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
        DCACHE_InvalidateByRange(EXAMPLE_FLEXSPI_AMBA_BASE + address, norHandle.bytesInPageSize);
#endif

        /* Read page data and check if the data read is equal to the data programed. */
        status = Nor_Flash_Read(&norHandle, address, mem_readBuffer, norHandle.bytesInPageSize);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Page %d Read Failed!***\r\n", pageIndex);
            ErrorTrap();
        }

        if (memcmp(mem_writeBuffer, mem_readBuffer, norHandle.bytesInPageSize) != 0)
        {
            PRINTF("\r\n***NOR Flash Page %d Read/Write Failed!***\r\n", pageIndex);
            ErrorTrap();
        }

        PRINTF("\r\n***NOR Flash Page %d Read/Write Success!***\r\n", pageIndex);

        /* Erase Sector */
        EXAMPLE_DISABLE_CACHE_OF_FLASH;
        status = Nor_Flash_Erase_Sector(&norHandle, address);
        EXAMPLE_ENABLE_CACHE_OF_FLASH;
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Erase Sector Failed!***\r\n");
            ErrorTrap();
        }

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
        DCACHE_InvalidateByRange(EXAMPLE_FLEXSPI_AMBA_BASE + address, norHandle.bytesInPageSize);
#endif

        status = Nor_Flash_Read(&norHandle, address, mem_readBuffer, norHandle.bytesInPageSize);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Page Read Failed!***\r\n");
            ErrorTrap();
        }

        for (uint32_t bytesIndex = 0; bytesIndex < norHandle.bytesInPageSize; bytesIndex++)
        {
            if (mem_readBuffer[bytesIndex] != 0xFF)
            {
                PRINTF("\r\n***NOR Flash Erase Block Failed!***\r\n");
                ErrorTrap();
            }
        }
    }

    PRINTF("\r\n***NOR Flash All Pages Read/Write Success!***\r\n");
    while (1)
    {
    }
}
