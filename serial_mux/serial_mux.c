/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * serial_mux.c - Portable COBS channel-framing MUX core implementation.
 *
 * Wire framing (identical to the PC-side scripts):
 *     [ COBS( <channel_byte> + <payload...> ) ] 0x00
 *
 * All comments are ASCII-only English per workspace convention.
 */

#include "serial_mux.h"

/* COBS decode of src[0..n) (delimiter 0x00 NOT included) into dst[0..dst_cap).
 * Returns decoded length, or 0 on malformed input / overflow. */
static uint32_t cobs_decode(const uint8_t *src, uint32_t n,
                            uint8_t *dst, uint32_t dst_cap)
{
    uint32_t si = 0, di = 0;
    while (si < n) {
        uint8_t code = src[si++];
        if (code == 0) {
            return 0;                    /* 0x00 must not appear inside a frame */
        }
        for (uint8_t i = 1; i < code; i++) {
            if (si >= n || di >= dst_cap) {
                return 0;
            }
            dst[di++] = src[si++];
        }
        if (code < 0xFF && si < n) {
            if (di >= dst_cap) {
                return 0;
            }
            dst[di++] = 0;
        }
    }
    return di;
}

void SERIAL_MUX_RxInit(serial_mux_rx_t *rx,
                        serial_mux_deliver_fn deliver,
                        void *user)
{
    rx->len = 0;
    rx->deliver = deliver;
    rx->user = user;
}

void SERIAL_MUX_RxByte(serial_mux_rx_t *rx, uint8_t b)
{
    if (b == 0x00) {                     /* frame delimiter */
        if (rx->len > 0) {
            uint8_t out[SERIAL_MUX_MAX_FRAME];
            uint32_t olen = cobs_decode(rx->buf, rx->len, out, sizeof(out));
            if (olen >= 1 && rx->deliver != 0) {
                rx->deliver(out[0], out + 1, olen - 1, rx->user);
            }
        }
        rx->len = 0;                     /* reset for next frame */
    } else if (rx->len < SERIAL_MUX_MAX_FRAME) {
        rx->buf[rx->len++] = b;
    } else {
        rx->len = 0;                     /* overflow: drop and resync on 0x00 */
    }
}

void SERIAL_MUX_RxFeed(serial_mux_rx_t *rx, const uint8_t *data, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        SERIAL_MUX_RxByte(rx, data[i]);
    }
}

uint32_t SERIAL_MUX_Encode(uint8_t channel,
                           const uint8_t *payload, uint32_t len,
                           uint8_t *out, uint32_t out_cap)
{
    /* Build [channel][payload] into tmp, COBS-encode into out, append 0x00. */
    uint8_t tmp[SERIAL_MUX_MAX_FRAME];
    uint32_t tlen = 0;

    if (1u + len > sizeof(tmp)) {
        return 0;                        /* payload too large for one frame */
    }
    tmp[tlen++] = channel;
    for (uint32_t i = 0; i < len; i++) {
        tmp[tlen++] = payload[i];
    }

    uint32_t oi = 0, start = 0;
    while (start <= tlen) {
        uint32_t end = start;
        uint8_t code = 1;
        while (end < tlen && tmp[end] != 0 && code < 0xFF) {
            end++;
            code++;
        }
        if (oi >= out_cap) {
            return 0;
        }
        out[oi++] = code;
        for (uint32_t k = start; k < end; k++) {
            if (oi >= out_cap) {
                return 0;
            }
            out[oi++] = tmp[k];
        }
        if (end < tlen && tmp[end] == 0) {
            start = end + 1;
            if (start == tlen) {         /* trailing zero: emit final 0x01 group */
                if (oi < out_cap) {
                    out[oi++] = 1;
                }
                break;
            }
        } else {
            start = end;
            if (start >= tlen) {
                break;
            }
        }
    }
    if (oi >= out_cap) {
        return 0;
    }
    out[oi++] = 0x00;                    /* frame delimiter */
    return oi;
}
