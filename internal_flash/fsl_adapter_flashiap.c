/*
 * Copyright 2018 NXP
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
#include "fsl_common.h"
#include "fsl_flashiap.h"
#include "fsl_adapter_flash.h"
/*****************************************************************************
 *****************************************************************************
 * Private macros
 *****************************************************************************
 *****************************************************************************/
#define PGM_SIZE_BYTE FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES
/*!*********************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
********************************************************************************** */
/*! @brief Flash state structure for flashiap driver adapter. */
typedef struct _flash_state
{
    uint32_t totalFlashSize; /*!< Total flash size */
    uint16_t pageSize;       /*!< Page size */
    uint16_t sectionSize;    /*!< Section size */
} flash_state_t;
/*!*********************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
********************************************************************************** */
static hal_flash_status_t HAL_FlashProgramAdaptation(uint32_t dest, uint32_t size, uint8_t *pData);
static flash_state_t s_flashState = {
    .totalFlashSize = FSL_FEATURE_SYSCON_FLASH_SIZE_BYTES,
    .pageSize       = FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES,
    .sectionSize    = FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES,
};
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
    hal_flash_status_t flashStatus = kStatus_HAL_Flash_Error;
    switch (status)
    {
        case kStatus_Success:
            flashStatus = kStatus_HAL_Flash_Success;
            break;
        case kStatus_FLASHIAP_InvalidCommand:
            flashStatus = kStatus_HAL_Flash_Error;
            break;
        default:
            break;
    }
    return flashStatus;
}

/*!*********************************************************************************
 * \brief  Write alligned data to FLASH
 *
 * \param[in] dest        The address of the Flash location
 * \param[in] size        The number of bytes to be programed
 * \param[in] pData       Pointer to the data to be programmed to Flash
 *
 * \return error code
 *
 ********************************************************************************** */
static hal_flash_status_t HAL_FlashProgramAdaptation(uint32_t dest, uint32_t size, uint8_t *pData)
{
    uint32_t progBuf[PGM_SIZE_BYTE / sizeof(uint32_t)];
    uint32_t status = kStatus_FLASHIAP_Success;

    if (((size & (PGM_SIZE_BYTE - 0x01U)) != 0) || ((dest & (PGM_SIZE_BYTE - 0x01U)) != 0))
    {
        return kStatus_HAL_Flash_AlignmentError;
    }

    while (size)
    {
        memcpy(progBuf, pData, PGM_SIZE_BYTE);

        FLASHIAP_PrepareSectorForWrite(dest / s_flashState.sectionSize,
                                       (dest + PGM_SIZE_BYTE) / s_flashState.sectionSize);
        status = FLASHIAP_CopyRamToFlash(dest, progBuf, PGM_SIZE_BYTE, SystemCoreClock);

        if (status != kStatus_FLASHIAP_Success)
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
hal_flash_status_t HAL_FlashInit(void)
{
    return kStatus_HAL_Flash_Success;
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
    uint32_t status, i;
    uint32_t page_buf[FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES / sizeof(uint32_t)];

    for (i = 0; i < s_flashState.pageSize; i++)
    {
        *(((uint8_t *)(&page_buf[0])) + i) = 0xFF;
    }
    /* Verify Erase */
    for (i = 0; i < (lengthInBytes / s_flashState.pageSize); i++)
    {
        status = FLASHIAP_Compare(start + (i * s_flashState.pageSize), &page_buf[0], s_flashState.pageSize);

        if (status != kStatus_FLASHIAP_Success)
        {
            break;
        }
    }

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
    uint16_t bytes = dest & (PGM_SIZE_BYTE - 1);
    uint32_t status;

    if (bytes)
    {
        uint16_t unalignedBytes = PGM_SIZE_BYTE - bytes;

        if (unalignedBytes > size)
        {
            unalignedBytes = size;
        }

        memcpy(buffer, (void *)(dest - bytes), PGM_SIZE_BYTE);
        memcpy(&buffer[bytes], pData, unalignedBytes);

        if ((status = HAL_FlashProgramAdaptation(dest - bytes, PGM_SIZE_BYTE, buffer)) != kStatus_FLASHIAP_Success)
        {
            return HAL_FlashGetStatus(status);
        }

        dest += PGM_SIZE_BYTE - bytes;
        pData += unalignedBytes;
        size -= unalignedBytes;
    }

    bytes = size & ~(PGM_SIZE_BYTE - 1U);

    if (bytes)
    {
        if ((status = HAL_FlashProgramAdaptation(dest, bytes, pData)) != kStatus_FLASHIAP_Success)
        {
            return HAL_FlashGetStatus(status);
        }

        dest += bytes;
        pData += bytes;
        size -= bytes;
    }

    if (size)
    {
        memcpy(buffer, (void *)dest, PGM_SIZE_BYTE);
        memcpy(buffer, pData, size);
        if ((status = HAL_FlashProgramAdaptation(dest, PGM_SIZE_BYTE, buffer)) != kStatus_FLASHIAP_Success)
        {
            return HAL_FlashGetStatus(status);
        }
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
    uint32_t status;
    uint32_t i = 0;
    for (i = dest / s_flashState.sectionSize; i < (dest + size) / s_flashState.sectionSize; i++)
    {
        FLASHIAP_PrepareSectorForWrite(i, i);
        status = FLASHIAP_EraseSector(i, i, SystemCoreClock);
        if (status != kStatus_FLASHIAP_Success)
        {
            break;
        }
    }

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
    hal_flash_status_t status = kStatus_HAL_Flash_Success;

    if (value == NULL)
    {
        return kStatus_HAL_Flash_InvalidArgument;
    }

    switch (property)
    {
        case kHAL_Flash_PropertyPflashSectorSize:
            *value = s_flashState.sectionSize;
            break;
        case kHAL_Flash_PropertyPflashTotalSize:
            *value = s_flashState.totalFlashSize;
            break;
        case kHAL_Flash_PropertyPflashBlockBaseAddr:
            *value = 0;
            break;
        default: /* catch inputs that are not recognized */
            return kStatus_HAL_Flash_NotSupport;
    }
    return status;
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
    hal_flash_status_t status = kStatus_HAL_Flash_Success;

    if (state == NULL)
    {
        return kStatus_HAL_Flash_InvalidArgument;
    }

    *state = kHAL_Flash_SecurityStateNotSecure;

    return status;
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
