/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Serial MUX UART transport - backward-compatible convenience layer.
 *
 * This header is the original one-call bring-up API (MUX_UART_HubInit /
 * HubService / HubEmit / HubIrq / ClientAttach). It is now a THIN SHIM on top
 * of the pluggable transport abstraction in mux_transport.h: the hub side
 * drives a mux_transport_t (the LPUART backend by default), and the shim keeps
 * the same struct/function surface so existing demos build unchanged.
 *
 * To use a NON-LPUART transport, bypass this shim and call MUX_HUB_Init()
 * directly with your own mux_transport_t's write/read/service hooks (see the
 * "Writing a transport backend" section in the component README).
 */

#ifndef MUX_UART_TRANSPORT_H_
#define MUX_UART_TRANSPORT_H_

#include "fsl_lpuart.h"
#include "serial_mux.h"
#include "mux_slots.h"
#include "mux_hub.h"
#include "mux_client.h"
#include "mux_transport.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Software ring sizes for the hub-side LPUART transport (Kconfig tunables). */
#ifndef MUX_UART_TX_RING_SIZE
#define MUX_UART_TX_RING_SIZE 1024u
#endif
#ifndef MUX_UART_RX_RING_SIZE
#define MUX_UART_RX_RING_SIZE 512u
#endif

/* ---- LPUART transport backend (concrete mux_transport_t) ---------------- */

/*!
 * @brief Board wiring for the LPUART transport backend. This is the only
 * place that names a physical LPUART instance; the backend hides it behind
 * the generic mux_transport_t ops table.
 */
typedef struct _mux_transport_lpuart_cfg
{
    LPUART_Type *uart;     /*!< LPUART instance carrying the COBS wire.  */
    IRQn_Type    irqn;     /*!< That LPUART's IRQ number (RX-full).      */
    uint32_t     clk_freq; /*!< LPUART source clock in Hz.               */
} mux_transport_lpuart_cfg_t;

/*!
 * @brief Construct the LPUART transport backend from board wiring.
 * @return Backend pointer to hand to the hub, or NULL on invalid config.
 */
const mux_transport_t *mux_transport_lpuart(const mux_transport_lpuart_cfg_t *cfg);

/*! RX ISR body for the LPUART backend; pumps one byte into its RX ring. */
void mux_transport_lpuart_rx_isr(void);

/* ---- Backward-compatible one-call hub/client bring-up ------------------- */

/*!
 * @brief Hub bring-up config (unchanged surface). Names the physical LPUART,
 * its clock and baud, plus the shared slot region and the hub's own channel.
 */
typedef struct
{
    LPUART_Type        *uart;
    IRQn_Type           irqn;
    uint32_t            clk_freq;
    uint32_t            baud;
    mux_slots_region_t *region;
    uint8_t             self_channel;
} mux_uart_hub_cfg_t;

/*!
 * @brief Hub transport instance. Wraps the transport-neutral mux_hub_t and
 * keeps a pointer to the bound transport backend (LPUART by default).
 */
typedef struct
{
    mux_hub_t              hub;
    const mux_transport_t *transport;
    uint8_t                self_channel;
} mux_uart_hub_t;

void     MUX_UART_HubInit(mux_uart_hub_t *t, const mux_uart_hub_cfg_t *cfg);
uint32_t MUX_UART_HubService(mux_uart_hub_t *t);
void     MUX_UART_HubEmit(mux_uart_hub_t *t, const char *fmt, ...);
void     MUX_UART_HubIrq(mux_uart_hub_t *t);

#if defined(__cplusplus)
}
#endif

#endif /* MUX_UART_TRANSPORT_H_ */
