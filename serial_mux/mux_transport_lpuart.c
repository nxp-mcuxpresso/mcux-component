/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * LPUART transport backend for the Serial MUX hub.
 *
 * This is the first concrete mux_transport_t: it carries the COBS wire stream
 * over an LPUART instance. It owns a software TX ring (drained on TDRE) and a
 * software RX ring (filled from the RX-full ISR), so the hub poll loop and the
 * ISR are decoupled from the mux core. The board-specific wiring (LPUART_Type*,
 * IRQ number, source clock) lives ONLY in this file behind the generic ops
 * table, so the mux core and the MUX_UART_* compat shims never see fsl_lpuart.h.
 */

#include <string.h>
#include "fsl_lpuart.h"
#include "mux_transport.h"
#include "mux_uart_transport.h" /* MUX_UART_TX_RING_SIZE / RX_RING_SIZE tunables */

typedef struct _mux_transport_lpuart_state
{
    LPUART_Type *uart;
    IRQn_Type    irqn;
    uint32_t     clk_freq;

    volatile uint8_t  tx_ring[MUX_UART_TX_RING_SIZE];
    volatile uint32_t tx_head, tx_tail;
    volatile uint8_t  rx_ring[MUX_UART_RX_RING_SIZE];
    volatile uint32_t rx_head, rx_tail;
} mux_transport_lpuart_state_t;

/* Single static instance: one hub owns one physical mux UART. */
static mux_transport_lpuart_state_t s_lpuart_state;
static mux_transport_t              s_lpuart_backend;
static mux_transport_ops_t          s_lpuart_ops;

static uint32_t lpuart_tx_count(const mux_transport_lpuart_state_t *st)
{
    return (st->tx_head - st->tx_tail);
}

static int lpuart_open(void *ctx, uint32_t baud)
{
    mux_transport_lpuart_state_t *st = (mux_transport_lpuart_state_t *)ctx;
    lpuart_config_t config;

    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = baud;
    config.enableTx     = true;
    config.enableRx     = true;
    LPUART_Init(st->uart, &config, st->clk_freq);
    LPUART_EnableInterrupts(st->uart, (uint32_t)kLPUART_RxDataRegFullInterruptEnable);
    EnableIRQ(st->irqn);
    return 0;
}

static void lpuart_write(void *ctx, const uint8_t *data, uint32_t len)
{
    mux_transport_lpuart_state_t *st = (mux_transport_lpuart_state_t *)ctx;
    for (uint32_t i = 0; i < len; i++)
    {
        while (lpuart_tx_count(st) >= MUX_UART_TX_RING_SIZE)
        {
            if (LPUART_GetStatusFlags(st->uart) & (uint32_t)kLPUART_TxDataRegEmptyFlag)
            {
                uint8_t b = st->tx_ring[st->tx_tail % MUX_UART_TX_RING_SIZE];
                st->tx_tail++;
                LPUART_WriteByte(st->uart, b);
            }
        }
        st->tx_ring[st->tx_head % MUX_UART_TX_RING_SIZE] = data[i];
        st->tx_head++;
    }
}

static uint32_t lpuart_read(void *ctx, uint8_t *buf, uint32_t cap)
{
    mux_transport_lpuart_state_t *st = (mux_transport_lpuart_state_t *)ctx;
    uint32_t n = 0;
    while (n < cap && st->rx_tail != st->rx_head)
    {
        buf[n++] = st->rx_ring[st->rx_tail % MUX_UART_RX_RING_SIZE];
        st->rx_tail++;
    }
    return n;
}

static uint32_t lpuart_service(void *ctx)
{
    mux_transport_lpuart_state_t *st = (mux_transport_lpuart_state_t *)ctx;
    uint32_t pushed = 0u;
    while (lpuart_tx_count(st) > 0u &&
           (LPUART_GetStatusFlags(st->uart) & (uint32_t)kLPUART_TxDataRegEmptyFlag))
    {
        uint8_t b = st->tx_ring[st->tx_tail % MUX_UART_TX_RING_SIZE];
        st->tx_tail++;
        LPUART_WriteByte(st->uart, b);
        pushed++;
    }
    return pushed;
}

const mux_transport_t *mux_transport_lpuart(const mux_transport_lpuart_cfg_t *cfg)
{
    if (cfg == NULL)
    {
        return NULL;
    }

    memset(&s_lpuart_state, 0, sizeof(s_lpuart_state));
    s_lpuart_state.uart     = cfg->uart;
    s_lpuart_state.irqn     = cfg->irqn;
    s_lpuart_state.clk_freq = cfg->clk_freq;

    s_lpuart_ops.open    = lpuart_open;
    s_lpuart_ops.write   = lpuart_write;
    s_lpuart_ops.read    = lpuart_read;
    s_lpuart_ops.service = lpuart_service;
    s_lpuart_ops.close   = NULL;

    s_lpuart_backend.ops  = &s_lpuart_ops;
    s_lpuart_backend.ctx  = &s_lpuart_state;
    s_lpuart_backend.name = "lpuart";

    return &s_lpuart_backend;
}

/* Feed one received byte into the RX ring; called from the board LPUART ISR
 * via the MUX_UART_HubIrq compat shim. Kept here so the ring stays private. */
void mux_transport_lpuart_rx_isr(void)
{
    mux_transport_lpuart_state_t *st = &s_lpuart_state;
    uint32_t status = LPUART_GetStatusFlags(st->uart);
    if (status & (uint32_t)kLPUART_RxDataRegFullFlag)
    {
        uint8_t b = LPUART_ReadByte(st->uart);
        if ((uint32_t)(st->rx_head - st->rx_tail) < MUX_UART_RX_RING_SIZE)
        {
            st->rx_ring[st->rx_head % MUX_UART_RX_RING_SIZE] = b;
            st->rx_head++;
        }
    }
}
