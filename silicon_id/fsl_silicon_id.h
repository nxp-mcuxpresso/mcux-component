/*
 * Copyright 2022, 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SILICON_ID_H_
#define _FSL_SILICON_ID_H_

#include "fsl_common.h"

#define SILICONID_MAX_LENGTH 16U

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Get the silicon ID.
 *
 * @param siliconId[out]  Memory address to store silicon ID, array length depends on specific Soc. The
 * SILICONID_MAX_LENGTH is the maximum length of all supported Socs.
 * @param idLen[in/out]  Silicon ID byte length. Set the length to read, return the length read out.
 * @retval kStatus_Success  Succeed to get silicon ID.
 * @retval kStatus_Fail  Fail to get silicon ID.
 */
status_t SILICONID_GetID(uint8_t *siliconId, uint32_t *idLen);

/*!
 * @brief Get and convert the silicon ID to ethernet MAC address.
 *
 * @warning This is the API provided by the NXP MCUX SDK for test cases. The generated address is
 * NOT guaranteed to be unique across devices. The MAC is composed of a fixed NXP OUI plus only the
 * lowest 3 bytes of the silicon ID, and on some SoCs those bytes are identical between parts.
 * Do not rely on this function as a source of a unique ethernet MAC address.
 *
 * @param macAddr[out]  Memory address to store ethernet MAC address.
 * @retval kStatus_Success  Succeed to get silicon ID and finish conversion.
 * @retval kStatus_Fail  Fail to get silicon ID and finish conversion.
 */
status_t SILICONID_ConvertToMacAddr(uint8_t (*macAddr)[6]);

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_SILICON_ID_H_ */
