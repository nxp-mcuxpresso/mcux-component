/*
 * Copyright 2018-2021 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include "srtm_heap.h"
#include "srtm_dispatcher.h"
#include "srtm_dispatcher_struct.h"
#include "srtm_peercore.h"
#include "srtm_peercore_struct.h"
#include "srtm_channel.h"
#include "srtm_channel_struct.h"
#include "srtm_rpmsg_emu.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef SRTM_DEBUG_COMMUNICATION
#define SRTM_DEBUG_COMMUNICATION (0)
#endif

typedef struct _srtm_rpmsg_emu
{
    struct _srtm_channel channel;
    srtm_rpmsg_endpoint_config_t config;
    srtm_rpmsg_endpoint_rx_cb_t rxCallback;
    void *rxCallbackParam;
    srtm_rpmsg_endpoint_tx_cb_t txCallback;
    void *txCallbackParam;
    bool started;
} *srtm_rpmsg_emu_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static srtm_rpmsg_endpoint_hook_t createHook;
static void *createParam;
static srtm_rpmsg_endpoint_hook_t destroyHook;
static void *destroyParam;

/*******************************************************************************
 * Code
 ******************************************************************************/
void SRTM_RPMsgEndpoint_SetCreateHook(srtm_rpmsg_endpoint_hook_t callback, void *param)
{
    createHook  = callback;
    createParam = param;
}

void SRTM_RPMsgEndpoint_SetDestroyHook(srtm_rpmsg_endpoint_hook_t callback, void *param)
{
    destroyHook  = callback;
    destroyParam = param;
}

int SRTM_RPMsgEndpoint_RecvData(srtm_channel_t channel, void *data, uint32_t len)
{
    srtm_rpmsg_emu_t handle = (srtm_rpmsg_emu_t)channel;

    assert(handle);

#if SRTM_DEBUG_COMMUNICATION
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: RPMsg recv:\r\n\t", __func__);
    for (uint32_t i = 0; i < len; i++)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%x ", ((uint8_t *)data)[i]);
    }
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "\r\n");
#endif

    if (handle->started)
    {
        if (handle->rxCallback)
        {
            return handle->rxCallback(&handle->channel, data, len, 0, handle->rxCallbackParam);
        }

        assert(handle->channel.core);
        assert(handle->channel.core->dispatcher);

        SRTM_Dispatcher_PostRecvData(handle->channel.core->dispatcher, &handle->channel, data, len);
    }
    else
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: Get data before channel start\r\n", __func__);
    }

    return RL_RELEASE;
}

static srtm_status_t SRTM_RPMsgEndpoint_Start(srtm_channel_t channel)
{
    srtm_rpmsg_emu_t handle = (srtm_rpmsg_emu_t)channel;
    srtm_status_t status    = SRTM_Status_Success;

    assert(handle);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle->started = true;

    return status;
}

static srtm_status_t SRTM_RPMsgEndpoint_Stop(srtm_channel_t channel)
{
    srtm_rpmsg_emu_t handle = (srtm_rpmsg_emu_t)channel;
    srtm_status_t status    = SRTM_Status_Success;

    assert(handle);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle->started = false;

    return status;
}

static srtm_status_t SRTM_RPMsgEndpoint_SendData(srtm_channel_t channel, void *data, uint32_t len)
{
    srtm_rpmsg_emu_t handle = (srtm_rpmsg_emu_t)channel;
    srtm_status_t status    = SRTM_Status_InvalidState;

    assert(handle);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_DEBUG, "%s: len %d\r\n", __func__, len);

    if (handle->started)
    {
#if SRTM_DEBUG_COMMUNICATION
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: RPMsg send:\r\n\t", __func__);
        for (uint32_t i = 0; i < len; i++)
        {
            SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%x ", ((uint8_t *)data)[i]);
        }
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "\r\n");
#endif
        assert(handle->txCallback);
        handle->txCallback(channel, data, len, handle->txCallbackParam);
        status = SRTM_Status_Success;
    }
    else
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: channel not started\r\n", __func__);
    }

    return status;
}

srtm_channel_t SRTM_RPMsgEndpoint_Create(srtm_rpmsg_endpoint_config_t *config)
{
    srtm_rpmsg_emu_t handle;

    assert(config);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_rpmsg_emu_t)SRTM_Heap_Malloc(sizeof(struct _srtm_rpmsg_emu));
    assert(handle);

    handle->started         = false;
    handle->rxCallback      = NULL;
    handle->rxCallbackParam = NULL;
    handle->txCallback      = NULL;
    handle->txCallbackParam = NULL;

    (void)memcpy(&handle->config, config, sizeof(struct _srtm_rpmsg_endpoint_config));

    SRTM_List_Init(&handle->channel.node);
    handle->channel.core     = NULL;
    handle->channel.destroy  = SRTM_RPMsgEndpoint_Destroy;
    handle->channel.start    = SRTM_RPMsgEndpoint_Start;
    handle->channel.stop     = SRTM_RPMsgEndpoint_Stop;
    handle->channel.sendData = SRTM_RPMsgEndpoint_SendData;

    if (createHook)
    {
        createHook(&handle->channel, &handle->config, createParam);
    }

    return &handle->channel;
}

void SRTM_RPMsgEndpoint_Destroy(srtm_channel_t channel)
{
    srtm_rpmsg_emu_t handle = (srtm_rpmsg_emu_t)channel;

    assert(channel);
    assert(!channel->core);                    /* Channel must be removed from core before destroy */
    assert(SRTM_List_IsEmpty(&channel->node)); /* Channel must not on any list */

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    if (destroyHook)
    {
        destroyHook(channel, &handle->config, destroyParam);
    }

    SRTM_Heap_Free(handle);
}

srtm_status_t SRTM_RPMsgEndpoint_OverrideRxHandler(srtm_channel_t channel,
                                                   srtm_rpmsg_endpoint_rx_cb_t callback,
                                                   void *param)
{
    srtm_rpmsg_emu_t handle = (srtm_rpmsg_emu_t)channel;

    assert(handle);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle->rxCallback      = callback;
    handle->rxCallbackParam = param;

    return SRTM_Status_Success;
}

srtm_status_t SRTM_RPMsgEndpoint_OverrideTxHandler(srtm_channel_t channel,
                                                   srtm_rpmsg_endpoint_tx_cb_t callback,
                                                   void *param)
{
    srtm_rpmsg_emu_t handle = (srtm_rpmsg_emu_t)channel;

    assert(handle);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle->txCallback      = callback;
    handle->txCallbackParam = param;

    return SRTM_Status_Success;
}
