/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MX25L_FLASH_H_
#define _MX25L_FLASH_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum mx25l_err
{
    mx25l_err_ok           = 0,
    mx25l_err_out_of_range = 1,
    mx25l_err_alignement   = 2,
} mx25l_err_t;

typedef void (*transfer_cb_t)(void *transfer_prv, uint8_t *tx_data, uint8_t *rx_data, size_t dataSize, bool eof);

struct mx25l_instance
{
    void *prv;
    transfer_cb_t callback;
    uint8_t cmd[5];
};

#if defined(__GNUC__)
#else
__packed
#endif
struct mx25l_rdid_result
{
    char manufacturer;
    char device[2];
#if defined(__GNUC__)
} __attribute__((packed));
#else
};
#endif

#if defined(__GNUC__)
#else
__packed
#endif
struct mx25l_rdsr_result
{
    char sr0;
#if defined(__GNUC__)
} __attribute__((packed));
#else
};
#endif

#if defined(__GNUC__)
#else
__packed
#endif
struct mx25l_read_result
{
    uint32_t word[4];
#if defined(__GNUC__)
} __attribute__((packed));
#else
};
#endif

union mx25l_result
{
    struct mx25l_rdid_result rdid;
    struct mx25l_rdsr_result rdsr;
    struct mx25l_read_result read;
};

mx25l_err_t mx25l_init(struct mx25l_instance *instance, transfer_cb_t callback, void *callback_prv);
mx25l_err_t mx25l_cmd_rdid(struct mx25l_instance *instance, struct mx25l_rdid_result *result);
mx25l_err_t mx25l_cmd_read(struct mx25l_instance *instance, uint32_t address, uint8_t *buffer, uint32_t size);
mx25l_err_t mx25l_cmd_rdsr(struct mx25l_instance *instance, struct mx25l_rdsr_result *result);
mx25l_err_t mx25l_cmd_wrdi(struct mx25l_instance *instance);
mx25l_err_t mx25l_cmd_wren(struct mx25l_instance *instance);
mx25l_err_t mx25l_cmd_write(struct mx25l_instance *instance,
                            uint32_t address_256_align,
                            uint8_t *buffer,
                            uint32_t size_256_max);
mx25l_err_t mx25l_cmd_sector_erase(struct mx25l_instance *instance, uint32_t address);

#endif
