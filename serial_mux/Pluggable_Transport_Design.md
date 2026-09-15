# Serial MUX Pluggable Transport Design

## Motivation

The Serial MUX hub multiplexes several logical byte streams (channels) onto a
single physical link using COBS channel framing:

```
[ COBS(<channel_byte> + <payload>) ] 0x00
```

Historically the hub's physical link was hard-wired to an LPUART. This design
makes the underlying transport hardware pluggable so the same mux core can run
over LPUART, USB-CDC, SPI, a socket on a Linux gateway, or any
application-supplied byte device - with no change to the mux core.

## Layering

```
+-------------------------------------------------------------+
|  Application / demo (console_mux, hello_world, ...)         |
+-------------------------------------------------------------+
|  Client console front end (optional)                        |
|    components/uart/fsl_adapter_muxuart.c                    |
|    - HAL UART adapter over the mux client API, so PRINTF /   |
|      GETCHAR on a non-hub core travel as a virtual channel   |
+-------------------------------------------------------------+
|  MUX_UART_Hub* compat shim  (mux_uart_transport_hub.c)      |  optional
|    - one-call hub bring-up, keeps the original API          |
+-------------------------------------------------------------+
|  Mux core (transport-neutral)                               |
|    hub:    MUX_HUB_Init(hub, region, write_fn, read_fn,     |
|                         user) / MUX_HUB_SetSelfHandler()    |
|            MUX_HUB_Poll(hub)                                |
|    client: MUX_CLIENT_Attach() / Send() / Recv() /          |
|            RecvBlocking()                                   |
|  mux_hub.c / mux_client.c / mux_slots.c / serial_mux.c      |
+-------------------------------------------------------------+
|  Transport abstraction  (mux_transport.h)                   |
|    mux_transport_ops_t { open, write, read, service, close }|
|    mux_transport_t     { ops, ctx, name }                   |
+-------------------------------------------------------------+
|  Concrete backend                                           |
|    mux_transport_lpuart.c  (the only in-tree backend)       |
|    <your backend>          (USB-CDC / SPI / socket / ...)   |
+-------------------------------------------------------------+
```

The key observation is that the mux core was already transport-neutral: the hub
takes `write_fn` / `read_fn` hooks plus an opaque `user` pointer in
`MUX_HUB_Init()`, and the client side never touches a UART at all - it only
moves payloads through the shared-memory SPSC slots. The only UART coupling
lived in the transport layer. This change formalizes that coupling behind a
small ops table.

## The transport interface (`mux_transport.h`)

```c
typedef struct _mux_transport_ops {
    int      (*open)(void *ctx, uint32_t baud);            /* once, before I/O   */
    void     (*write)(void *ctx, const uint8_t *d, uint32_t n); /* queue TX bytes */
    uint32_t (*read)(void *ctx, uint8_t *buf, uint32_t cap);    /* non-blocking RX */
    uint32_t (*service)(void *ctx);   /* pump link each poll; may be NULL          */
    void     (*close)(void *ctx);     /* tear down; may be NULL                    */
} mux_transport_ops_t;

typedef struct _mux_transport {
    const mux_transport_ops_t *ops;
    void                      *ctx;   /* backend-private state                    */
    const char                *name;  /* diagnostics                              */
} mux_transport_t;
```

Contract:
- `open()` is called once before any I/O; returns 0 on success.
- `write()` queues output bytes; must not block indefinitely.
- `read()` pulls up to `cap` bytes, non-blocking, returns the count.
- `service()` is called from the hub poll loop to drain queued TX / pump the
  link; may be NULL when the backend is self-draining.
- `close()` tears the link down; may be NULL.

`write()` and `read()` are mandatory; `open()`, `service()` and `close()` may be
NULL when the backend does not need them.

## Writing a transport backend

1. Define a private context struct for the backend's state.
2. Fill a `static const mux_transport_ops_t` with your callbacks.
3. Provide a constructor `mux_transport_<name>(cfg)` that returns a pointer to
   a `static const mux_transport_t` bundling ops + ctx.
4. In the application, construct your backend and pass its write/read hooks to
   `MUX_HUB_Init()` directly (bypassing the LPUART compat shim). Call your
   backend's `service()` alongside `MUX_HUB_Poll()` in the main loop.

The LPUART backend in `mux_transport_lpuart.c` is the reference example: it owns
a software TX ring (drained on TDRE), a software RX ring (filled from the
RX-full ISR), and hides `fsl_lpuart.h` from the mux core and the compat shims.

Only the HUB side sees a transport. A client core needs no backend at all, so
adding a new transport never touches the client sources.

## Kconfig

The component is split into role sub-options so a build compiles only the roles
it plays. `MCUX_COMPONENT_component.serial_mux` enables the arch-neutral COBS
core; inside it `.slots`, `.hub`, `.client` and `.transport` each default to `y`
and can be disabled individually (a client-only image, for instance, leaves
`.hub` and `.transport` off and pulls in no LPUART code).

Transport backends are selected through an independent bool group rather than a
single-select `choice`, so more than one backend can be enabled at once. A hub
instance is bound to exactly one physical link, so enabling several backends
lets an application create one hub instance per link and hand each a different
backend - that is how multiple backends coexist at runtime. Additional backends
(USB-CDC, SPI, socket, etc.) can be added later as their own `MUX_TRANSPORT_*`
bool without disturbing the others.

```
config MUX_TRANSPORT_LPUART   # LPUART backend (default y); the only in-tree backend today
    bool "LPUART transport backend"
    depends on MCUX_COMPONENT_component.serial_mux.transport
    default y
```

`mux_transport.h` is a header-only contract and always available with the
component; the LPUART backend source is compiled only when
`MUX_TRANSPORT_LPUART` is enabled. Leaving `MUX_TRANSPORT_LPUART` disabled is
the custom case: no in-tree backend is compiled and the application supplies its
own `mux_transport_t`.

Two other options shape how the wire is used, independently of which backend
carries it:

- `MUX_HUB_DEDICATED_CONSOLE` (default `n`) - when `n` the hub core sends its
  own console as one more logical channel on the SAME mux link; when `y` the hub
  keeps a second, dedicated physical UART for its console and the mux link
  carries only the client cores' channels.
- `MUX_CLIENT_BLOCKING_RECV` (default `y`) - makes `MUX_CLIENT_RecvBlocking()`
  spin until a downlink byte arrives, so a console reader on top of the client
  gets real UART `GETCHAR` semantics instead of a no-data sentinel. The
  non-blocking `MUX_CLIENT_Recv()` stays available for poll-style callers.

## Backward compatibility

`mux_uart_transport.h` / `mux_uart_transport_hub.c` remain as thin shims: the
`MUX_UART_HubInit/Service/Emit/Irq` one-call API now builds an LPUART backend
and wires it into `MUX_HUB_Init()`. Existing demos (e.g. `console_mux`) build
unchanged.

On the client side the console front end is the `muxuart` HAL UART adapter
(`components/uart/fsl_adapter_muxuart.c`, Kconfig
`MCUX_COMPONENT_component.muxuart_adapter`). It implements the six `HAL_Uart*`
strong symbols on top of `MUX_CLIENT_Send()` / `MUX_CLIENT_RecvBlocking()`, so a
non-hub core keeps using the normal SDK debug console and its PRINTF / GETCHAR
travel over the mux. Because only one uart adapter may define those symbols per
image, a client image selects `muxuart_adapter` instead of `lpuart_adapter`.

## Verification

- `console_mux` on imx943evk builds clean for all three cores
  (cm33_core1 hub, cm7_core0 and cm7_core1 clients; armgcc debug) with the
  default LPUART backend. The hub image links `fsl_adapter_lpuart.c.obj`, the
  two client images link `fsl_adapter_muxuart.c.obj`.
- `hello_world` on imx943evk cm7_core0 / cm7_core1 builds clean in muxuart mode,
  showing an unmodified demo can be moved onto a mux channel by prj.conf alone.
- Validated on imx943evk hardware: all three consoles multiplexed over the
  single hub LPUART, with interactive input reaching each core.
