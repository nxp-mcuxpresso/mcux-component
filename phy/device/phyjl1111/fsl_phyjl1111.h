/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/****************************************************************************
 * PHY JL1111 driver change log
 ****************************************************************************/

/*!
@page driver_log Driver Change Log

@section phyjl1111 PHYJL1111
  The current PHYJL1111 driver version is 2.0.0.

  - 2.0.0
    - Initial version.
*/

#ifndef _FSL_PHYJL1111_H_
#define _FSL_PHYJL1111_H_

#include "fsl_phy.h"

/*!
 * @addtogroup phy_driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief PHY driver version */
#define FSL_PHY_DRIVER_VERSION (MAKE_VERSION(2, 0, 0))

typedef struct _phy_jl1111_resource
{
    mdioWrite write;
    mdioRead read;
} phy_jl1111_resource_t;

extern const phy_operations_t phyjl1111_ops;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name PHY Driver
 * @{
 */

status_t PHY_JL1111_Init(phy_handle_t *handle, const phy_config_t *config);
status_t PHY_JL1111_Write(phy_handle_t *handle, uint8_t phyReg, uint16_t data);
status_t PHY_JL1111_Read(phy_handle_t *handle, uint8_t phyReg, uint16_t *pData);
status_t PHY_JL1111_WriteC45(phy_handle_t *handle, uint8_t devAddr, uint16_t regAddr, uint16_t data);
status_t PHY_JL1111_ReadC45(phy_handle_t *handle, uint8_t devAddr, uint16_t regAddr, uint16_t *pData);
status_t PHY_JL1111_GetAutoNegotiationStatus(phy_handle_t *handle, bool *status);
status_t PHY_JL1111_GetLinkStatus(phy_handle_t *handle, bool *status);
status_t PHY_JL1111_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex);
status_t PHY_JL1111_SetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t speed, phy_duplex_t duplex);
status_t PHY_JL1111_EnableLoopback(phy_handle_t *handle, phy_loop_t mode, phy_speed_t speed, bool enable);
status_t PHY_JL1111_EnableLinkInterrupt(phy_handle_t *handle, phy_interrupt_type_t type);
status_t PHY_JL1111_ClearInterrupt(phy_handle_t *handle);

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* _FSL_PHYJL1111_H_ */
