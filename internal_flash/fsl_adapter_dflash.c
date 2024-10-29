/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!*********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "fsl_flash.h"
#include "fsl_adapter_flash.h"
/*****************************************************************************
 *****************************************************************************
 * Private macros
 *****************************************************************************
 *****************************************************************************/
#define PGM_SIZE_BYTE FSL_FEATURE_FLASH_PFLASH_BLOCK_WRITE_UNIT_SIZE
/*!*********************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
********************************************************************************** */
static flexnvm_config_t s_flexNvmDriver;
static uint32_t s_dflashBlockBase;
/*!*********************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
********************************************************************************** */
static hal_flash_status_t HAL_FlashProgramAdaptation(uint32_t dest, uint32_t size, uint8_t *pData);
/*!*********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

/*****************************************************************************
 *****************************************************************************
 * Private functions
 *****************************************************************************
 *****************************************************************************/
static hal_flash_status_t HAL_FlashGetStatus(status_t status)
{
#if (defined(HAL_FLASH_TRANSFER_MODE) && (HAL_FLASH_TRANSFER_MODE > 0U))
    hal_flash_status_t flashStatus = kStatus_HAL_Flash_Error;
    switch (status)
    {
        case (status_t)kStatus_Success:
            flashStatus = kStatus_HAL_Flash_Success;
            break;
        case (status_t)kStatus_FTFx_InvalidArgument:
            flashStatus = kStatus_HAL_Flash_InvalidArgument;
            break;
        case (status_t)kStatus_FTFx_AlignmentError:
            flashStatus = kStatus_HAL_Flash_AlignmentError;
            break;
        default:
            /*MISRA rule 16.4*/
            break;
    }
    return flashStatus;
#else
    return (hal_flash_status_t)status;
#endif
}

static hal_flash_status_t HAL_FlashProgramAdaptation(uint32_t dest, uint32_t size, uint8_t *pData)
{
    uint32_t progBuf[PGM_SIZE_BYTE / sizeof(uint32_t)];
    status_t status = (status_t)kStatus_HAL_Flash_Success;

    if (((size & ((uint8_t)PGM_SIZE_BYTE - 0x01U)) != 0U) || ((dest & ((uint8_t)PGM_SIZE_BYTE - 0x01U)) != 0U))
    {
        return kStatus_HAL_Flash_AlignmentError;
    }

    while (0U != size)
    {
        (void)memcpy((void *)((uint32_t *)&progBuf[0]), (const void *)pData, PGM_SIZE_BYTE);

        status = FLEXNVM_DflashProgram(&s_flexNvmDriver, s_dflashBlockBase + dest, (uint8_t *)progBuf, PGM_SIZE_BYTE);
        if ((status_t)kStatus_FLASH_Success != status)
        {
            break;
        }

        pData += PGM_SIZE_BYTE;
        dest += PGM_SIZE_BYTE;
        size -= PGM_SIZE_BYTE;
    }

    return HAL_FlashGetStatus(status);
}

/*!*********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */

/*!
 * @brief Initializes the global flash properties structure members.
 *
 * This function initializes the Flash module for the other Flash APIs.
 *
 *
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 * @retval #kStatus_HAL_Flash_InvalidArgument An invalid argument is provided.
 * @retval #kStatus_HAL_Flash_ExecuteInRamFunctionNotReady Execute-in-RAM function is not available.
 * @retval #kStatus_HAL_Flash_PartitionStatusUpdateFailure Failed to update the partition status.
 */
hal_flash_status_t HAL_FlashInit()
{
    static uint8_t nvmInit = 0U;
    status_t status        = (status_t)kStatus_HAL_Flash_Success;

    if (0U == nvmInit)
    {
        /* Init Flash */
        memset(&s_flexNvmDriver, 0x0, sizeof(flexnvm_config_t));
        status = FLEXNVM_Init(&s_flexNvmDriver);
        (void)FLEXNVM_GetProperty(&s_flexNvmDriver, kFLEXNVM_PropertyDflashBlockBaseAddr, &s_dflashBlockBase);
        s_dflashBlockBase = s_dflashBlockBase - (uint32_t)((uint32_t)FSL_FEATURE_FLASH_FLEX_NVM_BLOCK_COUNT *
                                                           (uint32_t)FSL_FEATURE_FLASH_FLEX_NVM_BLOCK_SIZE);
        nvmInit           = 1U;
    }
    return HAL_FlashGetStatus(status);
}

/*!
 * \brief  Verify erase data in Flash
 *
 * @param start           The address of the Flash location
 * @param lengthInBytes   The number of bytes to be checked
 * @param margin          Flash margin value
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 */
hal_flash_status_t HAL_FlashVerifyErase(uint32_t start, uint32_t lengthInBytes, hal_flash_margin_value_t margin)
{
    status_t status;
    uint32_t regPrimask = DisableGlobalIRQ();
    status              = FLEXNVM_DflashVerifyErase(&s_flexNvmDriver, s_dflashBlockBase + start, lengthInBytes,
                                                    (ftfx_margin_value_t)margin);
    EnableGlobalIRQ(regPrimask);
    return HAL_FlashGetStatus(status);
}

/*!
 * \brief  Write alligned data to FLASH
 *
 * @param dest            The address of the Flash location
 * @param size            The number of bytes to be programed
 * @param pData           Pointer to the data to be programmed to Flash
 *
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 *
 */
hal_flash_status_t HAL_FlashProgram(uint32_t dest, uint32_t size, uint8_t *pData)
{
    return HAL_FlashProgramAdaptation(dest, size, pData);
}

/*!
 * \brief  Write data to FLASH
 *
 * @param dest        The address of the Flash location
 * @param size        The number of bytes to be programed
 * @param pData       Pointer to the data to be programmed to Flash
 *
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 *
 */
hal_flash_status_t HAL_FlashProgramUnaligned(uint32_t dest, uint32_t size, uint8_t *pData)
{
    uint8_t buffer[PGM_SIZE_BYTE];
    uint16_t bytes            = (uint16_t)(dest & ((uint32_t)PGM_SIZE_BYTE - 1U));
    hal_flash_status_t status = kStatus_HAL_Flash_Success;

    if (0U != bytes)
    {
        uint16_t unalignedBytes = (uint16_t)PGM_SIZE_BYTE - bytes;

        if (unalignedBytes > size)
        {
            unalignedBytes = (uint16_t)size;
        }

        (void)memcpy(buffer, (void *)((uint16_t *)(dest - bytes)), PGM_SIZE_BYTE);
        (void)memcpy(&buffer[bytes], pData, unalignedBytes);

        status = HAL_FlashProgramAdaptation(dest - bytes, PGM_SIZE_BYTE, buffer);

        if (kStatus_HAL_Flash_Success != status)
        {
            return status;
        }

        dest += (uint32_t)PGM_SIZE_BYTE - bytes;
        pData += unalignedBytes;
        size -= unalignedBytes;
    }

    bytes = (uint16_t)(size & ~((uint8_t)PGM_SIZE_BYTE - 1U));

    if (0U != bytes)
    {
        status = HAL_FlashProgramAdaptation(dest, bytes, pData);

        if (kStatus_HAL_Flash_Success != status)
        {
            return status;
        }

        dest += bytes;
        pData += bytes;
        size -= bytes;
    }

    if (0U != size)
    {
        (void)memcpy(buffer, (void *)((uint32_t *)dest), PGM_SIZE_BYTE);
        (void)memcpy(buffer, pData, size);

        return HAL_FlashProgramAdaptation(dest, PGM_SIZE_BYTE, buffer);
    }

    return kStatus_HAL_Flash_Success;
}

/*!
 * \brief  Erase to 0xFF one or more FLASH sectors.
 *
 * @param dest            The start address of the first sector to be erased
 * @param size            The amount of flash to be erased (multiple of sector size)
 *
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 *
 */
hal_flash_status_t HAL_FlashEraseSector(uint32_t dest, uint32_t size)
{
    int32_t status;
    uint32_t regPrimask = DisableGlobalIRQ();
    status = FLEXNVM_DflashErase(&s_flexNvmDriver, s_dflashBlockBase + dest, size, (uint32_t)kFLASH_ApiEraseKey);
    EnableGlobalIRQ(regPrimask);
    return HAL_FlashGetStatus(status);
}

/*!
 * \brief  Read data from FLASH
 *
 * @param scr             The address of the Flash location to be read
 * @param size            The number of bytes to be read
 * @param pData           Pointer to the data to be read from Flash
 *
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 *
 */
hal_flash_status_t HAL_FlashRead(uint32_t src, uint32_t size, uint8_t *pData)
{
    (void)memcpy(pData, (uint8_t *)src, size);

    return kStatus_HAL_Flash_Success;
}

/*!
 * @brief Returns the desired hal flash property.
 *
 * @param Property        The desired property from the list of properties in
 *                        enum hal_flash_property_tag_t
 * @param value           A pointer to the value returned for the desired flash property.
 *
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 * @retval #kStatus_HAL_Flash_InvalidArgument An invalid argument is provided.
 * @retval #kStatus_HAL_Flash_NotSupport Flash currently not support.
 */
hal_flash_status_t HAL_FlashGetProperty(hal_flash_property_tag_t property, uint32_t *value)
{
    return HAL_FlashGetStatus(FLASH_GetProperty(&s_flexNvmDriver, (flash_property_tag_t)property, value));
}

/*!
 * @brief Set the desired hal flash property.
 *
 * @param Property        The desired property from the list of properties in
 *                        enum hal_flash_property_tag_t
 * @param value           The value would be set to the desired flash property.
 *
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 * @retval #kStatus_HAL_Flash_InvalidArgument An invalid argument is provided.
 * @retval #kStatus_HAL_Flash_NotSupport Flash currently not support.
 */
hal_flash_status_t HAL_FlashSetProperty(hal_flash_property_tag_t property, uint32_t value)
{
    return kStatus_HAL_Flash_NotSupport;
}

/*!
 * @brief Returns the security state via the pointer passed into the function.
 *
 * This function retrieves the current flash security status, including the
 * security enabling state and the backdoor key enabling state.
 *
 * @param state           A pointer to the value returned for the current security status
 *
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 * @retval #kStatus_HAL_Flash_InvalidArgument An invalid argument is provided.
 * @retval #kStatus_HAL_Flash_NotSupport Flash currently not support.
 */
hal_flash_status_t HAL_FlashGetSecurityState(hal_flash_security_state_t *state)
{
    return HAL_FlashGetStatus(FLASH_GetSecurityState(&s_flexNvmDriver, (ftfx_security_state_t *)state));
}

/*!
 * \brief  Read data from FLASH with ECC Fault detection enabled.
 *
 *  Note : BusFault is not raised, just SoC indication
 *
 * @param scr             The address of the Flash location to be read
 * @param size            The number of bytes to be read
 * @param pData           Pointer to the data to be read from Flash
 *
 * @retval #kStatus_HAL_Flash_Success API was executed successfully.
 *         #kStatus_HAL_Flash_EccError if ECC Fault error got raised
 *
 */
hal_flash_status_t HAL_FlashReadCheckEccFaults(uint32_t src, uint32_t size, uint8_t *pData)
{
    return kStatus_HAL_Flash_NotSupport;
}
