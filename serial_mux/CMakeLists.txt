# Copyright 2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

# Portable COBS channel-framing MUX core (arch-neutral, no dependencies).
if (CONFIG_MCUX_COMPONENT_component.serial_mux)
    mcux_add_source(
        SOURCES serial_mux.c
                serial_mux.h
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

# Shared-memory SPSC slot IPC used between the hub core and non-hub cores.
if (CONFIG_MCUX_COMPONENT_component.serial_mux.slots)
    mcux_add_source(
        SOURCES mux_slots.c
                mux_slots.h
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

# Hub role: owns the physical UART, does COBS encode/decode and routing.
if (CONFIG_MCUX_COMPONENT_component.serial_mux.hub)
    mcux_add_source(
        SOURCES mux_hub.c
                mux_hub.h
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

# Client role: non-hub core, exchanges payloads with the hub via the slots.
# Transport-neutral - carries the generic client handshake (mux_client), which
# depends only on the slots IPC, never on any UART/driver.
if (CONFIG_MCUX_COMPONENT_component.serial_mux.client)
    mcux_add_source(
        SOURCES mux_client.c
                mux_client.h
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

# Transport abstraction (transport-neutral): ONLY the generic mux_transport_t
# ops-table contract. This header carries NO knowledge of any concrete physical
# link - no UART, no driver dependency - so an application can implement its own
# mux_transport_t backend against it without pulling in anything below.
if (CONFIG_MCUX_COMPONENT_component.serial_mux.transport)
    mcux_add_source(
        SOURCES mux_transport.h
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

# Concrete transport backends. Each backend is selected by its OWN independent
# MUX_TRANSPORT_* option (no longer a single-select choice), so more backends
# (e.g. I2C, SPI) can be added later and compiled alongside LPUART. For now the
# LPUART backend is the only one. EVERYTHING UART-specific lives here and is
# compiled only when MUX_TRANSPORT_LPUART is on: the LPUART mux_transport_t
# implementation (mux_transport_lpuart.c, the only file that pulls in
# fsl_lpuart.h), the UART convenience header (mux_uart_transport.h, which also
# includes fsl_lpuart.h) and the one-call hub bring-up glue
# (mux_uart_transport_hub.c, built on top of mux_uart_transport.h). With this
# option off, no UART driver dependency is dragged in. The hub glue is compiled
# only for the hub role (dotted CONFIG names are not valid C macros). The
# client-side console binding is NOT here - it is transport-neutral and lives in
# the client-role block above.
if (CONFIG_MUX_TRANSPORT_LPUART)
    mcux_add_source(
        SOURCES mux_transport_lpuart.c
                mux_uart_transport.h
    )
    mcux_add_include(
        INCLUDES .
    )

    if (CONFIG_MCUX_COMPONENT_component.serial_mux.hub)
        mcux_add_source(
            SOURCES mux_uart_transport_hub.c
        )
    endif()
endif()
