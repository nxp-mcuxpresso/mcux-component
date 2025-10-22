/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_phytenbaset.h"
#include "fsl_tenbaset_phy.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PHY_BASE(handle) (((phy_tenbaset_resource_t *)(handle)->resource)->base)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

const phy_operations_t phytenbaset_ops = {.phyInit            = &PHY_TENBASET_Init,
                                          .getAutoNegoStatus  = &PHY_TENBASET_GetAutoNegotiationStatus,
                                          .getLinkStatus      = &PHY_TENBASET_GetLinkStatus,
                                          .getLinkSpeedDuplex = &PHY_TENBASET_GetLinkSpeedDuplex,
                                          .setLinkSpeedDuplex = &PHY_TENBASET_SetLinkSpeedDuplex,
                                          .enableLoopback     = &PHY_TENBASET_EnableLoopback};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t PHY_TENBASET_Init(phy_handle_t *handle, const phy_config_t *config)
{
    phy_tenbaset_resource_t *resource;

    /* Assign PHY address and operation resource. */
    handle->phyAddr  = config->phyAddr;
    handle->resource = config->resource;

    resource = (phy_tenbaset_resource_t *)config->resource;

    TENBASET_PHY_CreateHandle(resource->base, &resource->handle, NULL, NULL);

    return TENBASET_PHY_Init(resource->base, &resource->config, &resource->handle);
}

status_t PHY_TENBASET_GetAutoNegotiationStatus(phy_handle_t *handle, bool *status)
{
    return PHY_TENBASET_GetLinkStatus(handle, status);
}

status_t PHY_TENBASET_GetLinkStatus(phy_handle_t *handle, bool *status)
{
    assert(status);

    *status = TENBASET_PHY_IsLinkUp(PHY_BASE(handle));

    return kStatus_Success;
}

status_t PHY_TENBASET_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex)
{
    assert(!((speed == NULL) && (duplex == NULL)));

    if (speed != NULL)
    {
        *speed = kPHY_Speed10M;
    }

    if (duplex != NULL)
    {
        *duplex = kPHY_HalfDuplex;
    }

    return kStatus_Success;
}

status_t PHY_TENBASET_SetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t speed, phy_duplex_t duplex)
{
    assert(speed == kPHY_Speed10M);
    assert(duplex == kPHY_HalfDuplex);

    if ((speed != kPHY_Speed10M) || (duplex != kPHY_HalfDuplex))
    {
        return kStatus_InvalidArgument;
    }

    return kStatus_Success;
}

status_t PHY_TENBASET_EnableLoopback(phy_handle_t *handle, phy_loop_t mode, phy_speed_t speed, bool enable)
{
    assert((mode == kPHY_LocalLoop) || (mode == kPHY_RemoteLoop));

    if (mode == kPHY_LocalLoop)
    {
        tenbaset_phy_pcs_config_t config;
        TENBASET_PHY_GetPCSConfig(PHY_BASE(handle), &config);
        config.loopbackEnable = enable;
        TENBASET_PHY_SetPCSConfig(PHY_BASE(handle), &config);
    }
    else if (mode == kPHY_RemoteLoop)
    {
        tenbaset_phy_pma_config_t config;
        TENBASET_PHY_GetPMAConfig(PHY_BASE(handle), &config);
        config.loopbackEnable = enable;
        TENBASET_PHY_SetPMAConfig(PHY_BASE(handle), &config);
    }
    else
    {
        return kStatus_InvalidArgument;
    }

    return kStatus_Success;
}
