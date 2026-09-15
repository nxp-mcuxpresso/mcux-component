/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * mux_slots.h - Alternative B shared-memory slot IPC (no MU doorbell).
 *
 * imx943evk instance: MUX_SLOTS_CORES == 2 (cm7_core0 -> channel 0,
 * cm7_core1 -> channel 1). The hub (cm33_core1) owns UART-MUX (LPUART11).
 * Channel numbering is 0-based here: core_index i maps directly to channel i.
 *
 * Each ring is single-producer single-consumer (SPSC). Two counters per ring
 * are the only cross-core handshake: producer owns 'tail', consumer owns
 * 'head'. Empty when head == tail, full when tail - head == MUX_SLOTS_DEPTH.
 * Ordering is enforced with memory barriers (see mux_slots.c); no lock and no
 * doorbell interrupt are used.
 *
 * The region must live in memory shared and coherent across all cores; map the
 * SAME physical address into every core's view.
 *
 * All comments are ASCII-only English per workspace convention.
 */

#ifndef MUX_SLOTS_H
#define MUX_SLOTS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Number of non-hub cores served by the hub (cm7_core0, cm7_core1). */
#if !defined(MUX_SLOTS_CORES)
#error "MUX_SLOTS_CORES is not set. Configure CONFIG_MUX_SLOTS_CORES (number of non-hub client cores served by the hub) for this board."
#endif

/* Max payload bytes carried in one slot. Keep below the peer max-payload. */
#if !defined(MUX_SLOTS_PAYLOAD_MAX)
#error "MUX_SLOTS_PAYLOAD_MAX is not set. Configure CONFIG_MUX_SLOTS_PAYLOAD_MAX (max payload bytes carried in one slot) for this board."
#endif

/*
 * Ring depth per direction per core. Depth 1 == the original single-slot
 * scheme; a small depth (4/8) lets short bursts through. Must be a power of
 * two so the head/tail counters wrap cleanly with a bit mask.
 */
#if !defined(MUX_SLOTS_DEPTH)
#error "MUX_SLOTS_DEPTH is not set. Configure CONFIG_MUX_SLOTS_DEPTH (SPSC ring depth per direction per core, a power of two) for this board."
#endif

/* Magic so a core can sanity-check the region was initialized by the hub. */
#define MUX_SLOTS_MAGIC 0x424C5453u /* "SLTB" little-endian-ish tag */

/*
 * MUX_REGION_ADDR is the base address of the shared slot region. It is
 * mandatory and has deliberately NO default: it must resolve to the SAME
 * physical memory on every core (hub and clients) and be reserved from all
 * their linker maps, which only the integrator knows. Set
 * CONFIG_MUX_REGION_ADDR in the application; the serial_mux Kconfig emits
 * the bare MUX_REGION_ADDR macro. A private-RAM fallback is NOT provided,
 * because it would not be shared across cores and would fail silently at
 * run time; force a build error here instead.
 */
#if !defined(MUX_REGION_ADDR)
#error "MUX_REGION_ADDR is not set. Configure CONFIG_MUX_REGION_ADDR (serial_mux slot-region base address) for this board."
#endif

/*
 * D-cache line size in bytes used to keep the producer-owned 'tail' and the
 * consumer-owned 'head' on SEPARATE cache lines. On imx943 the shared region
 * is cacheable and hardware-non-coherent, so mux_slots.c does explicit clean /
 * invalidate by address. If head and tail shared one line, a producer clean of
 * 'tail' would write back a stale 'head' (and vice versa), clobbering the peer.
 * 32 bytes covers the Cortex-M7 (cm7 clients) and Cortex-M33 (cm33 hub) lines.
 */
#if !defined(MUX_SLOTS_CACHELINE)
#error "MUX_SLOTS_CACHELINE is not set. Configure CONFIG_MUX_SLOTS_CACHELINE (D-cache line size in bytes for counter/slot padding) for this board."
#endif

/*
 * Compiler-portable alignment attribute. Every cross-core structure that is
 * cache-maintained by address MUST start on and occupy a whole number of
 * cache lines, otherwise a clean/invalidate of one counter would touch a
 * neighbor line and clobber the peer core's data on the non-coherent bus.
 */
#if defined(__GNUC__) || defined(__clang__)
#define MUX_SLOTS_ALIGNED __attribute__((aligned(MUX_SLOTS_CACHELINE)))
#elif defined(__ICCARM__)
#define MUX_SLOTS_ALIGNED _Pragma("data_alignment=32")
#else
#define MUX_SLOTS_ALIGNED
#endif


/*
 * One buffered message. Layout is fixed so every core agrees on it
 * byte-for-byte.
 */
typedef struct {
    volatile uint32_t channel;                /* logical mux channel (0-based) */
    volatile uint32_t len;                    /* payload length in bytes       */
    volatile uint8_t  data[MUX_SLOTS_PAYLOAD_MAX];
    /* Pad so sizeof(mux_slot_t) is a whole number of cache lines. The two
     * uint32_t plus MUX_SLOTS_PAYLOAD_MAX(240) == 248; pad to 256 (8*32). */
    uint8_t           pad_slot[MUX_SLOTS_CACHELINE -
        ((2u * sizeof(uint32_t) + MUX_SLOTS_PAYLOAD_MAX) % MUX_SLOTS_CACHELINE)];
} mux_slot_t;

/*
 * One direction of one core's mailbox: an SPSC ring of MUX_SLOTS_DEPTH slots.
 * 'tail' advanced only by the producer, 'head' only by the consumer; indexed
 * with (& (MUX_SLOTS_DEPTH - 1)). Counters MUST be accessed with barriers.
 *
 * 'head' and 'tail' are padded onto SEPARATE cache lines. The consumer cleans
 * the 'head' line and the producer cleans the 'tail' line; on a non-coherent
 * cache (imx943) sharing one line would let one core's writeback clobber the
 * other core's counter. Each pad reserves a full cache line for its counter.
 */
typedef struct {
    volatile uint32_t head;                   /* consumer read count  */
    uint8_t           pad_head[MUX_SLOTS_CACHELINE - sizeof(uint32_t)];
    volatile uint32_t tail;                   /* producer write count */
    uint8_t           pad_tail[MUX_SLOTS_CACHELINE - sizeof(uint32_t)];
    mux_slot_t        slot[MUX_SLOTS_DEPTH];
} MUX_SLOTS_ALIGNED mux_ring_t;


/*
 * The whole shared region: a header plus per-core uplink and downlink rings.
 * Index i in [0, MUX_SLOTS_CORES) maps to non-hub core i and channel i.
 */
typedef struct {
    volatile uint32_t magic;                  /* MUX_SLOTS_MAGIC once ready */
    volatile uint32_t cores;                  /* MUX_SLOTS_CORES            */
    volatile uint32_t payload_max;            /* MUX_SLOTS_PAYLOAD_MAX      */
    volatile uint32_t depth;                  /* MUX_SLOTS_DEPTH            */
    /* Pad the header to a full cache line so uplink[0] (and thus every ring)
     * starts on its own cache-line boundary. */
    uint8_t           pad_hdr[MUX_SLOTS_CACHELINE - 4u * sizeof(uint32_t)];
    mux_ring_t        uplink[MUX_SLOTS_CORES];   /* core(i) -> hub  */
    mux_ring_t        downlink[MUX_SLOTS_CORES]; /* hub    -> core(i) */
} MUX_SLOTS_ALIGNED mux_slots_region_t;

/* Attach a core to an already-mapped shared region. Does not modify it. */
void MUX_SLOTS_Attach(mux_slots_region_t *region);

/*
 * Hub-only: initialize the whole region (all rings empty) and publish the
 * magic. Call exactly once from the hub before any core sends.
 */
void MUX_SLOTS_HubInit(mux_slots_region_t *region);

/* Non-hub: returns non-zero once the hub has published a valid region. */
int MUX_SLOTS_Ready(const mux_slots_region_t *region);

/* ---- Non-hub core API ---------------------------------------------- */

/*
 * Try to enqueue an uplink payload for this core (0-based core_index).
 * Returns 1 on success, 0 if the uplink ring is full or on bad args.
 */
int MUX_SLOTS_UplinkTrySend(mux_slots_region_t *region,
                              uint32_t core_index,
                              const uint8_t *payload, uint32_t len);

/*
 * Try to dequeue a downlink payload addressed to this core. Returns bytes
 * copied into 'out' (<= out_cap), or 0 if nothing pending / bad args.
 */
uint32_t MUX_SLOTS_DownlinkTryRecv(mux_slots_region_t *region,
                                     uint32_t core_index,
                                     uint8_t *out, uint32_t out_cap);

/* ---- Hub API ------------------------------------------------------- */

/*
 * Poll one uplink ring. Returns 1 and fills channel, out and out_len if core
 * core_index has a pending uplink message; releases the slot. Returns 0 if
 * that ring is empty.
 */
int MUX_SLOTS_HubPollUplink(mux_slots_region_t *region,
                              uint32_t core_index,
                              uint32_t *channel,
                              uint8_t *out, uint32_t out_cap,
                              uint32_t *out_len);

/*
 * Hub -> core: try to enqueue a downlink payload to core core_index. Returns 1
 * on success, 0 if that core's downlink ring is full or on bad args.
 */
int MUX_SLOTS_HubTryDownlink(mux_slots_region_t *region,
                               uint32_t core_index,
                               uint32_t channel,
                               const uint8_t *payload, uint32_t len);

/* Map a 0-based logical mux channel (0..MUX_SLOTS_CORES-1) to a 0-based core
 * index, or return -1 if the channel has no owning core. */
int MUX_SLOTS_ChannelToCore(uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* MUX_SLOTS_H */
