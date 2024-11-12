/*
 * Copyright 2018 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_RPMSG_EMU_H__
#define __SRTM_RPMSG_EMU_H__

#include "srtm_rpmsg_endpoint.h"

/*!
 * @addtogroup srtm_channel
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief SRTM RPMsg endpoint channel create/destroy callback function type.
 */
typedef void (*srtm_rpmsg_endpoint_hook_t)(srtm_channel_t channel, srtm_rpmsg_endpoint_config_t *config, void *param);

/**
 * @brief SRTM RPMsg endpoint channel TX callback function type.
 */
typedef void (*srtm_rpmsg_endpoint_tx_cb_t)(srtm_channel_t channel, void *payload, int payloadLen, void *param);

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Set callback to be called when channel created.
 *
 * @param callback User function to be called when channel created.
 * @param param User parameter to be used in callback.
 */
void SRTM_RPMsgEndpoint_SetCreateHook(srtm_rpmsg_endpoint_hook_t callback, void *param);

/*!
 * @brief Set callback to be called when channel destroyed.
 *
 * @param callback User function to be called when channel destroyed.
 * @param param User parameter to be used in callback.
 */
void SRTM_RPMsgEndpoint_SetDestroyHook(srtm_rpmsg_endpoint_hook_t callback, void *param);

/*!
 * @brief Emulate received data from channel.
 *
 * @param channel SRTM channel received data from.
 * @param data start address of received data.
 * @param len data length in bytes.
 * @return RL_RELEASE on data buffer used or RL_HOLD on data buffer to be processed.
 */
int SRTM_RPMsgEndpoint_RecvData(srtm_channel_t channel, void *data, uint32_t len);

/*!
 * @brief Override RPMsg endpoint channel TX handler.
 *
 * This function is to provide callback for unit test application.
 *
 * @param channel SRTM channel to override TX handler.
 * @param callback User function to handle TX message.
 * @param param User parameter to be used in callback.
 * @return SRTM_Status_Success on success and others on failure.
 */
srtm_status_t SRTM_RPMsgEndpoint_OverrideTxHandler(srtm_channel_t channel,
                                                   srtm_rpmsg_endpoint_tx_cb_t callback,
                                                   void *param);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_RPMSG_EMU_H__ */
