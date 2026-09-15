/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * serial_mux.h - Portable COBS channel-framing MUX core (public API).
 *
 * Wire framing (identical to the PC-side scripts):
 *     [ COBS( <channel_byte> + <payload...> ) ] 0x00
 *
 * All comments are ASCII-only English per workspace convention.
 */

#ifndef SERIAL_MUX_H
#define SERIAL_MUX_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Max decoded frame size (channel byte + payload) handled at once. */
#ifndef SERIAL_MUX_MAX_FRAME
#define SERIAL_MUX_MAX_FRAME 256u
#endif

/* Called by the RX assembler once a full frame is decoded from the wire. */
typedef void (*serial_mux_deliver_fn)(uint8_t channel,
                                      const uint8_t *payload,
                                      uint32_t len,
                                      void *user);

/* RX assembler state: accumulates wire bytes until a 0x00 delimiter. */
typedef struct {
    uint8_t               buf[SERIAL_MUX_MAX_FRAME];
    uint32_t              len;
    serial_mux_deliver_fn deliver;
    void                 *user;
} serial_mux_rx_t;

/* Initialize an RX assembler with a delivery callback and user context. */
void SERIAL_MUX_RxInit(serial_mux_rx_t *rx,
                        serial_mux_deliver_fn deliver,
                        void *user);

/* Feed one received wire byte. */
void SERIAL_MUX_RxByte(serial_mux_rx_t *rx, uint8_t b);

/* Feed a buffer of received wire bytes. */
void SERIAL_MUX_RxFeed(serial_mux_rx_t *rx, const uint8_t *data, uint32_t n);

/*
 * Encode (channel, payload) into a wire frame in 'out'. Returns the number of
 * bytes written (including the trailing 0x00 delimiter), or 0 on overflow.
 */
uint32_t SERIAL_MUX_Encode(uint8_t channel,
                           const uint8_t *payload, uint32_t len,
                           uint8_t *out, uint32_t out_cap);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_MUX_H */
