/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "sm_reset.h"
#include "scmi_reset.h"

int32_t SM_Reset(sm_rst_t *sm_rst)
{
    uint32_t channel       = sm_rst->channel;
    uint32_t domainId      = sm_rst->domainId;
    uint32_t flags          = sm_rst->flags;
    uint32_t resetState    = sm_rst->resetState;

    return SCMI_Reset(channel, domainId, flags, resetState);
}
