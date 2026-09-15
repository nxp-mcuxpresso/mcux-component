/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * mux_slots.c - Alternative B shared-memory slot IPC (no MU doorbell).
 *
 * Implements the SPSC ring handoff declared in mux_slots.h. The only
 * cross-core synchronization is the per-ring head/tail counters plus memory
 * barriers; there is no lock and no interrupt/doorbell.
 *
 * imx943evk instance: channels are 0-based. core_index i maps directly to
 * channel i (cm7_core0 -> channel 0, cm7_core1 -> channel 1).
 *
 * All comments are ASCII-only English per workspace convention.
 */

#include "mux_slots.h"

#include <string.h>

/*
 * fsl_common.h pulls in the CMSIS core header (__DMB). fsl_cache.h provides
 * the per-core D-cache maintenance wrappers DCACHE_CleanByRange /
 * DCACHE_InvalidateByRange, which route to the correct cache controller for
 * each core: on imx943 the cm33 hub uses the system XCACHE controller
 * (XCACHE_CleanCacheByRange / XCACHE_InvalidateCacheByRange) while the cm7
 * clients use their own internal Cortex-M7 L1 D-cache. The build selects the
 * matching fsl_cache.h per core, so the same source works on both. If your
 * build does not use the MCUXpresso SDK, replace these with the appropriate
 * CMSIS / cache-controller calls.
 */
#include "fsl_common.h"
#include "fsl_cache.h"


/*
 * Portable full memory barrier. On Cortex-M / Cortex-A the GCC/Clang builtin
 * maps to a DMB. If your toolchain lacks it, provide a CMSIS __DMB() here.
 */
#if defined(__GNUC__) || defined(__clang__)
#define MUX_SLOTS_BARRIER() __atomic_thread_fence(__ATOMIC_SEQ_CST)
#else
#define MUX_SLOTS_BARRIER() do { /* insert platform DMB here */ } while (0)
#endif

/*
 * Explicit D-cache maintenance for the shared region. On imx943 the region is
 * cacheable and NOT hardware-coherent between the cm33 hub and the cm7
 * clients, so a plain barrier (compiler/CPU ordering fence) is not enough: a
 * producer's write can sit in its own dirty D-cache while the consumer keeps
 * reading its own stale cached copy. We therefore clean (write back) after a
 * producer write and invalidate before a consumer read, by address.
 *
 * head and tail live on separate cache lines (see mux_slots.h) so cleaning one
 * counter never writes back a stale copy of the other.
 *
 * We deliberately use the SDK's per-core DCACHE_CleanByRange /
 * DCACHE_InvalidateByRange wrappers instead of the CMSIS SCB_*DCache_by_Addr
 * calls: on the cm33 hub the shared-region lines are held by the system
 * XCACHE controller (NOT the internal Cortex-M33 SCB D-cache), so the SCB
 * calls would not maintain them. DCACHE_CleanByRange / DCACHE_InvalidateByRange
 * resolve to XCACHE_CleanCacheByRange / XCACHE_InvalidateCacheByRange on the
 * cm33 XCACHE build and to the internal Cortex-M7 L1 D-cache maintenance on
 * the cm7 clients, so the same source is correct on every core.
 */
#define MUX_SLOTS_CLEAN(addr, size)      \
    DCACHE_CleanByRange((uint32_t)(uintptr_t)(addr), (uint32_t)(size))
#define MUX_SLOTS_INVALIDATE(addr, size) \
    DCACHE_InvalidateByRange((uint32_t)(uintptr_t)(addr), (uint32_t)(size))


/* MUX_SLOTS_DEPTH must be a power of two for this mask to work. */
#define MUX_SLOTS_INDEX_MASK (MUX_SLOTS_DEPTH - 1u)


void MUX_SLOTS_Attach(mux_slots_region_t *region)
{
    /* Nothing to do: the region is shared and mapped by the platform. */
    (void)region;
}

void MUX_SLOTS_HubInit(mux_slots_region_t *region)
{
    uint32_t i;

    if (region == NULL) {
        return;
    }

    /* Reset every ring to empty before publishing the magic. */
    for (i = 0; i < MUX_SLOTS_CORES; i++) {
        region->uplink[i].head   = 0u;
        region->uplink[i].tail   = 0u;
        region->downlink[i].head = 0u;
        region->downlink[i].tail = 0u;
    }

    region->cores       = MUX_SLOTS_CORES;
    region->payload_max = MUX_SLOTS_PAYLOAD_MAX;
    region->depth       = MUX_SLOTS_DEPTH;

    /* Clean the whole freshly-initialized region back to main memory so the
     * cm7 clients (separate D-cache, non-coherent) see the zeroed rings and
     * the magic once we publish it, instead of stale/uninitialized lines. */
    MUX_SLOTS_BARRIER();
    MUX_SLOTS_CLEAN(region, sizeof(*region));

    /* Publish magic last: it is the "region is ready" signal. */
    MUX_SLOTS_BARRIER();
    region->magic = MUX_SLOTS_MAGIC;
    MUX_SLOTS_BARRIER();
    /* Write the magic line back so waiting clients can observe it. */
    MUX_SLOTS_CLEAN(&region->magic, sizeof(region->magic));
}

int MUX_SLOTS_Ready(const mux_slots_region_t *region)
{
    if (region == NULL) {
        return 0;
    }
    /* Invalidate the magic line so we read the hub's published value rather
     * than a stale cached copy from before the hub initialized the region. */
    MUX_SLOTS_INVALIDATE(&region->magic, sizeof(region->magic));
    MUX_SLOTS_BARRIER();
    return (region->magic == MUX_SLOTS_MAGIC) ? 1 : 0;
}


int MUX_SLOTS_ChannelToCore(uint32_t channel)
{
    /* 0-based: channel c (0..CORES-1) owns core index c directly. */
    if (channel < (uint32_t)MUX_SLOTS_CORES) {
        return (int)channel;
    }
    return -1;
}

/* Common producer enqueue into a ring. Returns 1 on success, 0 if full/bad. */
static int ring_enqueue(mux_ring_t *ring,
                        uint32_t channel,
                        const uint8_t *payload, uint32_t len)
{
    uint32_t head;
    uint32_t tail;
    mux_slot_t *slot;

    if (ring == NULL) {
        return 0;
    }
    if (len > MUX_SLOTS_PAYLOAD_MAX) {
        return 0;
    }
    if (len > 0u && payload == NULL) {
        return 0;
    }

    tail = ring->tail;
    head = ring->head;

    /* Ring full: the consumer has not drained enough messages yet. */
    if ((uint32_t)(tail - head) >= MUX_SLOTS_DEPTH) {
        return 0;
    }

    slot = &ring->slot[tail & MUX_SLOTS_INDEX_MASK];
    slot->channel = channel;
    slot->len     = len;
    if (len > 0u) {
        memcpy((void *)slot->data, payload, len);
    }

    /* Clean the written slot back to main memory BEFORE publishing the new
     * tail, so the consumer (other core, non-coherent D-cache) cannot observe
     * an advanced tail while the payload still sits in our dirty cache. */
    MUX_SLOTS_BARRIER();
    MUX_SLOTS_CLEAN(slot, sizeof(*slot));

    /* Ensure the slot payload is fully visible before the new tail is. */
    MUX_SLOTS_BARRIER();
    ring->tail = tail + 1u;
    MUX_SLOTS_BARRIER();
    /* Write the tail counter line back so the consumer sees the new tail.
     * tail is on its own cache line (see mux_slots.h), so this cannot clobber
     * a concurrent consumer update of head. */
    MUX_SLOTS_CLEAN(&ring->tail, sizeof(ring->tail));
    return 1;
}


/* Common consumer dequeue from a ring. Returns bytes copied, 0 if empty/bad. */
static uint32_t ring_dequeue(mux_ring_t *ring,
                             uint32_t *channel,
                             uint8_t *out, uint32_t out_cap)
{
    uint32_t head;
    uint32_t tail;
    uint32_t len;
    mux_slot_t *slot;

    if (ring == NULL || out == NULL) {
        return 0;
    }

    /* Invalidate the producer-owned tail line before reading it, so we see
     * the producer's latest tail rather than our own stale cached copy. tail
     * is on its own cache line, so this does not disturb our head. */
    MUX_SLOTS_INVALIDATE(&ring->tail, sizeof(ring->tail));
    MUX_SLOTS_BARRIER();

    head = ring->head;
    tail = ring->tail;

    /* Ring empty. */
    if (head == tail) {
        return 0;
    }

    /* Non-empty observed: order payload reads after the tail read. */
    MUX_SLOTS_BARRIER();

    slot = &ring->slot[head & MUX_SLOTS_INDEX_MASK];

    /* Invalidate the slot line before reading so we pull the producer's
     * freshly-written payload from main memory, not a stale cached copy. */
    MUX_SLOTS_INVALIDATE(slot, sizeof(*slot));
    MUX_SLOTS_BARRIER();

    len = slot->len;
    if (len > out_cap) {
        len = out_cap;
    }
    if (len > 0u) {
        memcpy(out, (const void *)slot->data, len);
    }
    if (channel != NULL) {
        *channel = slot->channel;
    }

    /* Release the slot only after we finished copying the payload out. */
    MUX_SLOTS_BARRIER();
    ring->head = head + 1u;
    MUX_SLOTS_BARRIER();
    /* Write the head counter line back so the producer sees the slot freed.
     * head is on its own cache line, so this cannot clobber the producer's
     * tail update. */
    MUX_SLOTS_CLEAN(&ring->head, sizeof(ring->head));
    return len;
}


int MUX_SLOTS_UplinkTrySend(mux_slots_region_t *region,
                              uint32_t core_index,
                              const uint8_t *payload, uint32_t len)
{
    if (region == NULL || core_index >= MUX_SLOTS_CORES) {
        return 0;
    }
    /* 0-based: uplink channel equals core_index (core 0 -> channel 0). */
    return ring_enqueue(&region->uplink[core_index],
                        core_index, payload, len);
}

uint32_t MUX_SLOTS_DownlinkTryRecv(mux_slots_region_t *region,
                                     uint32_t core_index,
                                     uint8_t *out, uint32_t out_cap)
{
    if (region == NULL || core_index >= MUX_SLOTS_CORES) {
        return 0;
    }
    return ring_dequeue(&region->downlink[core_index], NULL, out, out_cap);
}

int MUX_SLOTS_HubPollUplink(mux_slots_region_t *region,
                              uint32_t core_index,
                              uint32_t *channel,
                              uint8_t *out, uint32_t out_cap,
                              uint32_t *out_len)
{
    mux_ring_t *ring;
    uint32_t n;

    if (region == NULL || core_index >= MUX_SLOTS_CORES) {
        return 0;
    }

    ring = &region->uplink[core_index];

    /* Invalidate the producer-owned tail line BEFORE the fast-path empty
     * check. Otherwise the hub keeps reading its own stale cached tail
     * (==0 from region init) and never observes a client's newly enqueued
     * frame (e.g. the one-shot boot announce) until unrelated cache activity
     * happens to refresh the line. tail is on its own cache line, so this
     * does not disturb our head. */
    MUX_SLOTS_INVALIDATE(&ring->tail, sizeof(ring->tail));
    MUX_SLOTS_BARRIER();

    /* Empty ring: nothing to hand up this pass. */
    if (ring->head == ring->tail) {
        return 0;
    }


    n = ring_dequeue(ring, channel, out, out_cap);
    if (out_len != NULL) {
        *out_len = n;
    }
    return 1;
}

int MUX_SLOTS_HubTryDownlink(mux_slots_region_t *region,
                               uint32_t core_index,
                               uint32_t channel,
                               const uint8_t *payload, uint32_t len)
{
    if (region == NULL || core_index >= MUX_SLOTS_CORES) {
        return 0;
    }
    return ring_enqueue(&region->downlink[core_index], channel, payload, len);
}
