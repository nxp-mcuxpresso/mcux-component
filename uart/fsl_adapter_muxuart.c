/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * fsl_adapter_muxuart.c - serial_mux client exposed as a HAL UART adapter.
 *
 * Role: CLIENT ONLY. This adapter belongs on a non-hub core that owns no
 * physical UART. It implements the standard HAL_Uart* contract on top of a
 * serial_mux client (mux_client.h), so any HAL_Uart consumer - most notably
 * utility.debug_console_lite - runs unmodified on such a core: PRINTF and
 * GETCHAR travel through the shared-memory slots to the hub core, which owns
 * the physical UART and does the COBS framing.
 *
 * Do NOT enable this adapter on the hub core. The hub keeps using its transport
 * backend (mux_transport_lpuart.c) straight on the LPUART driver. If the hub
 * also compiled this file, HAL_UartSendBlocking would recurse into the mux, and
 * the image would carry two definitions of the same strong symbols (only one
 * uart adapter may be linked per image).
 *
 * Configuration:
 *   MUX_REGION_ADDR           shared slot region base (serial_mux Kconfig,
 *                             mandatory - no default)
 *   uart_config->instance     used as the client's 0-based core index, i.e. the
 *                             logical mux channel for this core
 *   HAL_UART_MUX_HANDLE_COUNT number of concurrently open mux "UARTs"
 *                             (default 1)
 *   HAL_UART_MUX_INIT_RETRIES bounded number of hub-availability polls in
 *                             HAL_UartInit; 0 (default) means wait forever
 *
 * Only the blocking HAL_Uart entry points are implemented, because a mux client
 * has no UART interrupt of its own to drive the transfer/DMA API. The adapter is
 * therefore for blocking-only consumers such as utility.debug_console_lite; a
 * build guard below rejects the interrupt/DMA transfer modes instead of failing
 * at link time.
 *
 * HAL_UartInit is assumed to be called from a single context (typically board
 * init before any RTOS is running): the client-pool scan below is not atomic,
 * so two concurrent HAL_UartInit calls could otherwise claim the same slot.
 *
 * srcClock_Hz / baudRate_Bps from hal_uart_config_t are intentionally ignored:
 * the wire is owned by the hub, so a client cannot and must not program a baud
 * rate. Parity/stop-bit settings are equally meaningless here.
 *
 * All comments are ASCII-only English per workspace convention.
 */

#include <assert.h>

#include "fsl_adapter_uart.h"

#include "mux_client.h"
#include "mux_slots.h"

#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U)) || \
    (defined(HAL_UART_TRANSFER_MODE) && (HAL_UART_TRANSFER_MODE > 0U)) ||                 \
    (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
#error "muxuart_adapter implements the blocking HAL_Uart API only; disable UART transfer/DMA mode"
#endif

#ifndef HAL_UART_MUX_HANDLE_COUNT
#define HAL_UART_MUX_HANDLE_COUNT (1U)
#endif

/*
 * Bounded wait for the hub to publish the shared region. 0 keeps the original
 * behaviour (wait forever), which is what a board whose hub always boots first
 * wants; a non-zero value lets HAL_UartInit report kStatus_HAL_UartError
 * instead of hanging the core when the hub never comes up.
 */
#ifndef HAL_UART_MUX_INIT_RETRIES
#define HAL_UART_MUX_INIT_RETRIES (0U)
#endif

#if !defined(MUX_REGION_ADDR)
#error "MUX_REGION_ADDR must be set (serial_mux Kconfig) for the muxuart adapter"
#endif

#define HAL_UART_MUX_SLOT_NONE (0xFFU)

/*
 * The caller-supplied handle can be as small as two pointers
 * (HAL_UART_HANDLE_SIZE), so only a pool index is stored there; the
 * mux_client_t instances live in this file.
 */
typedef struct
{
    uint8_t slot;
} hal_uart_mux_state_t;

static mux_client_t s_muxClients[HAL_UART_MUX_HANDLE_COUNT];
static bool s_muxClientUsed[HAL_UART_MUX_HANDLE_COUNT];

static mux_client_t *HAL_UartMuxClient(hal_uart_handle_t handle)
{
    hal_uart_mux_state_t *state;

    if (NULL == handle)
    {
        return NULL;
    }

    state = (hal_uart_mux_state_t *)handle;
    if (state->slot >= (uint8_t)HAL_UART_MUX_HANDLE_COUNT)
    {
        return NULL;
    }
    if (!s_muxClientUsed[state->slot])
    {
        return NULL;
    }

    return &s_muxClients[state->slot];
}

hal_uart_status_t HAL_UartInit(hal_uart_handle_t handle, const hal_uart_config_t *uart_config)
{
    hal_uart_mux_state_t *state;
    uint32_t i;

    assert(NULL != handle);
    assert(NULL != uart_config);
    assert(HAL_UART_HANDLE_SIZE >= sizeof(hal_uart_mux_state_t));

    if ((NULL == handle) || (NULL == uart_config))
    {
        return kStatus_HAL_UartError;
    }

    for (i = 0U; i < (uint32_t)HAL_UART_MUX_HANDLE_COUNT; i++)
    {
        if (!s_muxClientUsed[i])
        {
            break;
        }
    }
    if (i >= (uint32_t)HAL_UART_MUX_HANDLE_COUNT)
    {
        /* No free mux client: raise HAL_UART_MUX_HANDLE_COUNT. */
        return kStatus_HAL_UartError;
    }

    /*
     * Bind to the hub-published region and wait until the hub is up. The
     * instance field carries the 0-based core index (logical mux channel).
     * HAL_UART_MUX_INIT_RETRIES == 0 waits forever (MUX_CLIENT_Attach), any
     * other value bounds the wait so a missing hub is reported, not hung on.
     */
#if (HAL_UART_MUX_INIT_RETRIES == 0U)
    MUX_CLIENT_Attach(&s_muxClients[i], (mux_slots_region_t *)(MUX_REGION_ADDR),
                      (uint32_t)uart_config->instance);
#else
    {
        uint32_t retries = (uint32_t)HAL_UART_MUX_INIT_RETRIES;

        MUX_CLIENT_Init(&s_muxClients[i], (mux_slots_region_t *)(MUX_REGION_ADDR),
                        (uint32_t)uart_config->instance);

        while (0 == MUX_CLIENT_Ready(&s_muxClients[i]))
        {
            if (0U == --retries)
            {
                /* Hub never published the region: leave the pool slot free. */
                return kStatus_HAL_UartError;
            }
        }
    }
#endif

    s_muxClientUsed[i] = true;

    state       = (hal_uart_mux_state_t *)handle;
    state->slot = (uint8_t)i;

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartDeinit(hal_uart_handle_t handle)
{
    hal_uart_mux_state_t *state;

    if (NULL == HAL_UartMuxClient(handle))
    {
        return kStatus_HAL_UartError;
    }

    state                          = (hal_uart_mux_state_t *)handle;
    s_muxClientUsed[state->slot]   = false;
    state->slot                    = HAL_UART_MUX_SLOT_NONE;

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartSendBlocking(hal_uart_handle_t handle, const uint8_t *data, size_t length)
{
    mux_client_t *cli = HAL_UartMuxClient(handle);

    if ((NULL == cli) || (NULL == data))
    {
        return kStatus_HAL_UartError;
    }
    if (0U == length)
    {
        return kStatus_HAL_UartSuccess;
    }

    /*
     * A slot payload is capped at MUX_SLOTS_PAYLOAD_MAX, and the slot layer
     * rejects anything longer outright (same 0 return as "slot full"), so the
     * transfer is chunked here. Without the chunking a single PRINTF longer
     * than the payload limit would spin forever on a request that can never be
     * accepted. Within a chunk, 0 really does mean "uplink slot still full",
     * so spinning until the hub drains it gives the caller the blocking-send
     * semantics the HAL contract promises.
     */
    while (0U != length)
    {
        size_t chunk = length;

        if (chunk > (size_t)MUX_SLOTS_PAYLOAD_MAX)
        {
            chunk = (size_t)MUX_SLOTS_PAYLOAD_MAX;
        }

        while (0 == MUX_CLIENT_Send(cli, data, (uint32_t)chunk))
        {
        }

        data   += chunk;
        length -= chunk;
    }

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartReceiveBlocking(hal_uart_handle_t handle, uint8_t *data, size_t length)
{
    mux_client_t *cli = HAL_UartMuxClient(handle);
    size_t done       = 0U;

    if ((NULL == cli) || (NULL == data))
    {
        return kStatus_HAL_UartError;
    }

    while (done < length)
    {
        uint32_t got = MUX_CLIENT_RecvBlocking(cli, &data[done], (uint32_t)(length - done));

        if (0U == got)
        {
            return kStatus_HAL_UartError;
        }
        done += (size_t)got;
    }

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartEnterLowpower(hal_uart_handle_t handle)
{
    /* A client owns no UART clock or pins, so there is nothing to gate. */
    if (NULL == HAL_UartMuxClient(handle))
    {
        return kStatus_HAL_UartError;
    }

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartExitLowpower(hal_uart_handle_t handle)
{
    if (NULL == HAL_UartMuxClient(handle))
    {
        return kStatus_HAL_UartError;
    }

    return kStatus_HAL_UartSuccess;
}
