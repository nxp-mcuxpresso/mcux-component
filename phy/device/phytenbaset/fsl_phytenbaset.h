/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*****************************************************************************
 * PHY PHYTENBASET driver change log
 *****************************************************************************/

/*!
@page driver_log Driver Change Log

@section phytenbaset PHYTENBASET
  The current PHYTENBASET driver version is 2.0.0.

  - 2.0.0
    - Initial version.
*/

#ifndef _FSL_PHYTENBASET_H_
#define _FSL_PHYTENBASET_H_

#include "fsl_phy.h"
#include "fsl_tenbaset_phy.h"

/*!
 * @addtogroup phy_driver
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief PHY driver version */
#define FSL_PHY_DRIVER_VERSION (MAKE_VERSION(2, 0, 0))

typedef struct _phy_tenbaset_resource
{
    TENBASET_PHY_Type *base;
    tenbaset_phy_config_t config;
    tenbaset_phy_handle_t handle;
} phy_tenbaset_resource_t;

extern const phy_operations_t phytenbaset_ops;

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

/*!
 * @brief Initializes PHY.
 * This function initializes PHY.
 *
 * @param handle  PHY device handle.
 * @param config  PHY configuration.
 * @retval kStatus_Success  PHY initialization succeeds
 * @retval kStatus_Fail  PHY initialization fails
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_TENBASET_Init(phy_handle_t *handle, const phy_config_t *config);

/*!
 * @brief Gets the PHY auto-negotiation status.
 *
 * @param handle  PHY device handle.
 * @param status  The auto-negotiation status of the PHY.
 *         - true the auto-negotiation is over.
 *         - false the auto-negotiation is on-going or not started.
 * @retval kStatus_Success  PHY gets status success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_TENBASET_GetAutoNegotiationStatus(phy_handle_t *handle, bool *status);

/*!
 * @brief Gets the PHY link status.
 *
 * @param handle   PHY device handle.
 * @param status   The link up or down status of the PHY.
 *         - true the link is up.
 *         - false the link is down.
 * @retval kStatus_Success   PHY gets link status success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_TENBASET_GetLinkStatus(phy_handle_t *handle, bool *status);

/*!
 * @brief Gets the PHY link speed and duplex.
 *
 * @brief This function gets the speed and duplex mode of PHY. User can give one of speed
 * and duplex address paramter and set the other as NULL if only wants to get one of them.
 *
 * @param handle   PHY device handle.
 * @param speed    The address of PHY link speed.
 * @param duplex   The link duplex of PHY.
 * @retval kStatus_Success   PHY gets link speed and duplex success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_TENBASET_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex);

/*!
 * @brief Sets the PHY link speed and duplex.
 *
 * @param handle   PHY device handle.
 * @param speed    Specified PHY link speed.
 * @param duplex   Specified PHY link duplex.
 * @retval kStatus_Success   PHY gets status success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_TENBASET_SetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t speed, phy_duplex_t duplex);

/*!
 * @brief Enables/Disables PHY loopback.
 *
 * @param handle   PHY device handle.
 * @param mode     The loopback mode to be enabled, please see "phy_loop_t".
 * All loopback modes should not be set together, when one loopback mode is set
 * another should be disabled.
 * @param speed    PHY speed for loopback mode.
 * @param enable   True to enable, false to disable.
 * @retval kStatus_Success  PHY loopback success
 * @retval kStatus_Timeout  PHY MDIO visit time out
 */
status_t PHY_TENBASET_EnableLoopback(phy_handle_t *handle, phy_loop_t mode, phy_speed_t speed, bool enable);

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* _FSL_PHY_H_ */
