/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_silicon_id.h"

#define MCXE32X_UUID_ADDR (0x1B000040U)
#define MCXE32X_UUID_SIZE (8U)

status_t SILICONID_ReadUniqueID(uint8_t *siliconId, uint32_t *idLen)
{
    *idLen     = (MCXE32X_UUID_SIZE <= *idLen) ? MCXE32X_UUID_SIZE : *idLen;
    uint8_t *p = (uint8_t *)MCXE32X_UUID_ADDR;

    for (uint8_t i = 0U; i < *idLen; i++)
    {
        *siliconId = *p;
        siliconId++;
        p++;
    }

    return kStatus_Success;
}
