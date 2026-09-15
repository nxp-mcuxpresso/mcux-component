/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * mux_hub.c - Alternative B Mux Hub running on the hub core (polling slots).
 *
 * One MUX_HUB_Poll() pass does two things, both non-blocking:
 *   Uplink   (core -> PC): round-robin poll every uplink slot; COBS-encode
 *                          (channel, payload) and write it to UART-MUX.
 *   Downlink (PC -> core): read available UART-MUX RX bytes, feed the
 *                          serial_mux RX assembler; each decoded frame is
 *                          routed by channel into the target core's downlink
 *                          slot.
 *
 * All comments are ASCII-only English per workspace convention.
 */

#include "mux_hub.h"

#include <string.h>

/* Bytes pulled from the UART RX hook per poll pass. */
#ifndef MUX_HUB_RX_CHUNK
#define MUX_HUB_RX_CHUNK 64u
#endif

static void mux_hub_deliver(uint8_t channel, const uint8_t *payload,
                            uint32_t len, void *user)
{
    mux_hub_t *hub = (mux_hub_t *)user;
    int core_index;

    if (hub == NULL) {
        return;
    }

    core_index = MUX_SLOTS_ChannelToCore((uint32_t)channel);
    if (core_index < 0) {
        /* Channel has no owning core (e.g. the hub's own console). Hand it to
         * the registered self-handler if any, otherwise drop it. */
        if (hub->self_cb != NULL) {
            hub->self_cb(channel, payload, len, hub->self_user);
            hub->down_frames++;
        } else {
            hub->down_dropped++;
        }
        return;
    }

    if (MUX_SLOTS_HubTryDownlink(hub->region, (uint32_t)core_index,
                                   (uint32_t)channel, payload, len) != 0) {
        hub->down_frames++;
    } else {
        /* Target core's downlink ring is full: drop this frame. */
        hub->down_dropped++;
    }
}

void MUX_HUB_Init(mux_hub_t *hub,
                  mux_slots_region_t *region,
                  mux_hub_uart_write_fn uart_write,
                  mux_hub_uart_read_fn uart_read,
                  void *uart_user)
{
    if (hub == NULL) {
        return;
    }

    hub->region       = region;
    hub->uart_write   = uart_write;
    hub->uart_read    = uart_read;
    hub->uart_user    = uart_user;
    hub->self_cb      = NULL;
    hub->self_user    = NULL;
    hub->up_frames    = 0u;
    hub->down_frames  = 0u;
    hub->down_dropped = 0u;

    SERIAL_MUX_RxInit(&hub->rx, mux_hub_deliver, hub);
}

void MUX_HUB_SetSelfHandler(mux_hub_t *hub, mux_hub_self_fn cb, void *user)
{
    if (hub == NULL) {
        return;
    }
    hub->self_cb   = cb;
    hub->self_user = user;
}

uint32_t MUX_HUB_Poll(mux_hub_t *hub)
{
    uint32_t moved = 0u;
    uint32_t i;

    if (hub == NULL || hub->region == NULL) {
        return 0u;
    }

    /* ---- Step 1: uplink (core -> PC) ---- */
    for (i = 0u; i < (uint32_t)MUX_SLOTS_CORES; i++) {
        uint32_t drained;

        for (drained = 0u; drained < (uint32_t)MUX_SLOTS_DEPTH; drained++) {
            uint8_t  payload[MUX_SLOTS_PAYLOAD_MAX];
            uint32_t channel = 0u;
            uint32_t plen    = 0u;

            if (MUX_SLOTS_HubPollUplink(hub->region, i, &channel,
                                          payload, sizeof(payload), &plen) == 0) {
                break; /* this core's uplink ring is empty */
            }

            {
                uint8_t  frame[SERIAL_MUX_MAX_FRAME + 8];
                uint32_t n;

                n = SERIAL_MUX_Encode((uint8_t)channel, payload, plen,
                                      frame, sizeof(frame));
                if (n > 0u && hub->uart_write != NULL) {
                    hub->uart_write(frame, n, hub->uart_user);
                    hub->up_frames++;
                    moved++;
                }
            }
        }
    }

    /* ---- Step 2: downlink (PC -> core) ---- */
    if (hub->uart_read != NULL) {
        uint8_t  rxbuf[MUX_HUB_RX_CHUNK];
        uint32_t before = hub->down_frames + hub->down_dropped;
        uint32_t got;

        got = hub->uart_read(rxbuf, sizeof(rxbuf), hub->uart_user);
        if (got > 0u) {
            SERIAL_MUX_RxFeed(&hub->rx, rxbuf, got);
        }

        moved += (hub->down_frames + hub->down_dropped) - before;
    }

    return moved;
}
