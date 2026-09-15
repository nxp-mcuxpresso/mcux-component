/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Serial MUX UART hub - backward-compatible one-call bring-up.
 *
 * This file preserves the original MUX_UART_Hub* API but is now a thin shim:
 * it builds an LPUART transport backend (mux_transport_lpuart) and wires its
 * write/read hooks into the transport-neutral mux core (MUX_HUB_Init). All the
 * LPUART software rings and the RX ISR live in mux_transport_lpuart.c behind
 * the generic mux_transport_t ops table, so swapping in a different transport
 * only means constructing a different backend and calling MUX_HUB_Init with
 * its hooks - no change here or in the mux core.
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "mux_uart_transport.h"

/* Adapters: the mux core hooks take a void* user; forward to the backend ops. */
static void hub_write_adapter(const uint8_t *data, uint32_t len, void *user)
{
    const mux_transport_t *tp = (const mux_transport_t *)user;
    tp->ops->write(tp->ctx, data, len);
}

static uint32_t hub_read_adapter(uint8_t *buf, uint32_t cap, void *user)
{
    const mux_transport_t *tp = (const mux_transport_t *)user;
    return tp->ops->read(tp->ctx, buf, cap);
}

void MUX_UART_HubInit(mux_uart_hub_t *t, const mux_uart_hub_cfg_t *cfg)
{
    mux_transport_lpuart_cfg_t lp_cfg;

    memset(t, 0, sizeof(*t));
    t->self_channel = cfg->self_channel;

    lp_cfg.uart     = cfg->uart;
    lp_cfg.irqn     = cfg->irqn;
    lp_cfg.clk_freq = cfg->clk_freq;
    t->transport    = mux_transport_lpuart(&lp_cfg);

    (void)t->transport->ops->open(t->transport->ctx, cfg->baud);

    MUX_HUB_Init(&t->hub, cfg->region, hub_write_adapter, hub_read_adapter,
                 (void *)t->transport);
}

uint32_t MUX_UART_HubService(mux_uart_hub_t *t)
{
    uint32_t moved = MUX_HUB_Poll(&t->hub);
    if (t->transport->ops->service != NULL)
    {
        moved += t->transport->ops->service(t->transport->ctx);
    }
    return moved;
}

void MUX_UART_HubEmit(mux_uart_hub_t *t, const char *fmt, ...)
{
    char    line[MUX_SLOTS_PAYLOAD_MAX];
    uint8_t wire[SERIAL_MUX_MAX_FRAME];
    va_list ap;
    int     n;
    uint32_t w;

    va_start(ap, fmt);
    n = vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    if (n <= 0)
    {
        return;
    }
    if ((uint32_t)n > sizeof(line))
    {
        n = (int)sizeof(line);
    }
    w = SERIAL_MUX_Encode(t->self_channel, (const uint8_t *)line, (uint32_t)n, wire, sizeof(wire));
    if (w > 0u)
    {
        t->transport->ops->write(t->transport->ctx, wire, w);
    }
}

void MUX_UART_HubIrq(mux_uart_hub_t *t)
{
    (void)t;
    mux_transport_lpuart_rx_isr();
    SDK_ISR_EXIT_BARRIER;
}
