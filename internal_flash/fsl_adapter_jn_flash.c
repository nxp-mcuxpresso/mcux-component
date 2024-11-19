/*
 * Copyright 2018, 2020 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*! *********************************************************************************
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
extern flash_config_t gFlashConfig;
#define FLASH_WORD_SZ (16)

/*! *********************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
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
/*! *********************************************************************************
 * \brief This function ensures that there are no critical sections on and disables interrupts.
 *
 * \param[in] none
 * \return nothing
 *
 ********************************************************************************** */
/*! *********************************************************************************
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
    uint32_t status = kStatus_HAL_Flash_Success;

    if (((size & (PGM_SIZE_BYTE - 0x01U)) != 0) || ((dest & (PGM_SIZE_BYTE - 0x01U)) != 0))
    {
        return kStatus_HAL_Flash_AlignmentError;
    }

    while (size)
    {
        memcpy(progBuf, pData, PGM_SIZE_BYTE);

        status = FLASH_Program(FLASH, (uint32_t *)dest, (uint32_t *)progBuf, PGM_SIZE_BYTE);

        if (status != FLASH_DONE)
        {
            break;
        }

        pData += PGM_SIZE_BYTE;
        dest += PGM_SIZE_BYTE;
        size -= PGM_SIZE_BYTE;
    }

    return kStatus_HAL_Flash_Success;
}

static hal_flash_status_t HAL_FlashGetStatus(status_t status)
{
    hal_flash_status_t flashStatus = kStatus_HAL_Flash_Error;
    switch (status)
    {
        case kStatus_Success:
            flashStatus = kStatus_HAL_Flash_Success;
            break;
        case FLASH_FAIL:
            flashStatus = kStatus_HAL_Flash_Fail;
            break;
        case FLASH_ERR:
            flashStatus = kStatus_HAL_Flash_Error;
            break;
        case FLASH_DONE:
            flashStatus = kStatus_HAL_Flash_Success;
            break;

        default:
            break;
    }
    return flashStatus;
}
/*! *********************************************************************************
 * \brief  This function copies bytes from FLASH to ram.
 *
 * \param[in, out]  pDst Pointer to the destination buffer.
 *
 * \param[in]  pSrc Pointer to the source buffer.
 *
 * \param[in]  cBytes Number of bytes to copy.
 *
 * \remarks
 *
 ********************************************************************************** */
static uint8_t HAL_FlashCopy(void *pDst, const void *pSrc, uint32_t cBytes)
{
    uint32_t status;
    uint8_t is_blank = false;

    status = FLASH_BlankCheck(FLASH, (uint8_t *)pSrc, (uint8_t *)((uint8_t *)pSrc + cBytes - 1));
    if (FLASH_DONE != status)
    {
        memcpy(pDst, pSrc, cBytes);
    }
    else
    {
        memset(pDst, 0, cBytes);
        is_blank = true;
    }
    return is_blank;
}
/*! *********************************************************************************
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
    static uint8_t nvmInit = 0;

    if (nvmInit == 0)
    {
        /* Init Flash */
        FLASH_Init(FLASH);

        nvmInit = 1;
    }
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
    uint32_t status;

    status = FLASH_BlankCheck(FLASH, (uint8_t *)start, (uint8_t *)(start + lengthInBytes - 1));

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
    hal_flash_status_t status;

    if (bytes)
    {
        uint16_t unalignedBytes = PGM_SIZE_BYTE - bytes;

        if (unalignedBytes > size)
        {
            unalignedBytes = size;
        }

        HAL_FlashCopy(buffer, (void *)(dest - bytes), PGM_SIZE_BYTE);

        memcpy(&buffer[bytes], pData, unalignedBytes);

        if ((status = HAL_FlashProgramAdaptation(dest - bytes, PGM_SIZE_BYTE, buffer)) != kStatus_HAL_Flash_Success)
        {
            return status;
        }

        dest += PGM_SIZE_BYTE - bytes;
        pData += unalignedBytes;
        size -= unalignedBytes;
    }

    bytes = size & ~(PGM_SIZE_BYTE - 1U);

    if (bytes)
    {
        if ((status = HAL_FlashProgramAdaptation(dest, bytes, pData)) != kStatus_HAL_Flash_Success)
        {
            return status;
        }

        dest += bytes;
        pData += bytes;
        size -= bytes;
    }

    if (size)
    {
        HAL_FlashCopy(buffer, (void *)dest, PGM_SIZE_BYTE);
        memcpy(buffer, pData, size);
        if ((status = HAL_FlashProgramAdaptation(dest, PGM_SIZE_BYTE, buffer)) != kStatus_HAL_Flash_Success)
        {
            return status;
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

    status = FLASH_Erase(FLASH, (uint8_t *)dest, (uint8_t *)(dest + size - 1));
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
    uint32_t au32Data[FLASH_WORD_SZ / sizeof(uint32_t)];
    uint32_t status;
    uint32_t srcRead;
    uint32_t offset;
    uint8_t *pTempData;

    uint32_t nb_flash_words = (size + src % FLASH_WORD_SZ + (FLASH_WORD_SZ - 1)) / FLASH_WORD_SZ;
    srcRead                 = src - src % FLASH_WORD_SZ;
    size                    = size + src % FLASH_WORD_SZ;
    offset                  = src % FLASH_WORD_SZ;
    while (nb_flash_words-- > 0)
    {
        uint32_t sz;
        sz     = (size >= FLASH_WORD_SZ) ? FLASH_WORD_SZ : size;
        status = FLASH_Read(FLASH, (uint8_t *)srcRead, 0, au32Data);
        if (FLASH_DONE != status)
            break;
        pTempData = (uint8_t *)&au32Data[0];
        memcpy(pData, pTempData + offset, sz - offset);
        pData += sz - offset;
        srcRead += sz;
        size -= sz;
        offset = 0;
    }
    return HAL_FlashGetStatus(status);
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
            *value = gFlashConfig.PFlashSectorSize;
            break;

        case kHAL_Flash_PropertyPflashTotalSize:
            *value = gFlashConfig.PFlashTotalSize;
            break;
        case kHAL_Flash_PropertyPflashBlockBaseAddr:
            *value = gFlashConfig.PFlashBlockBase;
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
