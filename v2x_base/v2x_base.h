/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Public API for the EdgeLock V2X base (debug) service driver.
 *
 * These four base operations are ported from the Linux driver
 * drivers/firmware/imx/v2x_base_msg.c and are the minimal set used to bring up
 * and query the V2X firmware over its message unit (MU):
 *   - V2X_Base_StartRng()      start the V2X TRNG (0x0E)
 *   - V2X_Base_SetPowerState() change V2X-FW power state (0x04)
 *   - V2X_Base_GetFwVersion()  read the V2X firmware version (0x08)
 *   - V2X_Base_DebugDump()     read the V2X debug dump buffer (0x02)
 */

#ifndef V2X_BASE_H_
#define V2X_BASE_H_

#include "fsl_common.h"
#include "fsl_mu.h"
#include <stdbool.h>

/*!
 * @addtogroup v2x_base
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief V2X firmware version fields (from get_fw_version). */
typedef struct _v2x_fw_version
{
    uint32_t major;     /*!< Major version.                        */
    uint32_t minor;     /*!< Minor version.                        */
    uint32_t patch;     /*!< Patch level.                          */
    uint32_t commitId;  /*!< Firmware commit id.                   */
} v2x_fw_version_t;

/*! @brief V2X power-state request action. */
typedef enum _v2x_pwr_action
{
    kV2X_PowerOn  = 0x1u, /*!< Power the V2X firmware on.  */
    kV2X_PowerOff = 0x4u, /*!< Power the V2X firmware off. */
} v2x_pwr_action_t;

/*! @brief Maximum number of 32-bit words returned by a debug dump call. */
#define V2X_BASE_DBG_DUMP_MAX_WORDS (23u)

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Start the V2X TRNG.
 *
 * Sends the V2X_START_RNG_REQ (0x0E) command. If the firmware reports that
 * TRNG initialization is still in progress, kStatus_Busy is returned so the
 * caller can retry.
 *
 * @param mu V2X message unit base.
 * @retval kStatus_Success TRNG started (or already running).
 * @retval kStatus_Busy    TRNG initialization still in progress; retry.
 * @retval kStatus_Fail    Command failed.
 */
status_t V2X_Base_StartRng(MU_Type *mu);

/*!
 * @brief Change the V2X firmware power state.
 *
 * Sends the V2X_PWR_STATE (0x04) command with the requested action.
 *
 * @param mu     V2X message unit base.
 * @param action Power-on or power-off request.
 * @retval kStatus_Success        Power state changed.
 * @retval kStatus_InvalidArgument Argument invalid.
 * @retval kStatus_Fail           Command failed (permission denied / invalid op).
 */
status_t V2X_Base_SetPowerState(MU_Type *mu, v2x_pwr_action_t action);

/*!
 * @brief Read the V2X firmware version.
 *
 * Sends the V2X_DBG_GET_FW_VER (0x08) command and parses the version fields.
 *
 * @param mu      V2X message unit base.
 * @param version Output version structure.
 * @retval kStatus_Success        Version read.
 * @retval kStatus_InvalidArgument Argument invalid.
 * @retval kStatus_Fail           Command failed.
 */
status_t V2X_Base_GetFwVersion(MU_Type *mu, v2x_fw_version_t *version);

/*!
 * @brief Read the V2X debug dump buffer.
 *
 * Sends the V2X_DBG_DUMP_REQ (0x02) command and copies the returned dump
 * words into the caller buffer.
 *
 * @param mu       V2X message unit base.
 * @param dump     Output buffer for dump words (at least V2X_BASE_DBG_DUMP_MAX_WORDS).
 * @param capacity Size of dump in 32-bit words.
 * @param outWords On success, number of valid dump words written.
 * @retval kStatus_Success        Dump read.
 * @retval kStatus_InvalidArgument Argument invalid.
 * @retval kStatus_Fail           Command failed.
 */
status_t V2X_Base_DebugDump(MU_Type *mu, uint32_t *dump, uint32_t capacity, uint32_t *outWords);


#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* V2X_BASE_H_ */
