/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * mux_hub.h - Alternative B Mux Hub (runs on the hub core, cm33_core1).
 *
 * The hub is the sole owner of the UART-MUX physical port (LPUART11) and the
 * only COBS encode/decode + routing point. It uses the polling slot IPC
 * (mux_slots.h) instead of an MU doorbell:
 *
 *   Uplink   (core -> PC): poll every uplink slot, COBS-encode (channel,
 *                          payload), write bytes to UART-MUX TX.
 *   Downlink (PC -> core): feed UART-MUX RX bytes into the mux decoder; each
 *                          decoded frame is routed by channel into the target
 *                          core's downlink slot.
 *
 * Provide the two UART hooks below (write bytes / read available bytes).
 *
 * All comments are ASCII-only English per workspace convention.
 */

#ifndef MUX_HUB_H
#define MUX_HUB_H

#include <stdint.h>
#include <stddef.h>

#include "mux_slots.h"
#include "serial_mux.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Write 'len' bytes to the UART-MUX TX. Must send all bytes. */
typedef void (*mux_hub_uart_write_fn)(const uint8_t *data, uint32_t len, void *user);

/* Non-blocking read of up to 'cap' bytes from UART-MUX RX into 'buf'. Returns
 * the number of bytes read (0 if none available). */
typedef uint32_t (*mux_hub_uart_read_fn)(uint8_t *buf, uint32_t cap, void *user);

/* Optional handler for a decoded downlink frame whose channel has no owning
 * core (MUX_SLOTS_ChannelToCore() returned < 0). The hub itself owns such a
 * channel (its own console), so the demo can register this to consume/echo
 * those frames instead of having them dropped. */
typedef void (*mux_hub_self_fn)(uint8_t channel, const uint8_t *payload,
                                uint32_t len, void *user);

typedef struct {
    mux_slots_region_t   *region;      /* shared slot region              */
    serial_mux_rx_t       rx;          /* COBS RX assembler for downlink  */
    mux_hub_uart_write_fn uart_write;  /* UART-MUX TX hook                */
    mux_hub_uart_read_fn  uart_read;   /* UART-MUX RX hook                */
    void                 *uart_user;   /* opaque passed to the hooks      */

    mux_hub_self_fn       self_cb;     /* frames on an ownerless channel  */
    void                 *self_user;   /* opaque passed to self_cb        */

    /* Statistics (optional, useful for debug). */
    uint32_t up_frames;                /* uplink frames encoded to UART   */
    uint32_t down_frames;              /* downlink frames routed to cores */
    uint32_t down_dropped;             /* downlink frames dropped (slot full) */
} mux_hub_t;

/*
 * Initialize the hub. 'region' must already be published by
 * MUX_SLOTS_HubInit(). Provide UART TX/RX hooks and their user context.
 */
void MUX_HUB_Init(mux_hub_t *hub,
                  mux_slots_region_t *region,
                  mux_hub_uart_write_fn uart_write,
                  mux_hub_uart_read_fn uart_read,
                  void *uart_user);

/*
 * Register an optional handler for decoded downlink frames whose channel has
 * no owning core (e.g. the hub's own console channel). Without it such frames
 * are dropped (down_dropped++). Pass NULL to clear.
 */
void MUX_HUB_SetSelfHandler(mux_hub_t *hub, mux_hub_self_fn cb, void *user);

/*
 * Run one hub service iteration (call repeatedly from the hub main loop):
 *  1. Poll every uplink slot; encode+send any pending frames to UART-MUX.
 *  2. Drain available UART-MUX RX bytes; decode frames and route to cores.
 * Returns the number of frames moved this pass (uplink + downlink).
 */
uint32_t MUX_HUB_Poll(mux_hub_t *hub);

#ifdef __cplusplus
}
#endif

#endif /* MUX_HUB_H */
