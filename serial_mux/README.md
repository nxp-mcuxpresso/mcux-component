# Serial MUX

Portable COBS channel-framing serial multiplexer. It multiplexes several
logical byte streams (channels) onto a single physical link. The wire framing
is identical to the PC-side scripts:

```
[ COBS(<channel_byte> + <payload>) ] 0x00
```

## Layers

- **Mux core** (`serial_mux.c`, `mux_hub.c`, `mux_client.c`, `mux_slots.c`) -
  transport-neutral. `MUX_HUB_Init(hub, region, write_fn, read_fn, user)` takes
  raw write/read hooks plus an opaque `user` pointer; `MUX_HUB_Poll(hub)` pumps
  it. The core never touches any hardware header.
- **Transport abstraction** (`mux_transport.h`) - a small ops table
  (`open/write/read/service/close`) plus a bound instance
  (`mux_transport_t { ops, ctx, name }`).
- **Concrete backend** (`mux_transport_lpuart.c`) - the only in-tree backend.
  It owns the LPUART software TX/RX rings and the RX-full ISR, hiding
  `fsl_lpuart.h` from the core and the compat shims.
- **Compat shim** (`mux_uart_transport.h`, `mux_uart_transport_hub.c`) - keeps
  the original `MUX_UART_Hub*` one-call bring-up API. It now builds an LPUART
  backend and wires its hooks into `MUX_HUB_Init()`.

## Kconfig

Backends are selected through an independent bool group rather than a
single-select `choice`, so more than one backend can be enabled at once (each
one drives a separate hub instance / physical link). Additional backends
(USB-CDC, SPI, socket, etc.) can be added later as their own `MUX_TRANSPORT_*`
bool without disturbing the others.

```
config MUX_TRANSPORT_LPUART   # LPUART backend (default y); the only in-tree backend today
    bool "LPUART transport backend"
    depends on MCUX_COMPONENT_component.serial_mux.transport
    default y
```

`CMakeLists.txt` always compiles `mux_transport.h`; the LPUART backend source is
compiled only when `MUX_TRANSPORT_LPUART` is enabled. Leaving
`MUX_TRANSPORT_LPUART` disabled is the custom case: no in-tree backend is
compiled and the application supplies its own `mux_transport_t`.

## Writing a transport backend

The mux core is transport-neutral, so carrying the COBS stream over any byte
link (USB-CDC, SPI, a socket on a Linux gateway, etc.) only means providing a
different backend. Steps:

1. Define a private context struct for the backend state.
2. Fill a `static const mux_transport_ops_t` with your callbacks:
   - `open(ctx, baud)`  - called once before any I/O; return 0 on success.
   - `write(ctx, d, n)` - queue output bytes; must not block indefinitely.
   - `read(ctx, buf, cap)` - pull up to `cap` bytes, non-blocking; return count.
   - `service(ctx)`     - called from the hub poll loop to drain TX / pump the
     link; may be `NULL` when the backend is self-draining.
   - `close(ctx)`       - tear the link down; may be `NULL`.
3. Provide a constructor `mux_transport_<name>(cfg)` that returns a pointer to a
   `static const mux_transport_t` bundling ops + ctx.
4. In the application, construct your backend and pass its write/read hooks to
   `MUX_HUB_Init()` directly. Call your backend's `service()` alongside
   `MUX_HUB_Poll()` in the main loop.

`mux_transport_lpuart.c` is the reference backend to copy from.

## Backward compatibility

Existing users of the `MUX_UART_HubInit/Service/Emit/Irq` one-call API build
unchanged: those entry points remain as thin shims over the LPUART backend.

See [`Pluggable_Transport_Design.md`](Pluggable_Transport_Design.md) for the full design note.
