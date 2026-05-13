/*
 * Copyright (c) 2020 Laczen
 * Copyright 2026 NXP. Not a Contribution.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _EEPROM_EMULATION_H_
#define _EEPROM_EMULATION_H_

#include "eeprom_emulation_platform.h"


/* Used to describe binding to flash driver */
struct eeprom_emu_flashapi {
    int (*read)(uint32_t offset, void *dst, size_t len);
    int (*write)(uint32_t offset, const void *src, size_t len);
    int (*erase)(uint32_t offset, size_t len);
    uint8_t erase_value;
};

/* EEPROM device configuration structure */
struct eeprom_emu_config {
    /* EEPROM size */
    size_t size;
    /* EEPROM is read-only */
    bool readonly;
    /* Page size used to emulate the EEPROM, contains one area of EEPROM
     * size and an area to store changes.
     */
    size_t page_size;
    /* Offset of the flash partition used to emulate the EEPROM */
    off_t flash_offset;
    /* Size of the flash partition to emulate the EEPROM */
    size_t flash_size;
    /* Delay the erase of EEPROM pages until the complete partition is used.
     */
    bool partitionerase;
    /* Size of a change block */
    uint16_t flash_cbs;
    uint8_t *rambuf;

    struct eeprom_emu_flashapi flashapi;
};

/* EEPROM device context structure */
struct eeprom_emu_data {
    /* Offset in current (EEPROM) page where next change is written */
    off_t write_offset;
    /* Offset of the current (EEPROM) page */
    off_t page_offset;
    /* Flash erase value */
    uint8_t flash_erase_val;
    /* Mutex lock */
    struct k_mutex lock;
};


/* Main structure representing emulated EEPROM device */
struct eeprom_emu_dev {
    struct eeprom_emu_config config;
    struct eeprom_emu_data data;
};


/*********** The main API ***********/


/**
 * @brief Initialize EEPROM emulation.
 *
 * Prepares the flash-backed EEPROM by locating or creating a valid page
 * and initializing internal state.
 *
 * @param dev Pointer to the EEPROM emulation device structure.
 *
 * @retval 0        Initialization successful.
 * @retval negative Error code on failure.
 */

int eeprom_emu_init(struct eeprom_emu_dev *dev);


/**
 * @brief Read data from emulated EEPROM.
 *
 * Retrieves the latest data from the EEPROM by combining base data
 * and stored change records.
 *
 * @param dev     Pointer to the EEPROM emulation device structure.
 * @param address Start address within the emulated EEPROM.
 * @param data    Pointer to the buffer where read data will be stored.
 * @param len     Number of bytes to read.
 *
 * @retval 0        Read successful.
 * @retval negative Error code on failure.
 */

int eeprom_emu_read(struct eeprom_emu_dev *dev, off_t address, void *data, size_t len);


/**
 * @brief Write data to emulated EEPROM.
 *
 * Stores data as change records in flash; triggers page compaction
 * when space is exhausted.
 *
 * @param dev     Pointer to the EEPROM emulation device structure.
 * @param address Start address within the emulated EEPROM.
 * @param data    Pointer to the data to write.
 * @param len     Number of bytes to write.
 *
 * @retval 0        Write successful.
 * @retval negative Error code on failure.
 */

int eeprom_emu_write(struct eeprom_emu_dev *dev, off_t address, const void *data, size_t len);

#endif /* _EEPROM_EMULATION_H */

