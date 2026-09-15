/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * mux_client.c - Alternative B non-hub core client (imx943evk cm7 cores).
 *
 * Implements mux_client.h on top of the polling slot IPC (mux_slots.h). A
 * client never touches any UART; it only reads/writes its own two slots in the
 * shared region. The hub on cm33_core1 polls those slots and owns UART-MUX.
 *
 * All comments are ASCII-only English per workspace convention.
 */

#include "mux_client.h"

void MUX_CLIENT_Init(mux_client_t *cli,
                     mux_slots_region_t *region,
                     uint32_t core_index)
{
    if (cli == NULL) {
        return;
    }

    cli->region     = region;
    cli->core_index = core_index;
    cli->tx_sent    = 0u;
    cli->tx_busy    = 0u;
    cli->rx_recv    = 0u;
    cli->rx_len     = 0u;
    cli->rx_pos     = 0u;

    /* Attach is a no-op today but keeps API symmetry with the hub side. */
    MUX_SLOTS_Attach(region);
}

int MUX_CLIENT_Ready(const mux_client_t *cli)
{
    if (cli == NULL) {
        return 0;
    }
    return MUX_SLOTS_Ready(cli->region);
}

void MUX_CLIENT_Attach(mux_client_t *cli,
                       mux_slots_region_t *region,
                       uint32_t core_index)
{
    MUX_CLIENT_Init(cli, region, core_index);

    /* Spin until the hub has published the region. Each MUX_CLIENT_Ready pass
     * performs the cross-core read, so the loop always observes the hub's
     * publish and is never optimized away; the empty body keeps this layer
     * free of any CPU/CMSIS dependency. */
    while (!MUX_CLIENT_Ready(cli)) {
    }
}

int MUX_CLIENT_Send(mux_client_t *cli, const uint8_t *payload, uint32_t len)
{
    int ok;

    if (cli == NULL || cli->region == NULL) {
        return 0;
    }

    ok = MUX_SLOTS_UplinkTrySend(cli->region, cli->core_index, payload, len);
    if (ok != 0) {
        cli->tx_sent++;
    } else {
        cli->tx_busy++;
    }
    return ok;
}

/*
 * Hand out up to out_cap bytes of the residual frame. Returns 0 when the
 * residual buffer is empty.
 */
static uint32_t MUX_CLIENT_DrainResidual(mux_client_t *cli, uint8_t *out, uint32_t out_cap)
{
    uint32_t n = cli->rx_len - cli->rx_pos;
    uint32_t i;

    if ((n == 0u) || (out_cap == 0u)) {
        return 0u;
    }
    if (n > out_cap) {
        n = out_cap;
    }

    for (i = 0u; i < n; i++) {
        out[i] = cli->rx_buf[cli->rx_pos + i];
    }
    cli->rx_pos += n;

    if (cli->rx_pos >= cli->rx_len) {
        cli->rx_len = 0u;
        cli->rx_pos = 0u;
    }
    return n;
}

uint32_t MUX_CLIENT_Recv(mux_client_t *cli, uint8_t *out, uint32_t out_cap)
{
    uint32_t n;

    if (cli == NULL || cli->region == NULL || out == NULL) {
        return 0u;
    }

    /* Serve whatever is left of the frame currently being consumed first. */
    n = MUX_CLIENT_DrainResidual(cli, out, out_cap);
    if (n > 0u) {
        return n;
    }

    /*
     * Dequeue the next frame WHOLE into the residual buffer. The slot layer
     * releases the slot as soon as it is read, so a frame must never be
     * truncated there: taking it complete here is what keeps the tail of a
     * multi-byte frame alive for the following calls.
     */
    n = MUX_SLOTS_DownlinkTryRecv(cli->region, cli->core_index,
                                  cli->rx_buf, (uint32_t)MUX_SLOTS_PAYLOAD_MAX);
    if (n == 0u) {
        return 0u;
    }

    cli->rx_len = n;
    cli->rx_pos = 0u;
    cli->rx_recv++;

    return MUX_CLIENT_DrainResidual(cli, out, out_cap);
}

uint32_t MUX_CLIENT_RecvBlocking(mux_client_t *cli, uint8_t *out, uint32_t out_cap)
{
    uint32_t n;

    if (cli == NULL || cli->region == NULL || out == NULL || out_cap == 0u) {
        return 0u;
    }

    /* Spin until residual bytes or a freshly published frame are available.
     * MUX_CLIENT_Recv performs the cross-core cache maintenance on every pass,
     * so the loop always observes the hub's publish and is never optimized
     * away. */
    do {
        n = MUX_CLIENT_Recv(cli, out, out_cap);
    } while (n == 0u);

    return n;
}
