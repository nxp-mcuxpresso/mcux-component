/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_silicon_id_soc.h"

status_t SILICONID_ReadUniqueID(uint8_t *siliconId, uint32_t *idLen)
{
    uint32_t readBytes = *idLen;
    uint8_t uid[16];

    *((uint32_t *)(uintptr_t)&uid[0])  = MAIN__FSB->FUSE[15];
    *((uint32_t *)(uintptr_t)&uid[4])  = MAIN__FSB->FUSE[14];
    *((uint32_t *)(uintptr_t)&uid[8])  = MAIN__FSB->FUSE[13];
    *((uint32_t *)(uintptr_t)&uid[12]) = MAIN__FSB->FUSE[12];

    readBytes = (readBytes > 16U) ? 16U : readBytes;
    (void)memcpy((void *)&siliconId[0], (void *)&uid[0], readBytes);
    *idLen = readBytes;

    return kStatus_Success;
}
