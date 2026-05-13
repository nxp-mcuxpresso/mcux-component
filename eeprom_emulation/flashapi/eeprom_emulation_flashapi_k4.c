/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "fsl_flash.h"
#include "fsl_k4_flash.h"
#include "eeprom_emulation.h"

#define IS_ALIGN4(p) ((((uintptr_t)(p)) & 0x3) == 0)
#define PHRASE_SIZE FSL_FEATURE_FLASH_PFLASH_PHRASE_SIZE

static flash_config_t s_flashDriver;

/* Flash API bindings for K4 devices */


int eeprom_emu_flashapi_read(uint32_t offset, void *dst, size_t len)
{
    uint32_t src = FSL_FEATURE_FLASH_PFLASH_START_ADDRESS + offset;

    memcpy(dst, (void *)src, len);

    return 0;
}


int eeprom_emu_flashapi_write(uint32_t offset, const void *src, size_t len)
{
    status_t result;
    uint32_t dst = FSL_FEATURE_FLASH_PFLASH_START_ADDRESS + offset;

    if (IS_ALIGN4(src))
    {
        result = FLASH_Program(&s_flashDriver, FLASH, dst, (uint8_t *)src, len);
    }
    else
    {
        /* if the source buffer is not 4B aligned then use an intermediate buffer */
        uint32_t tmp[PHRASE_SIZE / 4];

        while (len)
        {
            size_t chunk = len > PHRASE_SIZE ? PHRASE_SIZE : len;
            memcpy(tmp, src, chunk);

            result = FLASH_Program(&s_flashDriver, FLASH, dst, (uint8_t *)tmp, chunk);
            if (result != kStatus_Success)
            {
                return EEPROM_EMU_EIO;
            }

            len -= chunk;
            dst += chunk;
        }
    }

    return result;
}


int eeprom_emu_flashapi_erase(uint32_t offset, size_t len)
{
    status_t result;

    uint32_t erase_offset = FSL_FEATURE_FLASH_PFLASH_START_ADDRESS + offset;
    uint32_t sector_size = FSL_FEATURE_FLASH_PFLASH_SECTOR_SIZE;

    while (len)
    {
        result = FLASH_Erase(&s_flashDriver, FLASH, erase_offset, sector_size, kFLASH_ApiEraseKey);
        if (result != kStatus_Success)
        {
            return EEPROM_EMU_EIO;
        }

        erase_offset += sector_size;
        len          -= sector_size;
    }

    return 0;
}

int eeprom_emu_flashapi_init(struct eeprom_emu_flashapi *api)
{
   status_t result;

   result = FLASH_Init(&s_flashDriver);

   if (result != kStatus_Success)
       return -EEPROM_EMU_EIO;;

   if (api)
   {
       api->read = eeprom_emu_flashapi_read;
       api->write = eeprom_emu_flashapi_write;
       api->erase = eeprom_emu_flashapi_erase;
       api->erase_value = 0xFF;
   }

   return 0;
}

