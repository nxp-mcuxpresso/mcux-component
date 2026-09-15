/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Serial MUX transport plug-in interface.
 *
 * The MUX hub core owns one physical link that carries the COBS wire stream
 * (COBS(<channel_byte> + <payload>) 0x00). Historically that link was always
 * an LPUART. This header abstracts the link behind a small ops table so the
 * hub can drive ANY byte transport - LPUART, USB-CDC, SPI, a socket on a
 * Linux gateway, or an application-supplied custom device - without any
 * change to the transport-neutral mux core (mux_hub.c / mux_client.c).
 *
 * Writing a transport backend:
 *   1. Define a private context struct for the backend's state.
 *   2. Fill a static const mux_transport_ops_t with your callbacks.
 *   3. Provide a constructor mux_transport_<name>(cfg) returning a pointer to
 *      a static const mux_transport_t bundling ops + ctx.
 *
 * Contract:
 *   - open()    is called once before any I/O; returns 0 on success.
 *   - write()   queues/sends output bytes; must not block indefinitely.
 *   - read()    pulls up to cap input bytes, non-blocking, returns count.
 *   - service() is called from the hub poll loop to drain queued TX / pump
 *               the link; may be NULL if the backend is self-draining.
 *   - close()   tears the link down; may be NULL.
 */

#ifndef MUX_TRANSPORT_H_
#define MUX_TRANSPORT_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Transport operations table. Every callback receives the backend's
 * own @p ctx pointer. write() and read() are mandatory; service() and
 * close() may be NULL.
 */
typedef struct _mux_transport_ops
{
    /*! One-time link bring-up at @p baud (ignored by link-less transports).
     *  Return 0 on success, non-zero on error. */
    int (*open)(void *ctx, uint32_t baud);

    /*! Queue @p n output bytes for transmission. Non-blocking on a full
     *  software buffer is backend-defined (LPUART spins on TDRE). */
    void (*write)(void *ctx, const uint8_t *data, uint32_t n);

    /*! Pull up to @p cap input bytes into @p buf. Non-blocking. Returns the
     *  number of bytes produced (0 if none pending). */
    uint32_t (*read)(void *ctx, uint8_t *buf, uint32_t cap);

    /*! Pump the link once (drain queued TX, poll hardware). Called each hub
     *  poll iteration. Returns bytes moved, or 0. May be NULL. */
    uint32_t (*service)(void *ctx);

    /*! Link tear-down. May be NULL. */
    void (*close)(void *ctx);
} mux_transport_ops_t;

/*!
 * @brief A bound transport instance: an ops table plus its private context.
 * Backend constructors return a pointer to a static const one of these.
 */
typedef struct _mux_transport
{
    const mux_transport_ops_t *ops;
    void                      *ctx;
    const char                *name; /*!< Human-readable id, for diagnostics. */
} mux_transport_t;

#if defined(__cplusplus)
}
#endif

#endif /* MUX_TRANSPORT_H_ */
