/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mx25l_flash.h"

#define MX25L_BYTE_ADDR1(address) ((uint8_t)(((address) >> 16UL) & 0x0FUL))
#define MX25L_BYTE_ADDR2(address) ((uint8_t)(((address) >> 8UL) & 0x0FUL))
#define MX25L_BYTE_ADDR3(address) ((uint8_t)((address)&0x0FUL))

/* initialize 'mx25l_instance' */
mx25l_err_t mx25l_init(struct mx25l_instance *instance, transfer_cb_t callback, void *callback_prv)
{
    instance->callback = callback;
    instance->prv      = callback_prv;
    return mx25l_err_ok;
}

/* read 'rdid' to 'result' */
mx25l_err_t mx25l_cmd_rdid(struct mx25l_instance *instance, struct mx25l_rdid_result *result)
{
    instance->cmd[0] = 0x9f;
    instance->callback(instance->prv, instance->cmd, NULL, 1, false);
    instance->callback(instance->prv, NULL, (uint8_t *)result, sizeof(*result), true);
    return mx25l_err_ok;
}

/* read n bytes starting at 'address' */
mx25l_err_t mx25l_cmd_read(struct mx25l_instance *instance, uint32_t address, uint8_t *buffer, uint32_t size)
{
    if ((address & 0xFF000000U) != 0x00U)
    {
        return mx25l_err_out_of_range;
    }
    instance->cmd[0] = 0x03;
    instance->cmd[1] = MX25L_BYTE_ADDR1(address);
    instance->cmd[2] = MX25L_BYTE_ADDR2(address);
    instance->cmd[3] = MX25L_BYTE_ADDR3(address);
    instance->callback(instance->prv, instance->cmd, NULL, 4, false);
    instance->callback(instance->prv, NULL, (uint8_t *)buffer, size, true);
    return mx25l_err_ok;
}

/* read status register */
mx25l_err_t mx25l_cmd_rdsr(struct mx25l_instance *instance, struct mx25l_rdsr_result *result)
{
    instance->cmd[0] = 0x05;
    instance->callback(instance->prv, instance->cmd, NULL, 1, false);
    instance->callback(instance->prv, NULL, (uint8_t *)result, sizeof(*result), true);
    return mx25l_err_ok;
}

/* disable write operations */
mx25l_err_t mx25l_cmd_wrdi(struct mx25l_instance *instance)
{
    instance->cmd[0] = 0x04;
    instance->callback(instance->prv, instance->cmd, NULL, 1, true);
    return mx25l_err_ok;
}

/* enable write operations */
mx25l_err_t mx25l_cmd_wren(struct mx25l_instance *instance)
{
    instance->cmd[0] = 0x06;
    instance->callback(instance->prv, instance->cmd, NULL, 1, true);
    return mx25l_err_ok;
}

/* write n bytes (256 max) starting at 'address' aligned to 256 */
mx25l_err_t mx25l_cmd_write(struct mx25l_instance *instance,
                            uint32_t address_256_align,
                            uint8_t *buffer,
                            uint32_t size_256_max)
{
    struct mx25l_rdsr_result result;
    if ((address_256_align & 0xFF000000U) != 0x00U)
    {
        return mx25l_err_out_of_range;
    }
    if ((address_256_align & 0xFFU) != 0x00U)
    {
        return mx25l_err_alignement;
    }
    if (size_256_max > 256U)
    {
        return mx25l_err_out_of_range;
    }
    /* enable write and wait until WEL is 1 */
    (void)mx25l_cmd_wren(instance);
    do
    {
        (void)mx25l_cmd_rdsr(instance, &result);
    } while (((uint8_t)result.sr0 & 0x2U) != 0x02U);
    /* write sequence */
    instance->cmd[0] = 0x02;
    instance->cmd[1] = MX25L_BYTE_ADDR1(address_256_align);
    instance->cmd[2] = MX25L_BYTE_ADDR2(address_256_align);
    instance->cmd[3] = 0;
    instance->callback(instance->prv, instance->cmd, NULL, 4, false);
    instance->callback(instance->prv, (uint8_t *)buffer, NULL, size_256_max, true);
    /* wait until WRI is 0 and WEL is 0 */
    do
    {
        (void)mx25l_cmd_rdsr(instance, &result);
    } while (((uint8_t)result.sr0 & 0x3U) != 0x00U);
    return mx25l_err_ok;
}

/* erase sector at 'address' aligned to sector size = 4kB */
mx25l_err_t mx25l_cmd_sector_erase(struct mx25l_instance *instance, uint32_t address)
{
    struct mx25l_rdsr_result result;
    /* enable write and wait until WEL is 1 */
    (void)mx25l_cmd_wren(instance);
    do
    {
        (void)mx25l_cmd_rdsr(instance, &result);
    } while (((uint8_t)result.sr0 & 0x2U) != 0x02U);
    /* write sequence */
    instance->cmd[0] = 0x20;
    instance->cmd[1] = MX25L_BYTE_ADDR1(address);
    instance->cmd[2] = MX25L_BYTE_ADDR2(address);
    instance->cmd[3] = MX25L_BYTE_ADDR3(address);
    instance->callback(instance->prv, instance->cmd, NULL, 4, true);
    /* wait until WRI is 0 and WEL is 0 */
    do
    {
        (void)mx25l_cmd_rdsr(instance, &result);
    } while (((uint8_t)result.sr0 & 0x3U) != 0x00U);
    return mx25l_err_ok;
}
