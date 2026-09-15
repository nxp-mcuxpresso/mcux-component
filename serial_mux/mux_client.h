/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * mux_client.h - Alternative B non-hub core client (imx943evk cm7 cores).
 *
 * A thin wrapper over the polling slot IPC (mux_slots.h) for a single non-hub
 * core. On imx943evk each cm7 core binds itself to one 0-based core_index
 * (cm7_core0 -> 0, cm7_core1 -> 1) and then uses send/recv without touching
 * any UART: the hub on cm33_core1 owns UART-MUX (LPUART11) and does all COBS
 * encode/decode + routing.
 *
 * Usage on a cm7 core with core_index k:
 *   mux_client_t cli;
 *   MUX_CLIENT_Init(&cli, shared_region, k);        // core_index = k (0-based)
 *   while (!MUX_CLIENT_Ready(&cli)) { ... }          // wait for hub init
 *   MUX_CLIENT_Send(&cli, payload, len);             // uplink to PC
 *   n = MUX_CLIENT_Recv(&cli, buf, sizeof(buf));     // downlink from PC
 *
 * All comments are ASCII-only English per workspace convention.
 */

#ifndef MUX_CLIENT_H
#define MUX_CLIENT_H

#include <stdint.h>
#include <stddef.h>

#include "mux_slots.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    mux_slots_region_t *region;      /* shared slot region (mapped by platform) */
    uint32_t            core_index;  /* 0-based: cm7_core0 -> 0, cm7_core1 -> 1  */
    uint32_t            tx_sent;      /* uplink frames accepted by the slot      */
    uint32_t            tx_busy;      /* uplink attempts rejected (slot full)    */
    uint32_t            rx_recv;      /* downlink frames taken from the slot     */
    /*
     * Residual of the downlink frame currently being consumed. A caller may ask
     * for fewer bytes than the frame carries (console getchar asks for 1), so
     * the frame is dequeued whole into rx_buf and handed out incrementally.
     * Without this the tail of every multi-byte frame would be dropped when the
     * slot is released.
     */
    uint8_t             rx_buf[MUX_SLOTS_PAYLOAD_MAX];
    uint32_t            rx_len;       /* valid bytes in rx_buf                   */
    uint32_t            rx_pos;       /* next byte of rx_buf to hand out         */
} mux_client_t;

/*
 * Bind this core to a shared region and its 0-based core index. The region
 * must be the same physical memory the hub published with MUX_SLOTS_HubInit().
 */
void MUX_CLIENT_Init(mux_client_t *cli,
                     mux_slots_region_t *region,
                     uint32_t core_index);

/* Non-zero once the hub has published a valid region (safe to send/recv). */
int MUX_CLIENT_Ready(const mux_client_t *cli);

/*
 * Generic one-call client handshake: bind this core to the shared region and
 * its 0-based core index (MUX_CLIENT_Init), then spin until the hub has
 * published the region (MUX_CLIENT_Ready). Backend- and console-neutral; any
 * client-side attach (console binding, raw data transport, ...) builds on this
 * to avoid re-implementing the busy-wait. Returns once the client is ready.
 */
void MUX_CLIENT_Attach(mux_client_t *cli,
                       mux_slots_region_t *region,
                       uint32_t core_index);

/*
 * Uplink: try to send one payload to the PC (via the hub). Returns 1 on
 * success, 0 if the uplink slot is still full (hub has not consumed the last
 * message) or on bad args. Non-blocking; the channel is implied by core_index.
 */
int MUX_CLIENT_Send(mux_client_t *cli, const uint8_t *payload, uint32_t len);

/*
 * Downlink: try to receive payload addressed to this core. Returns the number
 * of bytes copied into 'out' (<= out_cap), or 0 if nothing is pending.
 * Non-blocking.
 *
 * A frame is dequeued from the slot as a whole (the slot is released back to
 * the hub right away) and then handed to the caller across as many calls as
 * needed, so asking for fewer bytes than the frame carries never loses data;
 * the remainder is served from the client's residual buffer on the next call.
 */
uint32_t MUX_CLIENT_Recv(mux_client_t *cli, uint8_t *out, uint32_t out_cap);

/*
 * Downlink (blocking): receive payload addressed to this core, spinning
 * until at least one byte is available. Residual bytes left over from a
 * previously dequeued frame are returned first, exactly like MUX_CLIENT_Recv.
 * Returns the number of bytes copied into 'out' (always >= 1, <= out_cap), or
 * 0 only on bad args. Use this for
 * console getchar semantics where the caller wants to wait for input rather
 * than poll; the non-blocking MUX_CLIENT_Recv above stays available for
 * poll-style callers.
 */
uint32_t MUX_CLIENT_RecvBlocking(mux_client_t *cli, uint8_t *out, uint32_t out_cap);

#ifdef __cplusplus
}
#endif

#endif /* MUX_CLIENT_H */
