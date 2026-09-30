/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Implementation of the EdgeLock V2X / FCE (Fast Crypto Engine) firmware
 * service driver.
 *
 * The FCE firmware service reuses the same enclave message framing as
 * ele_crypto and v2x_crypto, but the command opcodes and payloads follow the
 * PRIME / FCE chapter of the EdgeLock Secure Enclave User Guide. The MU
 * transport is driven directly with the generic MU hardware driver (fsl_mu).
 * See v2x_fce_internal.h for the wire format details.
 */

#include <assert.h>
#include <string.h>

#include "fsl_memory.h"


#include "v2x_fce.h"
#include "v2x_fce_internal.h"


/*
 * Optional debug tracing. Define V2X_FCE_DEBUG=1 (e.g. via the example build
 * flags) to print the raw request/response words and the exact validation
 * step that fails inside V2X_FCE_Transceive. Requires fsl_debug_console.
 */
#ifndef V2X_FCE_DEBUG
#define V2X_FCE_DEBUG 0
#endif

#if V2X_FCE_DEBUG
#include "fsl_debug_console.h"
#define V2X_FCE_TRACE(...) PRINTF(__VA_ARGS__)
#else
#define V2X_FCE_TRACE(...) \
    do                     \
    {                      \
    } while (0)
#endif


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Maximum number of 32-bit words in a request or response buffer handled here.
 * The largest S3MU message built below is LOAD_HMAC_PLAIN_KEY (0x25 words).
 * Round up to a comfortable fixed buffer for both requests and responses. */
#define V2X_FCE_MSG_MAX_WORDS (40u)

/* Buffers referenced by the DMA engine must be 64-bit aligned. */
#define V2X_FCE_ADDR_ALIGNED(p) ((((uint32_t)(uint8_t *)(p)) & 0x7u) == 0u)

/*
 * Completion status word the FCE writes back. This is the only DMA buffer the
 * driver owns: every other descriptor address is a caller buffer passed through
 * untouched. It is a plain variable so the application linker script places it
 * (TCM or DDR).
 */
SDK_ALIGN(static volatile uint32_t s_v2xFceStatus, 8u);

/* MCUX-90943: real interrupt-driven completion path.
 *
 * The V2X/FCE public API is synchronous. Historically the completion wait
 * busy-polled the MU RX-full flag, so the benchmark SIGNAL mode (which only set
 * the MU RX interrupt-enable bit) degenerated to polling. This block adds a
 * genuine interrupt path: the V2X-FH MU RX-full interrupt reaches the Cortex-M7
 * through IRQSTEER as Reserved968_IRQn (IRQ 952, IRQSTEER input 714, V2X-FH APSHE/SHE channel per DTS GIC_SPI 384).
 * EnableIRQ() in fsl_common_arm.h configures IRQSTEER automatically. When
 * interrupt mode is armed via V2X_FCE_SetWaitMode(), the MU header round-trip
 * in V2X_FCE_Transceive blocks in __WFI() until the ISR reports RX-full,
 * instead of busy-waiting the flag. */
/* MCUX-90943 / MCUX-91696: platform-independent interrupt completion.
 *
 * The completion state machine below is core-agnostic: it never names a
 * chip-specific IRQn enum or *_IRQHandler vector. The application installs
 * its own ISR (named for whatever V2X-FH MU line is routed to its core's
 * IRQSTEER) and calls V2X_FCE_HandleIRQ() from it; the caller passes the
 * matching IRQ number to V2X_FCE_SetWaitMode() so the driver can
 * EnableIRQ/DisableIRQ it. On i.MX943 that line is Reserved968_IRQn on the
 * Cortex-M7 (V2X-FH APSHE RX) and, where no dedicated SHE line reaches the
 * core IRQSTEER, the closest available V2X-FH MU line (e.g.
 * V2X_FH_MU_APCH1_IRQn on cm33_core1). If the interrupt never fires the
 * transceive wait loop still makes progress by re-checking the RX-full
 * flag, so the path degrades gracefully to busy-poll. */
static volatile uint32_t s_v2xFceIrqMode = 0u;   /* 1 = interrupt-driven wait */
static volatile uint32_t s_v2xFceDone    = 0u;   /* set by ISR on RX-full */
static MU_Type          *s_v2xFceIrqMu   = NULL; /* MU armed for interrupts */
static int32_t           s_v2xFceIrqn    = -1;   /* caller-supplied IRQ number */

/*!
 * @brief V2X/FCE MU RX-full completion hook, to be called from the
 *        application ISR that owns the V2X-FH MU RX interrupt vector.
 *
 * On MU RX-full it masks the RX interrupt and flags completion; the
 * transceive wait loop then drains the MU response registers. Safe to call
 * spuriously (it checks the RX-full flag first).
 */
void V2X_FCE_HandleIRQ(void)
{
    MU_Type *mu = s_v2xFceIrqMu;
    if (mu != NULL)
    {
        uint32_t flags = MU_GetStatusFlags(mu);
        if ((flags & (uint32_t)kMU_Rx0FullFlag) != 0u)
        {
            MU_DisableInterrupts(mu, (uint32_t)kMU_Rx0FullInterruptEnable);
            s_v2xFceDone = 1u;
        }
    }
    SDK_ISR_EXIT_BARRIER;
}


/* Address of a buffer as the FCE AHB-DMA engine sees it. A core-local TCM alias
 * is translated to the DMA view; SoC-global addresses pass through unchanged. */
#define V2X_FCE_DMA_ADDR(p) \
    MEMORY_ConvertMemoryMapAddress((uint32_t)(uintptr_t)(const void *)(p), kMEMORY_Local2DMA)

/* Address of the driver-owned status word as the FCE sees it. */
#define V2X_FCE_STATUS_DMA_ADDR() V2X_FCE_DMA_ADDR(&s_v2xFceStatus)

/* Maximum AES-GCM payload the FCE accepts (see PRIME / FCE Limitations). */
#define V2X_FCE_GCM_MAX_PAYLOAD (0x1fffc0u)

/*
 * FCE request-id register (completion / FIFO-drain mechanism).
 *
 * The reference PRIME/FCE library (secure_enclave sab_prime_api.c
 * read_req_id() / sab_prime_process_msg.c enqueue_ops()) does NOT poll the
 * GET_REQUEST_STATUS (0xC0) command to detect completion. Instead every
 * request descriptor sets the AUTO-POP flag (bit 1) so the firmware pops the
 * completed entry from its internal request FIFO, and the host watches the
 * firmware request-id register at mu_buf + PRIME_REQ_ID_OFFSET (0xFC) - i.e.
 * the slot-region base (MU + 0x8000) + 0xFC, which is slot D word 15. The
 * firmware advances this "consumer" request-id by PRIME_REQ_ID_DIFF (0x10)
 * for every request it completes; PUSH_REQUEST returns the matching
 * "producer" id (already advanced by 0x10).
 *
 * The previous implementation polled GET_REQUEST_STATUS, which reads a
 * request's status but does not advance the consumer index / free the FIFO
 * entry. After a few thousand back-to-back requests the firmware request FIFO
 * therefore filled and every subsequent PUSH_REQUEST returned an immediate
 * error, wedging the engine for all algorithms. Detecting completion via the
 * request-id register (matching the reference) keeps the FIFO drained.
 *
 * This driver is strictly synchronous (one outstanding request at a time), so
 * completion is simply "consumer request-id register == the id PUSH returned".
 */
#define V2X_FCE_REQ_ID_OFFSET (0xFCu) /* PRIME_REQ_ID_OFFSET within slot window */
#define V2X_FCE_REQ_ID_DIFF   (0x10u) /* req id advances by 0x10 per completed request */
#define V2X_FCE_REQ_ID_ADDR(mu) \
    ((volatile uint32_t *)((uintptr_t)(mu) + V2X_FCE_MU_BUF_OFFSET + V2X_FCE_REQ_ID_OFFSET))

/*
 * D-cache maintenance for the FCE DMA buffers. The FCE is a separate bus
 * master; the caller crypto buffers and the driver status word are ordinary
 * cacheable memory. On a core with a data cache (Cortex-M7 on i.MX943) clean
 * (flush) a buffer before handing it to the FCE and invalidate it before
 * reading FCE-written results. The Cortex-M33 has no data cache, so these
 * compile to nothing there.
 */
#if (defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U))
#define V2X_FCE_DCACHE 1
static void V2X_FCE_CacheClean(const void *p, uint32_t size)
{
    uint32_t line  = (uint32_t)__SCB_DCACHE_LINE_SIZE;
    uint32_t addr  = (uint32_t)(uintptr_t)p;
    uint32_t start = addr & ~(line - 1u);
    uint32_t end   = (addr + size + line - 1u) & ~(line - 1u);
    SCB_CleanDCache_by_Addr((uint32_t *)start, (int32_t)(end - start));
}

static void V2X_FCE_CacheInvalidate(const void *p, uint32_t size)
{
    uint32_t line  = (uint32_t)__SCB_DCACHE_LINE_SIZE;
    uint32_t addr  = (uint32_t)(uintptr_t)p;
    uint32_t start = addr & ~(line - 1u);
    uint32_t end   = (addr + size + line - 1u) & ~(line - 1u);
    SCB_InvalidateDCache_by_Addr((uint32_t *)start, (int32_t)(end - start));
}
#else
#define V2X_FCE_DCACHE 0
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static status_t V2X_FCE_Transceive(MU_Type *mu,
                                   const uint32_t *msg,
                                   uint32_t msgWords,
                                   uint32_t *resp,
                                   uint8_t command);

/*!
 * @brief Message CRC used by the plain-key load messages.
 *
 * Mirrors the reference PRIME crc_compute() (secure_enclave
 * sab_prime_api.c): the CRC is the XOR of every message word except the
 * final CRC word. The FCE firmware validates this field on the
 * LOAD_AES_PLAIN_KEY / LOAD_HMAC_PLAIN_KEY messages and drops the request
 * if it does not match.
 */
static uint32_t V2X_FCE_CrcCompute(const uint32_t *msg, uint32_t words)
{
    uint32_t crc = 0u;
    uint32_t i;

    for (i = 0u; (i + 1u) < words; i++)
    {
        crc ^= msg[i];
    }

    return crc;
}

static status_t V2X_FCE_PushWait(v2x_fce_handle_t *handle, uint8_t bitmap, uint32_t count);

/* MCUX-90943: fold multi-slot batching into the base crypto APIs. */
static status_t V2X_FCE_PushWait(v2x_fce_handle_t *handle, uint8_t bitmap, uint32_t count)
{
    uint32_t msg[V2X_FCE_GET_REQUEST_STATUS_SIZE];
    uint32_t resp[V2X_FCE_MSG_MAX_WORDS];
    uint32_t reqId[4];
    status_t status;
    uint32_t s, i;

    __DSB();
    msg[0] = V2X_FCE_CMD_HEADER((uint8_t)(V2X_FCE_CMD_PUSH_REQUEST | bitmap), V2X_FCE_PUSH_REQUEST_SIZE);
    status = V2X_FCE_Transceive(handle->mu, msg, V2X_FCE_PUSH_REQUEST_SIZE, resp,
                                (uint8_t)(V2X_FCE_CMD_PUSH_REQUEST | bitmap));
    if (status != kStatus_Success)
    {
        return status;
    }
    if ((resp[2] & 0xFu) != 0u)
    {
        V2X_FCE_TRACE("[FCE] PUSH rejected: req_id=0x%08x bitmap=0x%x\r\n", (unsigned)resp[2], (unsigned)bitmap);
        return kStatus_Fail;
    }
    for (s = 0u; s < count; s++)
    {
        reqId[s] = resp[2] - ((count - 1u - s) * V2X_FCE_REQ_ID_DIFF);
    }
    {
        volatile uint32_t *reqIdReg = V2X_FCE_REQ_ID_ADDR(handle->mu);
        uint32_t lastReqId          = reqId[count - 1u];
        bool     completed          = false;
        for (i = 0u; i < V2X_FCE_GCM_STATUS_RETRIES; i++)
        {
            if (*reqIdReg >= lastReqId)
            {
                completed = true;
                break;
            }
        }
        if (!completed)
        {
            V2X_FCE_TRACE("[FCE] multi-slot timeout: lastReqId=0x%08x\r\n", (unsigned)lastReqId);
            return kStatus_Timeout;
        }
    }
    __DSB();
    return kStatus_Success;
}

static status_t V2X_FCE_AesGcm(v2x_fce_handle_t *handle,
                               const v2x_fce_gcm_op_t *ops,
                               uint32_t count,
                               uint8_t requestId)
{
    uint32_t desc[V2X_FCE_GCM_DESC_WORDS];
    uint32_t s, i;
    uint8_t  bitmap = 0u;

    if ((handle == NULL) || (handle->mu == NULL) || (ops == NULL) || (count == 0u) || (count > 4u))
    {
        return kStatus_InvalidArgument;
    }

    /* Per-op validation. */
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_gcm_op_t *op = &ops[s];
        if ((op->iv == NULL) || (op->input == NULL) || (op->output == NULL) || (op->tag == NULL))
        {
            return kStatus_InvalidArgument;
        }
        if (op->keySlot >= V2X_FCE_AES_KEY_SLOTS)
        {
            return kStatus_InvalidArgument;
        }
        if (op->inputSize > V2X_FCE_GCM_MAX_PAYLOAD)
        {
            return kStatus_InvalidArgument;
        }
        if (!V2X_FCE_ADDR_ALIGNED(op->input) || !V2X_FCE_ADDR_ALIGNED(op->output) ||
            !V2X_FCE_ADDR_ALIGNED(op->tag))
        {
            return kStatus_InvalidArgument;
        }
        if ((op->aad != NULL) && !V2X_FCE_ADDR_ALIGNED(op->aad))
        {
            return kStatus_InvalidArgument;
        }
        /* AAD is MANDATORY (see the FCE prefetcher note): NULL/empty AAD makes
         * the request fail with status=0x29 rating=0x1229. */
        if ((op->aad == NULL) || (op->aadSize == 0u))
        {
            V2X_FCE_TRACE("[FCE] GCM slot %u: AAD is mandatory (aad=%p aadSize=%u).\r\n",
                          (unsigned)s, (void *)op->aad, (unsigned)op->aadSize);
            assert((op->aad != NULL) && (op->aadSize != 0u));
            return kStatus_InvalidArgument;
        }
    }

    /* FCE writes the shared status word only on error; seed to success once. */
    s_v2xFceStatus = (uint32_t)V2X_FCE_STATUS_SUCCESS;

    /* Step 1: build one Table 27 GCM descriptor per used slot A..D. */
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_gcm_op_t *op = &ops[s];
        volatile uint32_t *slot    = (volatile uint32_t *)V2X_FCE_SLOT_BASE(handle->mu, s);

        for (i = 0u; i < V2X_FCE_GCM_DESC_WORDS; i++)
        {
            desc[i] = 0u;
        }
        desc[0] = V2X_FCE_BUILD_HEADER((uint8_t)(V2X_FCE_MSG_TAG_CMD_BASE + 1u), requestId,
                                       V2X_FCE_GCM_DESC_WORDS, V2X_FCE_MSG_VERSION);
        desc[1] = V2X_FCE_STATUS_DMA_ADDR();
        desc[2] = (((uint32_t)op->ivSize & 0xFFu) << 24) |
                  (((uint32_t)op->keySlot & 0xFFu) << 16) |
                  (((uint32_t)op->tagSize & 0xFFu) << 8) |
                  ((V2X_FCE_REQ_FLAG_SIGNAL | V2X_FCE_REQ_FLAG_AUTO_POP) & 0xFFu);
        desc[3] = V2X_FCE_DMA_ADDR(op->tag);
        desc[4] = op->inputSize;
        desc[5] = V2X_FCE_DMA_ADDR(op->input);
        desc[6] = V2X_FCE_DMA_ADDR(op->output);
        for (i = 0u; (i < op->ivSize) && (i < 16u); i++)
        {
            desc[7u + (i / 4u)] |= ((uint32_t)op->iv[i]) << (8u * (i % 4u));
        }
        desc[11] = op->aadSize;
        desc[12] = V2X_FCE_DMA_ADDR(op->aad);
        for (i = 0u; i < V2X_FCE_GCM_DESC_WORDS; i++)
        {
            slot[i] = desc[i];
        }
        bitmap |= (uint8_t)(1u << s);
    }

#if V2X_FCE_DCACHE
    V2X_FCE_CacheClean((const void *)&s_v2xFceStatus, sizeof(s_v2xFceStatus));
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_gcm_op_t *op = &ops[s];
        if (op->inputSize != 0u)
        {
            V2X_FCE_CacheClean(op->input, op->inputSize);
            V2X_FCE_CacheInvalidate(op->output, op->inputSize);
        }
        if ((op->aad != NULL) && (op->aadSize != 0u))
        {
            V2X_FCE_CacheClean(op->aad, op->aadSize);
        }
        if (op->tagSize != 0u)
        {
            V2X_FCE_CacheInvalidate(op->tag, op->tagSize);
        }
    }
#endif

    if (V2X_FCE_PushWait(handle, bitmap, count) != kStatus_Success)
    {
        return kStatus_Fail;
    }

#if V2X_FCE_DCACHE
    V2X_FCE_CacheInvalidate((const void *)&s_v2xFceStatus, sizeof(s_v2xFceStatus));
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_gcm_op_t *op = &ops[s];
        if (op->inputSize != 0u)
        {
            V2X_FCE_CacheInvalidate(op->output, op->inputSize);
        }
        if (op->tagSize != 0u)
        {
            V2X_FCE_CacheInvalidate(op->tag, op->tagSize);
        }
    }
#endif

    if (V2X_FCE_RESP_STATUS(s_v2xFceStatus) != V2X_FCE_STATUS_SUCCESS)
    {
        V2X_FCE_TRACE("[FCE] GCM completion failed: status=0x%08x\r\n", (unsigned)s_v2xFceStatus);
        return kStatus_Fail;
    }
    return kStatus_Success;
}

status_t V2X_FCE_AesGcmEncrypt(v2x_fce_handle_t *handle, const v2x_fce_gcm_op_t *ops, uint32_t count)
{
    return V2X_FCE_AesGcm(handle, ops, count, V2X_FCE_REQ_AES_GCM_ENCRYPT);
}

status_t V2X_FCE_AesGcmDecrypt(v2x_fce_handle_t *handle, const v2x_fce_gcm_op_t *ops, uint32_t count)
{
    return V2X_FCE_AesGcm(handle, ops, count, V2X_FCE_REQ_AES_GCM_DECRYPT);
}

static status_t V2X_FCE_AesCbc(v2x_fce_handle_t *handle,
                               const v2x_fce_cbc_op_t *ops,
                               uint32_t count,
                               uint8_t requestId)
{
    uint32_t desc[V2X_FCE_CBC_DESC_WORDS];
    uint32_t s, i;
    uint8_t  bitmap = 0u;

    if ((handle == NULL) || (handle->mu == NULL) || (ops == NULL) || (count == 0u) || (count > 4u))
    {
        return kStatus_InvalidArgument;
    }
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_cbc_op_t *op = &ops[s];
        if ((op->iv == NULL) || (op->input == NULL) || (op->output == NULL))
        {
            return kStatus_InvalidArgument;
        }
        if (op->keySlot >= V2X_FCE_AES_KEY_SLOTS)
        {
            return kStatus_InvalidArgument;
        }
        if (!V2X_FCE_ADDR_ALIGNED(op->input) || !V2X_FCE_ADDR_ALIGNED(op->output))
        {
            return kStatus_InvalidArgument;
        }
    }

    s_v2xFceStatus = (uint32_t)V2X_FCE_STATUS_SUCCESS;

    for (s = 0u; s < count; s++)
    {
        const v2x_fce_cbc_op_t *op = &ops[s];
        volatile uint32_t *slot    = (volatile uint32_t *)V2X_FCE_SLOT_BASE(handle->mu, s);

        for (i = 0u; i < V2X_FCE_CBC_DESC_WORDS; i++)
        {
            desc[i] = 0u;
        }
        desc[0] = V2X_FCE_BUILD_HEADER((uint8_t)(V2X_FCE_MSG_TAG_CMD_BASE + 1u), requestId,
                                       V2X_FCE_CBC_DESC_WORDS, V2X_FCE_MSG_VERSION);
        desc[1] = V2X_FCE_STATUS_DMA_ADDR();
        desc[2] = (((uint32_t)op->keySlot & 0xFFu) << 16) |
                  ((V2X_FCE_REQ_FLAG_SIGNAL | V2X_FCE_REQ_FLAG_AUTO_POP) & 0xFFu);
        desc[3] = V2X_FCE_STATUS_DMA_ADDR();
        desc[4] = op->inputSize;
        desc[5] = V2X_FCE_DMA_ADDR(op->input);
        desc[6] = V2X_FCE_DMA_ADDR(op->output);
        for (i = 0u; (i < op->ivSize) && (i < 16u); i++)
        {
            desc[7u + (i / 4u)] |= ((uint32_t)op->iv[i]) << (8u * (i % 4u));
        }
        for (i = 0u; i < V2X_FCE_CBC_DESC_WORDS; i++)
        {
            slot[i] = desc[i];
        }
        bitmap |= (uint8_t)(1u << s);
    }

#if V2X_FCE_DCACHE
    V2X_FCE_CacheClean((const void *)&s_v2xFceStatus, sizeof(s_v2xFceStatus));
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_cbc_op_t *op = &ops[s];
        if (op->inputSize != 0u)
        {
            V2X_FCE_CacheClean(op->input, op->inputSize);
            V2X_FCE_CacheInvalidate(op->output, op->inputSize);
        }
    }
#endif

    if (V2X_FCE_PushWait(handle, bitmap, count) != kStatus_Success)
    {
        return kStatus_Fail;
    }

#if V2X_FCE_DCACHE
    V2X_FCE_CacheInvalidate((const void *)&s_v2xFceStatus, sizeof(s_v2xFceStatus));
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_cbc_op_t *op = &ops[s];
        if (op->inputSize != 0u)
        {
            V2X_FCE_CacheInvalidate(op->output, op->inputSize);
        }
    }
#endif

    if (V2X_FCE_RESP_STATUS(s_v2xFceStatus) != V2X_FCE_STATUS_SUCCESS)
    {
        V2X_FCE_TRACE("[FCE] CBC completion failed: status=0x%08x\r\n", (unsigned)s_v2xFceStatus);
        return kStatus_Fail;
    }
    return kStatus_Success;
}

status_t V2X_FCE_AesCbcEncrypt(v2x_fce_handle_t *handle, const v2x_fce_cbc_op_t *ops, uint32_t count)
{
    return V2X_FCE_AesCbc(handle, ops, count, V2X_FCE_REQ_AES_CBC_ENCRYPT);
}

status_t V2X_FCE_AesCbcDecrypt(v2x_fce_handle_t *handle, const v2x_fce_cbc_op_t *ops, uint32_t count)
{
    return V2X_FCE_AesCbc(handle, ops, count, V2X_FCE_REQ_AES_CBC_DECRYPT);
}

/* Shared HASH / HMAC authenticate. ops[] and count let callers batch up to 4
 * ops (each mapped to slot A..D) into ONE PUSH_REQUEST. keySlot/shaAlgo/input/
 * digest are taken per op from the caller descriptor; isHash comes from the
 * requestId. */
typedef struct _v2x_fce_auth_op
{
    uint8_t  keySlot;
    uint8_t  shaAlgo;
    const uint8_t *input;
    uint32_t inputSize;
    uint8_t *digest;
    uint32_t digestSize;
} v2x_fce_auth_op_t;

static status_t V2X_FCE_Authenticate(v2x_fce_handle_t *handle,
                                     uint8_t requestId,
                                     const v2x_fce_auth_op_t *ops,
                                     uint32_t count)
{
    uint32_t desc[V2X_FCE_AUTH_DESC_WORDS];
    uint32_t s, i;
    uint8_t  bitmap = 0u;
    bool     isHash = (requestId == V2X_FCE_REQ_HASH);

    if ((handle == NULL) || (handle->mu == NULL) || (ops == NULL) || (count == 0u) || (count > 4u))
    {
        return kStatus_InvalidArgument;
    }
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_auth_op_t *op = &ops[s];
        if ((op->input == NULL) || (op->digest == NULL))
        {
            return kStatus_InvalidArgument;
        }
        if (!isHash && (op->keySlot >= V2X_FCE_HMAC_KEY_SLOTS))
        {
            return kStatus_InvalidArgument;
        }
        if (!V2X_FCE_ADDR_ALIGNED(op->input) || !V2X_FCE_ADDR_ALIGNED(op->digest))
        {
            return kStatus_InvalidArgument;
        }
    }

    s_v2xFceStatus = (uint32_t)V2X_FCE_STATUS_SUCCESS;

    for (s = 0u; s < count; s++)
    {
        const v2x_fce_auth_op_t *op = &ops[s];
        volatile uint32_t *slot     = (volatile uint32_t *)V2X_FCE_SLOT_BASE(handle->mu, s);

        for (i = 0u; i < V2X_FCE_AUTH_DESC_WORDS; i++)
        {
            desc[i] = 0u;
        }
        desc[0] = V2X_FCE_BUILD_HEADER((uint8_t)(V2X_FCE_MSG_TAG_CMD_BASE + 1u), requestId,
                                       V2X_FCE_AUTH_DESC_WORDS, V2X_FCE_MSG_VERSION);
        desc[1] = V2X_FCE_STATUS_DMA_ADDR();
        desc[2] = (((uint32_t)op->shaAlgo & 0xFu) << 4) |
                  ((V2X_FCE_REQ_FLAG_SIGNAL | V2X_FCE_REQ_FLAG_AUTO_POP) & 0xFu);
        if (!isHash)
        {
            desc[2] |= ((uint32_t)op->keySlot << V2X_FCE_KEY_SLOT_SHIFT);
        }
        desc[3] = V2X_FCE_DMA_ADDR(op->digest);
        desc[4] = op->inputSize;
        desc[5] = V2X_FCE_DMA_ADDR(op->input);
        for (i = 0u; i < V2X_FCE_AUTH_DESC_WORDS; i++)
        {
            slot[i] = desc[i];
        }
        bitmap |= (uint8_t)(1u << s);
    }

#if V2X_FCE_DCACHE
    V2X_FCE_CacheClean((const void *)&s_v2xFceStatus, sizeof(s_v2xFceStatus));
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_auth_op_t *op = &ops[s];
        if (op->inputSize != 0u)
        {
            V2X_FCE_CacheClean(op->input, op->inputSize);
        }
        if (op->digestSize != 0u)
        {
            V2X_FCE_CacheClean(op->digest, op->digestSize);
            V2X_FCE_CacheInvalidate(op->digest, op->digestSize);
        }
    }
#endif

    if (V2X_FCE_PushWait(handle, bitmap, count) != kStatus_Success)
    {
        return kStatus_Fail;
    }

#if V2X_FCE_DCACHE
    V2X_FCE_CacheInvalidate((const void *)&s_v2xFceStatus, sizeof(s_v2xFceStatus));
    for (s = 0u; s < count; s++)
    {
        const v2x_fce_auth_op_t *op = &ops[s];
        if (op->digestSize != 0u)
        {
            V2X_FCE_CacheInvalidate(op->digest, op->digestSize);
        }
    }
#endif

    if (V2X_FCE_RESP_STATUS(s_v2xFceStatus) != V2X_FCE_STATUS_SUCCESS)
    {
        V2X_FCE_TRACE("[FCE] AUTH completion failed: status=0x%08x\r\n", (unsigned)s_v2xFceStatus);
        return kStatus_Fail;
    }
    return kStatus_Success;
}

static status_t V2X_FCE_AuthDispatch(v2x_fce_handle_t *handle,
                                     uint8_t requestId,
                                     const v2x_fce_hmac_op_t *hmacOps,
                                     const v2x_fce_hash_op_t *hashOps,
                                     uint32_t count)
{
    v2x_fce_auth_op_t aops[4];
    uint32_t s;

    if ((count == 0u) || (count > 4u))
    {
        return kStatus_InvalidArgument;
    }
    for (s = 0u; s < count; s++)
    {
        if (hmacOps != NULL)
        {
            aops[s].keySlot    = hmacOps[s].keySlot;
            aops[s].shaAlgo    = (uint8_t)hmacOps[s].shaAlgo;
            aops[s].input      = hmacOps[s].input;
            aops[s].inputSize  = hmacOps[s].inputSize;
            aops[s].digest     = hmacOps[s].digest;
            aops[s].digestSize = hmacOps[s].digestSize;
        }
        else
        {
            aops[s].keySlot    = 0u;
            aops[s].shaAlgo    = (uint8_t)hashOps[s].shaAlgo;
            aops[s].input      = hashOps[s].input;
            aops[s].inputSize  = hashOps[s].inputSize;
            aops[s].digest     = hashOps[s].digest;
            aops[s].digestSize = hashOps[s].digestSize;
        }
    }
    return V2X_FCE_Authenticate(handle, requestId, aops, count);
}

status_t V2X_FCE_HmacGenerate(v2x_fce_handle_t *handle, const v2x_fce_hmac_op_t *ops, uint32_t count)
{
    if (ops == NULL)
    {
        return kStatus_InvalidArgument;
    }
    return V2X_FCE_AuthDispatch(handle, V2X_FCE_REQ_HMAC_GEN, ops, NULL, count);
}

status_t V2X_FCE_HmacVerify(v2x_fce_handle_t *handle, const v2x_fce_hmac_op_t *ops, uint32_t count)
{
    if (ops == NULL)
    {
        return kStatus_InvalidArgument;
    }
    return V2X_FCE_AuthDispatch(handle, V2X_FCE_REQ_HMAC_VERIFY, ops, NULL, count);
}

status_t V2X_FCE_HashGenerate(v2x_fce_handle_t *handle, const v2x_fce_hash_op_t *ops, uint32_t count)
{
    if (ops == NULL)
    {
        return kStatus_InvalidArgument;
    }
    return V2X_FCE_AuthDispatch(handle, V2X_FCE_REQ_HASH, NULL, ops, count);
}

status_t V2X_FCE_GetInfo(v2x_fce_handle_t *handle, uint8_t *buffer, uint32_t size)
{
    uint32_t msg[V2X_FCE_GET_INFO_SIZE];
    uint32_t resp[V2X_FCE_MSG_MAX_WORDS];
    status_t status;

    if ((handle == NULL) || (handle->mu == NULL) || (buffer == NULL))
    {
        return kStatus_InvalidArgument;
    }
    /* Need room for the version triplet and FIFO entry count. */
    if (size < 8u)
    {
        return kStatus_InvalidArgument;
    }

    /* Table 9: header only. */
    msg[0] = V2X_FCE_CMD_HEADER(V2X_FCE_CMD_GET_INFO, V2X_FCE_GET_INFO_SIZE);

    status = V2X_FCE_Transceive(handle->mu, msg, V2X_FCE_GET_INFO_SIZE, resp, V2X_FCE_CMD_GET_INFO);
    if (status == kStatus_Success)
    {
        /*
         * Table 10 response:
         *   W2 = [0 | VERSION MAJOR | VERSION MINOR | VERSION PATCH]
         *   W3 = Number of FIFO entries
         * Copy both words into the caller buffer, little endian.
         */
        buffer[0] = (uint8_t)((resp[2] >> 0) & 0xFFu);
        buffer[1] = (uint8_t)((resp[2] >> 8) & 0xFFu);
        buffer[2] = (uint8_t)((resp[2] >> 16) & 0xFFu);
        buffer[3] = (uint8_t)((resp[2] >> 24) & 0xFFu);
        buffer[4] = (uint8_t)((resp[3] >> 0) & 0xFFu);
        buffer[5] = (uint8_t)((resp[3] >> 8) & 0xFFu);
        buffer[6] = (uint8_t)((resp[3] >> 16) & 0xFFu);
        buffer[7] = (uint8_t)((resp[3] >> 24) & 0xFFu);
    }

    return status;
}



/* ------------------------------------------------------------------
 * Recovered transport + service-management functions (unchanged from the
 * pre-multislot driver): MU transceive, service Ping/Open/Close, and plain
 * AES/HMAC key load. Kept verbatim; the multi-slot refactor above only
 * touched the crypto request builders.
 * ------------------------------------------------------------------ */
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static status_t V2X_FCE_Transceive(MU_Type *mu,
                                   const uint32_t *msg,
                                   uint32_t msgWords,
                                   uint32_t *resp,
                                   uint8_t command);

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Send a fully formed FCE message and wait for its response.
 *
 * Sends msgWords 32-bit words through the MU transmit registers, retrieves the
 * response from the receive registers, then validates the response tag, echoed
 * command, and status field.
 *
 * @param mu       FCE MU register base.
 * @param msg      Request buffer (word 0 must already contain the header).
 * @param msgWords Number of 32-bit words in msg (equal to the header Size).
 * @param resp     Response buffer, at least V2X_FCE_MSG_MAX_WORDS words.
 * @param command  Expected echoed command opcode in the response header.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
static status_t V2X_FCE_Transceive(MU_Type *mu,
                                   const uint32_t *msg,
                                   uint32_t msgWords,
                                   uint32_t *resp,
                                   uint8_t command)
{
    status_t status;
    uint32_t txMsg[V2X_FCE_MSG_MAX_WORDS];
    uint32_t respWords;
    uint32_t regIdx;
    /* PING uses the base Tag; all service commands (open/get-info/close)
     * share the next Tag value. The firmware echoes Tag + offset. */
    uint8_t  txTag       = (command == V2X_FCE_CMD_PING)
                               ? (uint8_t)V2X_FCE_MSG_TAG_CMD_BASE
                               : (uint8_t)(V2X_FCE_MSG_TAG_CMD_BASE + 1u);
    uint8_t  expRespTag  = (uint8_t)(txTag + V2X_FCE_TAG_RESP_OFFSET);
    uint32_t i;

    /* Copy the caller request and stamp the command-based Tag into
     * header bits 31-24. */
    for (i = 0u; (i < msgWords) && (i < V2X_FCE_MSG_MAX_WORDS); i++)
    {
        txMsg[i] = msg[i];
    }
    txMsg[0] = (txMsg[0] & 0x00FFFFFFu) | ((uint32_t)txTag << 24);

#if V2X_FCE_DEBUG
    {
        uint32_t w;
        V2X_FCE_TRACE("[FCE] TX cmd=0x%02x tag=0x%02x words=%u:", (unsigned)command,
                      (unsigned)txTag, (unsigned)msgWords);
        for (w = 0u; w < msgWords; w++)
        {
            V2X_FCE_TRACE(" %08x", (unsigned)txMsg[w]);
        }
        V2X_FCE_TRACE("\r\n");
    }
#endif

    /* Send the request word-by-word through the MU transmit registers,
     * round-robining across the available TR registers exactly as the S3MU
     * transport did (regIndex = word counter modulo MU_TR_COUNT). MU_SendMsg
     * blocks until the target TR register is empty before writing. */
    for (i = 0u; i < msgWords; i++)
    {
        regIdx = i % (uint32_t)MU_TR_COUNT;
        status = MU_SendMsg(mu, regIdx, txMsg[i]);
        if (status != kStatus_Success)
        {
            V2X_FCE_TRACE("[FCE] MU_SendMsg failed at word %u: %d\r\n", (unsigned)i, (int)status);
            return status;
        }
    }

    /* Collect the response. The header always arrives in RR[0]; its Size field
     * (bits 15-8) gives the total word count (including the header). The
     * remaining payload words are read round-robin across the RR registers
     * (regIndex = word counter modulo MU_RR_COUNT), matching the S3MU flow. */
    /* Interrupt-driven completion: if the caller armed an IRQ for this MU,
     * enable the MU RX-full interrupt so the application ISR (via
     * V2X_FCE_HandleIRQ) runs on completion, then wait for RX-full.
     *
     * The wait deliberately does NOT block in __WFI(): the V2X-FH MU RX line
     * is only routed to some cores' NVIC (e.g. the Cortex-M7 SHE vector). On a
     * core where the line is not delivered, __WFI() would sleep with no
     * pending event and never wake - a lost-wakeup deadlock. Polling the
     * RX-full flag (bounded by the same retry budget as the busy-poll path)
     * cannot lose a wakeup and works identically on every core; the enabled
     * interrupt still fires harmlessly into the application ISR where routed.
     * If the budget is exhausted the loop falls through to the receive below,
     * matching the pure busy-poll completion path. */
    if ((s_v2xFceIrqMode != 0u) && (s_v2xFceIrqMu == mu))
    {
        uint32_t guard = 0u;
        s_v2xFceDone = 0u;
        MU_EnableInterrupts(mu, (uint32_t)kMU_Rx0FullInterruptEnable);
        while ((s_v2xFceDone == 0u) &&
               ((MU_GetStatusFlags(mu) & (uint32_t)kMU_Rx0FullFlag) == 0u) &&
               (guard < V2X_FCE_GCM_STATUS_RETRIES))
        {
            guard++;
        }
        MU_DisableInterrupts(mu, (uint32_t)kMU_Rx0FullInterruptEnable);
    }
    resp[0]   = MU_ReceiveMsg(mu, 0u);
    respWords = (resp[0] >> 8) & 0xFFu;
    if ((respWords == 0u) || (respWords > V2X_FCE_MSG_MAX_WORDS))
    {
        respWords = V2X_FCE_MSG_MAX_WORDS;
    }
    for (i = 1u; i < respWords; i++)
    {
        regIdx  = i % (uint32_t)MU_RR_COUNT;
        resp[i] = MU_ReceiveMsg(mu, regIdx);
    }


#if V2X_FCE_DEBUG
    {
        /* respWords was already computed above from the header Size field. */
        uint32_t w;
        V2X_FCE_TRACE("[FCE] RX words=%u:", (unsigned)respWords);

        for (w = 0u; w < respWords; w++)
        {
            V2X_FCE_TRACE(" %08x", (unsigned)resp[w]);
        }
        V2X_FCE_TRACE("\r\n");
    }
    V2X_FCE_TRACE("[FCE]   tag=0x%02x cmd=0x%02x status=0x%02x rating=0x%02x\r\n",
                  (unsigned)V2X_FCE_HDR_TAG(resp[0]), (unsigned)V2X_FCE_HDR_CMD(resp[0]),
                  (unsigned)V2X_FCE_RESP_STATUS(resp[1]), (unsigned)V2X_FCE_RESP_RATING(resp[1]));
#endif

    /* Validate response framing: tag, echoed command, and status. */
    if (V2X_FCE_HDR_TAG(resp[0]) != expRespTag)
    {
        V2X_FCE_TRACE("[FCE] bad response tag: got 0x%02x expected 0x%02x\r\n",
                      (unsigned)V2X_FCE_HDR_TAG(resp[0]), (unsigned)expRespTag);
        return kStatus_Fail;
    }
    if (V2X_FCE_HDR_CMD(resp[0]) != command)
    {
        V2X_FCE_TRACE("[FCE] bad echoed command: got 0x%02x expected 0x%02x\r\n",
                      (unsigned)V2X_FCE_HDR_CMD(resp[0]), (unsigned)command);
        return kStatus_Fail;
    }
    if (V2X_FCE_RESP_STATUS(resp[1]) != V2X_FCE_STATUS_SUCCESS)
    {
        V2X_FCE_TRACE("[FCE] command failed: status=0x%02x rating=0x%02x ratingExt=0x%04x\r\n",
                      (unsigned)V2X_FCE_RESP_STATUS(resp[1]), (unsigned)V2X_FCE_RESP_RATING(resp[1]),
                      (unsigned)((resp[1] >> 16) & 0xFFFFu));
        return kStatus_Fail;
    }

    return kStatus_Success;
}

status_t V2X_FCE_Ping(MU_Type *mu)

{
    uint32_t msg[V2X_FCE_PING_SIZE];
    uint32_t resp[V2X_FCE_MSG_MAX_WORDS];

    if (mu == NULL)
    {
        return kStatus_InvalidArgument;
    }

    /* Header-only PING (command 0x01). */
    msg[0] = V2X_FCE_CMD_HEADER(V2X_FCE_CMD_PING, V2X_FCE_PING_SIZE);

    return V2X_FCE_Transceive(mu, msg, V2X_FCE_PING_SIZE, resp, V2X_FCE_CMD_PING);
}

status_t V2X_FCE_Open(MU_Type *mu, v2x_fce_handle_t *handle)

{
    uint32_t msg[V2X_FCE_SERVICE_OPEN_SIZE];
    uint32_t resp[V2X_FCE_MSG_MAX_WORDS];
    status_t status;

    if ((mu == NULL) || (handle == NULL))
    {
        return kStatus_InvalidArgument;
    }

    /* Table 5: header only. */
    msg[0] = V2X_FCE_CMD_HEADER(V2X_FCE_CMD_SERVICE_OPEN, V2X_FCE_SERVICE_OPEN_SIZE);

    status = V2X_FCE_Transceive(mu, msg, V2X_FCE_SERVICE_OPEN_SIZE, resp, V2X_FCE_CMD_SERVICE_OPEN);
    if (status == kStatus_Success)
    {
        handle->mu = mu;
        /* Service handle is returned in response word 2 (Table 6). */
        handle->serviceId = resp[2];
    }

    return status;
}

status_t V2X_FCE_Close(v2x_fce_handle_t *handle)
{
    uint32_t msg[V2X_FCE_SERVICE_CLOSE_SIZE];
    uint32_t resp[V2X_FCE_MSG_MAX_WORDS];

    if ((handle == NULL) || (handle->mu == NULL))
    {
        return kStatus_InvalidArgument;
    }

    /* Table 7: header only. */
    msg[0] = V2X_FCE_CMD_HEADER(V2X_FCE_CMD_SERVICE_CLOSE, V2X_FCE_SERVICE_CLOSE_SIZE);

    return V2X_FCE_Transceive(handle->mu, msg, V2X_FCE_SERVICE_CLOSE_SIZE, resp, V2X_FCE_CMD_SERVICE_CLOSE);
}

status_t V2X_FCE_LoadAesPlainKey(v2x_fce_handle_t *handle, uint8_t keySlot, const uint8_t *key, uint32_t keySize)
{
    uint32_t msg[V2X_FCE_LOAD_AES_KEY_SIZE];
    uint32_t resp[V2X_FCE_MSG_MAX_WORDS];
    uint32_t i;

    if ((handle == NULL) || (handle->mu == NULL) || (key == NULL))
    {
        return kStatus_InvalidArgument;
    }
    if (keySlot >= V2X_FCE_AES_KEY_SLOTS)
    {
        return kStatus_InvalidArgument;
    }
    if ((keySize != 16u) && (keySize != 24u) && (keySize != 32u))
    {
        return kStatus_InvalidArgument;
    }

    /*
     * Table 13: the plain key is inlined into the message, not passed by
     * address.
     *   W0    Header
     *   W1    [RFU(31-16) | Key Byte Size(15-8) | Slot to update(7-0)]
     *   W2-9  Key Word 0-7 (up to 32 bytes, little endian)
     *   W10   CRC
     */
    for (i = 0u; i < V2X_FCE_LOAD_AES_KEY_SIZE; i++)
    {
        msg[i] = 0u;
    }

    msg[0] = V2X_FCE_CMD_HEADER(V2X_FCE_CMD_LOAD_AES_PLAIN_KEY, V2X_FCE_LOAD_AES_KEY_SIZE);
    msg[1] = ((keySize & 0xFFu) << 8) | ((uint32_t)keySlot & 0xFFu);

    /* Inline the key bytes into words 2..9 (little endian). */
    for (i = 0u; i < keySize; i++)
    {
        msg[2u + (i / 4u)] |= ((uint32_t)key[i]) << (8u * (i % 4u));
    }

    /* W10 CRC: XOR of W0..W9 (firmware validates this field). */
    msg[V2X_FCE_LOAD_AES_KEY_SIZE - 1u] = V2X_FCE_CrcCompute(msg, V2X_FCE_LOAD_AES_KEY_SIZE);

    return V2X_FCE_Transceive(handle->mu, msg, V2X_FCE_LOAD_AES_KEY_SIZE, resp, V2X_FCE_CMD_LOAD_AES_PLAIN_KEY);
}

status_t V2X_FCE_LoadHmacPlainKey(v2x_fce_handle_t *handle, uint8_t keySlot, v2x_fce_sha_algo_t shaAlgo, const uint8_t *key, uint32_t keySize)
{
    uint32_t msg[V2X_FCE_LOAD_HMAC_KEY_SIZE];
    uint32_t resp[V2X_FCE_MSG_MAX_WORDS];
    uint32_t i;

    if ((handle == NULL) || (handle->mu == NULL) || (key == NULL))
    {
        return kStatus_InvalidArgument;
    }
    if (keySlot >= V2X_FCE_HMAC_KEY_SLOTS)
    {
        return kStatus_InvalidArgument;
    }
    /* Key Word 0-31 => up to 32 words = 128 bytes of key material. */
    if ((keySize == 0u) || (keySize > 128u))
    {
        return kStatus_InvalidArgument;
    }

    /*
     * Table 15:
     *   W0     Header
     *   W1     [RFU(31-16) | SHA algo(15-8) | Slot to update(7-0)]
     *   W2-33  Key Word 0-31 (up to 128 bytes, little endian)
     *   W34    CRC
     */
    for (i = 0u; i < V2X_FCE_LOAD_HMAC_KEY_SIZE; i++)
    {
        msg[i] = 0u;
    }

    msg[0] = V2X_FCE_CMD_HEADER(V2X_FCE_CMD_LOAD_HMAC_PLAIN_KEY, V2X_FCE_LOAD_HMAC_KEY_SIZE);
    /* [RFU(31-16) | SHA algo(15-8) | Slot(7-0)]: the FCE binds the loaded
     * HMAC key to this SHA algorithm, matching the reference
     * prime_load_hmac_key (data[0] = (sha_algo << 8) | key_slot). */
    msg[1] = (((uint32_t)shaAlgo & 0xFFu) << 8) | ((uint32_t)keySlot & 0xFFu);

    for (i = 0u; i < keySize; i++)
    {
        msg[2u + (i / 4u)] |= ((uint32_t)key[i]) << (8u * (i % 4u));
    }

    /* W34 CRC: XOR of W0..W33 (firmware validates this field). */
    msg[V2X_FCE_LOAD_HMAC_KEY_SIZE - 1u] = V2X_FCE_CrcCompute(msg, V2X_FCE_LOAD_HMAC_KEY_SIZE);

    return V2X_FCE_Transceive(handle->mu, msg, V2X_FCE_LOAD_HMAC_KEY_SIZE, resp, V2X_FCE_CMD_LOAD_HMAC_PLAIN_KEY);
}

/*!
 * @brief Select the request-completion wait strategy of the V2X/FCE driver.
 *
 * When useInterrupt is true, the MU header round-trip in the driver waits on
 * the V2X-FH MU RX-full interrupt using __WFI() instead of busy-polling the
 * RX-full flag. The application must supply the IRQ number of the V2X-FH MU
 * RX line routed to the running core (it differs per core and per SoC, so it
 * is not baked into this component) and must install an ISR on that vector
 * that calls V2X_FCE_HandleIRQ(). When false, the driver reverts to the
 * busy-poll path. Call after V2X_FCE_Open() so the handle MU is known.
 *
 * @param handle       V2X/FCE service handle (must be open).
 * @param useInterrupt true = interrupt-driven wait, false = busy-poll.
 * @param irqn         IRQ number of the V2X-FH MU RX line for this core
 *                     (cast from the SoC IRQn_Type enum). Ignored when
 *                     useInterrupt is false.
 * @return kStatus_Success on success, kStatus_InvalidArgument on a bad handle.
 */
status_t V2X_FCE_SetWaitMode(v2x_fce_handle_t *handle, bool useInterrupt, int32_t irqn)
{
    if ((handle == NULL) || (handle->mu == NULL))
    {
        return kStatus_InvalidArgument;
    }

    s_v2xFceIrqMu   = handle->mu;
    s_v2xFceIrqMode = useInterrupt ? 1u : 0u;
    s_v2xFceIrqn    = irqn;

    if (useInterrupt && (irqn >= 0))
    {
        s_v2xFceDone = 0u;
        (void)EnableIRQ((IRQn_Type)irqn);
    }
    else if (irqn >= 0)
    {
        (void)DisableIRQ((IRQn_Type)irqn);
    }
    else
    {
        /* No IRQ supplied: stay in busy-poll mode. */
        s_v2xFceIrqMode = 0u;
    }
    return kStatus_Success;
}

