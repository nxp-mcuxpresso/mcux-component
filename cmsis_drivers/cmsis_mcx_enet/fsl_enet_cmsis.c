/*
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 * Copyright (c) 2016, Freescale Semiconductor, Inc. Not a Contribution.
 * Copyright 2016-2026 NXP. Not a Contribution.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "fsl_enet_cmsis.h"

#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "platform.drivers.mcx_enet_cmsis"
#endif

#define ARM_ETH_MAC_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2, 4)

#if defined(FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL) && FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL
#if defined(FSL_FEATURE_L2CACHE_LINESIZE_BYTE) && \
    ((!defined(FSL_SDK_DISBLE_L2CACHE_PRESENT)) || (FSL_SDK_DISBLE_L2CACHE_PRESENT == 0))
#define CMSIS_CACHE_LINESIZE_MAX  MAX(FSL_FEATURE_L1DCACHE_LINESIZE_BYTE, FSL_FEATURE_L2CACHE_LINESIZE_BYTE)
#define CMSIS_ENET_BUFF_ALIGNMENT MAX(ENET_BUFF_ALIGNMENT, CMSIS_CACHE_LINESIZE_MAX)
#elif defined(FSL_FEATURE_L1DCACHE_LINESIZE_BYTE)
#define CMSIS_ENET_BUFF_ALIGNMENT MAX(ENET_BUFF_ALIGNMENT, FSL_FEATURE_L1DCACHE_LINESIZE_BYTE)
#else
#define CMSIS_ENET_BUFF_ALIGNMENT ENET_BUFF_ALIGNMENT
#endif
#else
#define CMSIS_ENET_BUFF_ALIGNMENT ENET_BUFF_ALIGNMENT
#endif

#define ENET_RXBD_NUM    (4U)
#define ENET_TXBD_NUM    (4U)
#define ENET_RXBUFF_SIZE (ENET_FRAME_MAX_FRAMELEN)

typedef struct _cmsis_enet_mac_driver_state
{
    cmsis_enet_mac_resource_t *resource;
    enet_handle_t *handle;
    enet_buffer_config_t *buffers;
    enet_config_t *config;
    ARM_ETH_MAC_SignalEvent_t cb_event;
    uint8_t flags;
    bool rxEnabled;
    bool txEnabled;
    bool started;
} cmsis_enet_mac_driver_state_t;

static const ARM_DRIVER_VERSION s_enetDriverVersion = {ARM_ETH_MAC_API_VERSION, ARM_ETH_MAC_DRV_VERSION};

static const ARM_ETH_MAC_CAPABILITIES s_enetDriverCapabilities = {
    0,                      /* checksum_offload_rx_ip4  */
    0,                      /* checksum_offload_rx_ip6  */
    0,                      /* checksum_offload_rx_udp  */
    0,                      /* checksum_offload_rx_tcp  */
    0,                      /* checksum_offload_rx_icmp */
    0,                      /* checksum_offload_tx_ip4  */
    0,                      /* checksum_offload_tx_ip6  */
    0,                      /* checksum_offload_tx_udp  */
    0,                      /* checksum_offload_tx_tcp  */
    0,                      /* checksum_offload_tx_icmp */
    ARM_ETH_INTERFACE_MII,  /* media_interface          */
    0,                      /* mac_address              */
    1,                      /* event_rx_frame           */
    1,                      /* event_tx_frame           */
    0,                      /* event_wakeup             */
    0                       /* precision_timer          */
};

static ARM_DRIVER_VERSION ENETx_GetVersion(void)
{
    return s_enetDriverVersion;
}

static ARM_ETH_MAC_CAPABILITIES ENETx_GetCapabilities(void)
{
    return s_enetDriverCapabilities;
}

static void ENET_ConfigureDefaults(cmsis_enet_mac_driver_state_t *enet)
{
    ENET_GetDefaultConfig(enet->config);
    enet->config->miiMode   = kENET_MiiMode;
    enet->config->miiSpeed  = kENET_MiiSpeed10M;
    enet->config->miiDuplex = kENET_MiiHalfDuplex;
    enet->config->interrupt = (uint32_t)kENET_DmaRx;
    enet->config->specialControl = 0U;
}

static void ENET_EventCallback(ENET_Type *base,
                               enet_handle_t *handle,
                               enet_event_t event,
                               uint8_t channel,
                               enet_tx_reclaim_info_t *txReclaimInfo,
                               void *userData)
{
    uint32_t cmsisEvent = ETH_MAC_EVENT_OTHERS;

    (void)base;
    (void)handle;
    (void)channel;
    (void)txReclaimInfo;

    switch (event)
    {
        case kENET_RxIntEvent:
            cmsisEvent = ARM_ETH_MAC_EVENT_RX_FRAME;
            break;
        case kENET_TxIntEvent:
            cmsisEvent = ARM_ETH_MAC_EVENT_TX_FRAME;
            break;
#ifdef ENET_PTP1588FEATURE_REQUIRED
        case kENET_TimeStampIntEvent:
            cmsisEvent = ARM_ETH_MAC_EVENT_TIMER_ALARM;
            break;
#endif
        default:
            break;
    }

    if (userData != NULL)
    {
        ((ARM_ETH_MAC_SignalEvent_t)userData)(cmsisEvent);
    }
}

static int32_t ENET_CommonInitialize(ARM_ETH_MAC_SignalEvent_t cb_event, cmsis_enet_mac_driver_state_t *enet)
{
    if ((enet->flags & ENET_FLAG_INIT) == 0U)
    {
        enet->cb_event = cb_event;
        ENET_ConfigureDefaults(enet);
        enet->flags    = ENET_FLAG_INIT;
        enet->rxEnabled = false;
        enet->txEnabled = false;
        enet->started   = false;
    }

    return ARM_DRIVER_OK;
}

static int32_t ENET_CommonUninitialize(cmsis_enet_mac_driver_state_t *enet)
{
    enet->flags     = ENET_FLAG_UNINIT;
    enet->rxEnabled = false;
    enet->txEnabled = false;
    enet->started   = false;
    return ARM_DRIVER_OK;
}

static int32_t ENET_CommonPowerControl(ARM_POWER_STATE state, cmsis_enet_mac_driver_state_t *enet)
{
    uint8_t macAddr[6] = {0};
    status_t status;

    switch (state)
    {
        case ARM_POWER_OFF:
            if ((enet->flags & ENET_FLAG_POWER) != 0U)
            {
                ENET_DisableInterrupts(enet->resource->base, 0xFFFFFFFFU);
                ENET_Deinit(enet->resource->base);
                enet->flags     = ENET_FLAG_INIT;
                enet->rxEnabled = false;
                enet->txEnabled = false;
                enet->started   = false;
            }
            return ARM_DRIVER_OK;

        case ARM_POWER_LOW:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        case ARM_POWER_FULL:
            if (enet->flags == ENET_FLAG_UNINIT)
            {
                return ARM_DRIVER_ERROR;
            }

            if ((enet->flags & ENET_FLAG_POWER) != 0U)
            {
                return ARM_DRIVER_OK;
            }

            enet->config->miiSpeed  = kENET_MiiSpeed10M;
            enet->config->miiDuplex = kENET_MiiHalfDuplex;
            ENET_Init(enet->resource->base, enet->config, macAddr, enet->resource->GetFreq());
            status = ENET_DescriptorInit(enet->resource->base, enet->config, enet->buffers);
            if (status != kStatus_Success)
            {
                ENET_Deinit(enet->resource->base);
                return ARM_DRIVER_ERROR;
            }

            ENET_CreateHandler(enet->resource->base, enet->handle, enet->config, enet->buffers, ENET_EventCallback,
                               (void *)enet->cb_event);

            enet->flags |= ENET_FLAG_POWER;
            return ARM_DRIVER_OK;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

static int32_t ENET_Phy_Write(cmsis_enet_mac_resource_t *enet, uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    return (ENET_MDIOWrite(enet->base, phy_addr, reg_addr, data) == kStatus_Success) ? ARM_DRIVER_OK : ARM_DRIVER_ERROR;
}

static int32_t ENET_Phy_Read(cmsis_enet_mac_resource_t *enet, uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
    return (ENET_MDIORead(enet->base, phy_addr, reg_addr, data) == kStatus_Success) ? ARM_DRIVER_OK : ARM_DRIVER_ERROR;
}

static int32_t ENET_CommonSendFrame(cmsis_enet_mac_driver_state_t *enet,
                                    const uint8_t *frame,
                                    uint32_t len,
                                    uint32_t flags)
{
    enet_buffer_struct_t txBuffer;
    enet_tx_frame_struct_t txFrame;
    status_t status;

    if ((flags & (ARM_ETH_MAC_TX_FRAME_FRAGMENT | ARM_ETH_MAC_TX_FRAME_TIMESTAMP)) != 0U)
    {
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    txBuffer.buffer = (void *)frame;
    txBuffer.length = (uint16_t)len;

    (void)memset(&txFrame, 0, sizeof(txFrame));
    txFrame.txBuffArray            = &txBuffer;
    txFrame.txBuffNum              = 1U;
    txFrame.txConfig.intEnable     = ((flags & ARM_ETH_MAC_TX_FRAME_EVENT) != 0U) ? 1U : 0U;
    txFrame.txConfig.tsEnable      = 0U;
    txFrame.txConfig.txOffloadOps  = kENET_TxOffloadDisable;
    txFrame.context                = (void *)frame;

    status = ENET_SendFrame(enet->resource->base, enet->handle, &txFrame, 0U);
    if (status == kStatus_ENET_TxFrameOverLen)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    if (status == kStatus_ENET_TxFrameBusy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    return (status == kStatus_Success) ? ARM_DRIVER_OK : ARM_DRIVER_ERROR;
}

static uint32_t ENET_CommonGetFrameSize(cmsis_enet_mac_driver_state_t *enet)
{
    uint32_t length = 0U;
    status_t status = ENET_GetRxFrameSize(enet->resource->base, enet->handle, &length, 0U);

    if (status == kStatus_ENET_RxFrameError)
    {
        (void)ENET_ReadFrame(enet->resource->base, enet->handle, NULL, 0U, 0U, NULL);
        return 0U;
    }

    return (status == kStatus_Success) ? length : 0U;
}

static int32_t ENET_CommonReadFrame(cmsis_enet_mac_driver_state_t *enet, uint8_t *frame, uint32_t len)
{
    status_t status;

    if (len > 0x7FFFFFFFU)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    status = ENET_ReadFrame(enet->resource->base, enet->handle, frame, len, 0U, NULL);
    return (status == kStatus_Success) ? (int32_t)len : ARM_DRIVER_ERROR;
}

static void ENET_UpdateStartState(cmsis_enet_mac_driver_state_t *enet)
{
    if (enet->txEnabled || enet->rxEnabled)
    {
        if (!enet->started)
        {
            ENET_StartRxTx(enet->resource->base, 1U, 1U);
            enet->started = true;
        }
    }
    else if (enet->started)
    {
        enet->resource->base->MAC_CONFIGURATION &= ~(ENET_MAC_CONFIGURATION_TE_MASK | ENET_MAC_CONFIGURATION_RE_MASK);
        enet->resource->base->DMA_CH[0].DMA_CHX_TX_CTRL &= ~ENET_DMA_CH_DMA_CHX_TX_CTRL_ST_MASK;
        enet->resource->base->DMA_CH[0].DMA_CHX_RX_CTRL &= ~ENET_DMA_CH_DMA_CHX_RX_CTRL_SR_MASK;
        enet->started = false;
    }
}

static int32_t ENET_CommonControl(uint32_t control, uint32_t arg, cmsis_enet_mac_driver_state_t *enet)
{
    if (enet->flags == ENET_FLAG_UNINIT)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (control)
    {
        case ARM_ETH_MAC_CONFIGURE:
            switch (arg & ARM_ETH_MAC_SPEED_Msk)
            {
                case ARM_ETH_MAC_SPEED_10M:
                    enet->config->miiSpeed = kENET_MiiSpeed10M;
                    break;
                default:
                    return ARM_DRIVER_ERROR_UNSUPPORTED;
            }

            switch (arg & ARM_ETH_MAC_DUPLEX_Msk)
            {
                case ARM_ETH_MAC_DUPLEX_HALF:
                    enet->config->miiDuplex = kENET_MiiHalfDuplex;
                    break;
                default:
                    return ARM_DRIVER_ERROR_UNSUPPORTED;
            }

            if ((arg & (ARM_ETH_MAC_CHECKSUM_OFFLOAD_RX | ARM_ETH_MAC_CHECKSUM_OFFLOAD_TX)) != 0U)
            {
                return ARM_DRIVER_ERROR_UNSUPPORTED;
            }

            if ((arg & ARM_ETH_MAC_ADDRESS_BROADCAST) != 0U)
            {
                enet->config->specialControl &= ~(uint16_t)kENET_BroadCastRxDisable;
                enet->resource->base->MAC_PACKET_FILTER &= ~ENET_MAC_PACKET_FILTER_DBF_MASK;
            }
            else
            {
                enet->config->specialControl |= (uint16_t)kENET_BroadCastRxDisable;
                enet->resource->base->MAC_PACKET_FILTER |= ENET_MAC_PACKET_FILTER_DBF_MASK;
            }

            if ((arg & ARM_ETH_MAC_ADDRESS_MULTICAST) != 0U)
            {
                enet->config->specialControl |= (uint16_t)kENET_MulticastAllEnable;
                ENET_AcceptAllMulticast(enet->resource->base);
            }
            else
            {
                enet->config->specialControl &= ~(uint16_t)kENET_MulticastAllEnable;
                ENET_RejectAllMulticast(enet->resource->base);
            }

            if ((arg & ARM_ETH_MAC_ADDRESS_ALL) != 0U)
            {
                enet->config->specialControl |= (uint16_t)kENET_PromiscuousEnable;
                enet->resource->base->MAC_PACKET_FILTER |= ENET_MAC_PACKET_FILTER_PR_MASK;
            }
            else
            {
                enet->config->specialControl &= ~(uint16_t)kENET_PromiscuousEnable;
                enet->resource->base->MAC_PACKET_FILTER &= ~ENET_MAC_PACKET_FILTER_PR_MASK;
            }

            ENET_SetMII(enet->resource->base, enet->config->miiSpeed, enet->config->miiDuplex);
            return ARM_DRIVER_OK;

        case ARM_ETH_MAC_CONTROL_TX:
            enet->txEnabled = (arg != 0U);
            ENET_UpdateStartState(enet);
            return ARM_DRIVER_OK;

        case ARM_ETH_MAC_CONTROL_RX:
            enet->rxEnabled = (arg != 0U);
            ENET_UpdateStartState(enet);
            return ARM_DRIVER_OK;

        case ARM_ETH_MAC_SLEEP:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

#if RTE_ENET
AT_NONCACHEABLE_SECTION_ALIGN(static enet_rx_bd_struct_t g_rxBuffDescrip[ENET_RXBD_NUM], ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(static enet_tx_bd_struct_t g_txBuffDescrip[ENET_TXBD_NUM], ENET_BUFF_ALIGNMENT);
static enet_tx_reclaim_info_t g_txDirtyInfo[ENET_TXBD_NUM];
SDK_ALIGN(static uint8_t g_rxDataBuff[ENET_RXBD_NUM][SDK_SIZEALIGN(ENET_RXBUFF_SIZE, CMSIS_ENET_BUFF_ALIGNMENT)],
          CMSIS_ENET_BUFF_ALIGNMENT);
static uint32_t g_rxDataBuffAddr[ENET_RXBD_NUM];

extern cmsis_enet_mac_resource_t ENET0_Resource;
extern void ENET0_InitPins(void);
extern void ENET0_DeinitPins(void);

static enet_handle_t ENET0_Handle;
static enet_config_t ENET0_Config;
static enet_buffer_config_t ENET0_BuffConfig = {
    ENET_RXBD_NUM,
    ENET_TXBD_NUM,
    &g_txBuffDescrip[0],
    &g_txBuffDescrip[ENET_TXBD_NUM],
    &g_txDirtyInfo[0],
    &g_rxBuffDescrip[0],
    &g_rxBuffDescrip[ENET_RXBD_NUM],
    &g_rxDataBuffAddr[0],
    SDK_SIZEALIGN(ENET_RXBUFF_SIZE, CMSIS_ENET_BUFF_ALIGNMENT)};

static cmsis_enet_mac_driver_state_t ENET0_DriverState = {
    &ENET0_Resource, &ENET0_Handle, &ENET0_BuffConfig, &ENET0_Config, NULL, ENET_FLAG_UNINIT, false, false, false};

static void ENET0_PrepareBuffers(void)
{
    uint32_t i;

    for (i = 0U; i < ENET_RXBD_NUM; i++)
    {
        g_rxDataBuffAddr[i] = (uint32_t)(uintptr_t)&g_rxDataBuff[i][0];
    }
}

static int32_t ENET0_Initialize(ARM_ETH_MAC_SignalEvent_t cb_event)
{
    ENET0_PrepareBuffers();
    ENET0_InitPins();
    return ENET_CommonInitialize(cb_event, &ENET0_DriverState);
}

static int32_t ENET0_Uninitialize(void)
{
    ENET0_DeinitPins();
    return ENET_CommonUninitialize(&ENET0_DriverState);
}

static int32_t ENET0_PowerControl(ARM_POWER_STATE state)
{
    return ENET_CommonPowerControl(state, &ENET0_DriverState);
}

static int32_t ENET0_GetMacAddress(ARM_ETH_MAC_ADDR *ptr_addr)
{
    ENET_GetMacAddr(ENET0_Resource.base, &ptr_addr->b[0]);
    return ARM_DRIVER_OK;
}

static int32_t ENET0_SetMacAddress(const ARM_ETH_MAC_ADDR *ptr_addr)
{
    uint8_t mac[6];
    (void)memcpy((void *)&mac[0], (const void *)ptr_addr, 6U);
    ENET_SetMacAddr(ENET0_Resource.base, &mac[0]);
    return ARM_DRIVER_OK;
}

static int32_t ENET0_TransmitFrame(const uint8_t *frame, uint32_t len, uint32_t flags)
{
    return ENET_CommonSendFrame(&ENET0_DriverState, frame, len, flags);
}

static uint32_t ENET0_GetReceiveFrameSize(void)
{
    return ENET_CommonGetFrameSize(&ENET0_DriverState);
}

static int32_t ENET0_ReceiveFrame(uint8_t *frame, uint32_t len)
{
    return ENET_CommonReadFrame(&ENET0_DriverState, frame, len);
}

static int32_t ENET0_Control(uint32_t control, uint32_t arg)
{
    return ENET_CommonControl(control, arg, &ENET0_DriverState);
}

static int32_t ENET0_PhyWrite(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    return ENET_Phy_Write(&ENET0_Resource, phy_addr, reg_addr, data);
}

static int32_t ENET0_PhyRead(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
    return ENET_Phy_Read(&ENET0_Resource, phy_addr, reg_addr, data);
}

static int32_t ENET0_SetAddressFilter(const ARM_ETH_MAC_ADDR *ptr_addr, uint32_t num_addr)
{
    (void)ptr_addr;
    (void)num_addr;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static int32_t ENET0_GetRxFrameTime(ARM_ETH_MAC_TIME *time)
{
    (void)time;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static int32_t ENET0_GetTxFrameTime(ARM_ETH_MAC_TIME *time)
{
    (void)time;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static int32_t ENET0_ControlTimer(uint32_t control, ARM_ETH_MAC_TIME *time)
{
    (void)control;
    (void)time;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

ARM_DRIVER_ETH_MAC Driver_ETH_MAC0 = {ENETx_GetVersion,
                                      ENETx_GetCapabilities,
                                      ENET0_Initialize,
                                      ENET0_Uninitialize,
                                      ENET0_PowerControl,
                                      ENET0_GetMacAddress,
                                      ENET0_SetMacAddress,
                                      ENET0_SetAddressFilter,
                                      ENET0_TransmitFrame,
                                      ENET0_ReceiveFrame,
                                      ENET0_GetReceiveFrameSize,
                                      ENET0_GetRxFrameTime,
                                      ENET0_GetTxFrameTime,
                                      ENET0_ControlTimer,
                                      ENET0_Control,
                                      ENET0_PhyRead,
                                      ENET0_PhyWrite};
#endif
