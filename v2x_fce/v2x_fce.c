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

#include <string.h>

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

/* Maximum AES-GCM payload the FCE accepts (see PRIME / FCE Limitations). */
#define V2X_FCE_GCM_MAX_PAYLOAD (0x1fffc0u)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static status_t V2X_FCE_Transceive(MU_Type *mu,
                                   const uint32_t *msg,
                                   uint32_t msgWords,
                                   uint32_t *resp,
                                   uint8_t command);

static status_t V2X_FCE_AesGcm(v2x_fce_handle_t *handle,
                               const v2x_fce_gcm_op_t *op,
                               uint8_t requestId);

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

status_t V2X_FCE_LoadHmacPlainKey(v2x_fce_handle_t *handle, uint8_t keySlot, const uint8_t *key, uint32_t keySize)
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
    /* Key Word 0-32 => up to 33 words = 132 bytes of key material. */
    if ((keySize == 0u) || (keySize > 132u))
    {
        return kStatus_InvalidArgument;
    }

    /*
     * Table 15:
     *   W0     Header
     *   W1     [RFU(31-16) | SHA algo(15-8) | Slot to update(7-0)]
     *   W2-35  Key Word 0-32 (up to 132 bytes, little endian)
     *   W36    CRC
     */
    for (i = 0u; i < V2X_FCE_LOAD_HMAC_KEY_SIZE; i++)
    {
        msg[i] = 0u;
    }

    msg[0] = V2X_FCE_CMD_HEADER(V2X_FCE_CMD_LOAD_HMAC_PLAIN_KEY, V2X_FCE_LOAD_HMAC_KEY_SIZE);
    /* SHA algo field left zero (default); slot in the low byte. */
    msg[1] = ((uint32_t)keySlot & 0xFFu);

    for (i = 0u; i < keySize; i++)
    {
        msg[2u + (i / 4u)] |= ((uint32_t)key[i]) << (8u * (i % 4u));
    }

    /* W36 CRC: XOR of W0..W35 (firmware validates this field). */
    msg[V2X_FCE_LOAD_HMAC_KEY_SIZE - 1u] = V2X_FCE_CrcCompute(msg, V2X_FCE_LOAD_HMAC_KEY_SIZE);

    return V2X_FCE_Transceive(handle->mu, msg, V2X_FCE_LOAD_HMAC_KEY_SIZE, resp, V2X_FCE_CMD_LOAD_HMAC_PLAIN_KEY);
}

/*!
 * @brief Shared AES-GCM authenticate request.
 *
 * Unlike the synchronous S3MU command messages (Ping/Open/GetInfo/LoadKey), an
 * AES-GCM Oneshot request is submitted exactly the way the reference PRIME/FCE
 * library does it (secure_enclave sab_prime_aes.c / sab_prime_process_msg.c /
 * sab_prime_api.c):
 *
 *   1. The 13-word Table 27 request descriptor is written DIRECTLY into one of
 *      the FCE MU slot buffers (mu_buf = MU_base + 0x8000, 64 bytes per slot;
 *      slot A = V2X_FCE_SLOT_A_BASE(mu)). The descriptor header carries the request
 *      ID (0x68 encrypt / 0x69 decrypt) as its command, Size 0xD words, Tag
 *      0x1b, Ver 0x02. There is NO trailing CRC word - only the plain-key load
 *      messages are CRC protected.
 *   2. A header-only PUSH_REQUEST (0xB0 | slot bitmap, 1 word) is sent through
 *      the MU transmit registers. The FCE returns the request id in response
 *      word 2; a set low nibble in that id signals an error.
 *   3. The host polls GET_REQUEST_STATUS until the request is no longer ongoing
 *      (0x10D6), then reads the completion status word the FCE wrote back to
 *      the descriptor Status Address.
 *
 * The descriptor addresses (Status/TAG/Input/Output/AAD) point into a plain DDR
 * scratch region the FCE addresses directly with the same physical address the
 * CM33 uses; the driver stages caller input/AAD in and copies results back out.
 */
static status_t V2X_FCE_AesGcm(v2x_fce_handle_t *handle,
                               const v2x_fce_gcm_op_t *op,
                               uint8_t requestId)
{
    /* Table 27 descriptor written straight into the FCE MU slot A buffer. */
    volatile uint32_t *slot = (volatile uint32_t *)V2X_FCE_SLOT_A_BASE(handle->mu);
    /* GCM DMA buffers live in a plain DDR scratch region the FCE can address
     * directly. The caller input/AAD are staged in, results copied back out. */
    volatile uint32_t *ddrStatus = (volatile uint32_t *)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_STATUS_OFF);
    uint8_t *ddrTag              = (uint8_t *)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_TAG_OFF);
    uint8_t *ddrInput            = (uint8_t *)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_INPUT_OFF);
    uint8_t *ddrOutput           = (uint8_t *)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_OUTPUT_OFF);
    uint8_t *ddrAad              = (uint8_t *)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_AAD_OFF);
    uint32_t desc[V2X_FCE_GCM_DESC_WORDS];
    /* Big enough for both the 1-word PUSH_REQUEST and the 2-word
     * GET_REQUEST_STATUS message (whose word 1 must carry the request id). */
    uint32_t msg[V2X_FCE_GET_REQUEST_STATUS_SIZE];
    uint32_t resp[V2X_FCE_MSG_MAX_WORDS];
    status_t status;
    uint32_t i;
    uint32_t reqId;
    uint32_t reqStatus = V2X_FCE_REQ_ONGOING;


    if ((handle == NULL) || (handle->mu == NULL) || (op == NULL))
    {
        V2X_FCE_TRACE("[FCE] GCM arg check: NULL handle/mu/op\r\n");
        return kStatus_InvalidArgument;
    }
    if ((op->iv == NULL) || (op->input == NULL) || (op->output == NULL) || (op->tag == NULL))
    {
        V2X_FCE_TRACE("[FCE] GCM arg check: NULL buffer iv=%p input=%p output=%p tag=%p\r\n",
                      (void *)op->iv, (void *)op->input, (void *)op->output, (void *)op->tag);
        return kStatus_InvalidArgument;
    }
    if (op->keySlot >= V2X_FCE_AES_KEY_SLOTS)
    {
        V2X_FCE_TRACE("[FCE] GCM arg check: bad keySlot=%u (max %u)\r\n",
                      (unsigned)op->keySlot, (unsigned)V2X_FCE_AES_KEY_SLOTS);
        return kStatus_InvalidArgument;
    }
    if (op->inputSize > V2X_FCE_GCM_MAX_PAYLOAD)
    {
        V2X_FCE_TRACE("[FCE] GCM arg check: inputSize=%u exceeds max=%u\r\n",
                      (unsigned)op->inputSize, (unsigned)V2X_FCE_GCM_MAX_PAYLOAD);
        return kStatus_InvalidArgument;
    }
    /* DMA-referenced buffers must be 64-bit aligned. AAD may be NULL. */
    if (!V2X_FCE_ADDR_ALIGNED(op->input) || !V2X_FCE_ADDR_ALIGNED(op->output) ||
        !V2X_FCE_ADDR_ALIGNED(op->tag))
    {
        V2X_FCE_TRACE("[FCE] GCM arg check: unaligned buffer input=%p output=%p tag=%p (need 8-byte)\r\n",
                      (void *)op->input, (void *)op->output, (void *)op->tag);
        return kStatus_InvalidArgument;
    }
    if ((op->aad != NULL) && !V2X_FCE_ADDR_ALIGNED(op->aad))
    {
        V2X_FCE_TRACE("[FCE] GCM arg check: unaligned aad=%p (need 8-byte)\r\n", (void *)op->aad);
        return kStatus_InvalidArgument;
    }

    /* Seed the DDR status word with success (0xd6) and stage the caller input
     * (and AAD) into the DDR scratch region the FCE can address directly.
     *
     * The reference PRIME library (sab_prime_crypt.c prime_aead_build) writes
     * REQ_SUCCESS (0xd6) into the status buffer before pushing the request and
     * relies on the FCE firmware to overwrite it ONLY on error. On success the
     * firmware leaves the status word untouched, so it must already read as
     * success. Clearing it to 0 (as before) made every successful GCM appear
     * to fail. */
    *ddrStatus = (uint32_t)V2X_FCE_STATUS_SUCCESS;

    if (op->inputSize != 0u)
    {
        (void)memcpy(ddrInput, op->input, op->inputSize);
    }
    if ((op->aad != NULL) && (op->aadSize != 0u))
    {
        (void)memcpy(ddrAad, op->aad, op->aadSize);
    }

    /*
     * Step 1: build the Table 27 AES-GCM Oneshot request descriptor (13 words,
     * no CRC) and write it directly into the FCE MU slot A buffer.
     *   W0     Header: [Tag 0x1b | Command=requestId | Size 0xD | Ver 0x02]
     *   W1     Status Address
     *   W2     [IV nb bytes(31-24) | Key Slot(23-16) | TAG nb bytes(15-8) | Flags(7-0)]
     *   W3     TAG Address
     *   W4     Payload Byte Length
     *   W5     Input Address
     *   W6     Output Address
     *   W7-10  IV (inline, little endian)
     *   W11    AAD Byte Length
     *   W12    AAD Address
     */
    for (i = 0u; i < V2X_FCE_GCM_DESC_WORDS; i++)
    {
        desc[i] = 0u;
    }

    /* W0 descriptor header: request ID command, size 0xD, command Tag 0x1b. */
    desc[0] = V2X_FCE_BUILD_HEADER((uint8_t)(V2X_FCE_MSG_TAG_CMD_BASE + 1u), requestId,
                                   V2X_FCE_GCM_DESC_WORDS, V2X_FCE_MSG_VERSION);
    desc[1] = (uint32_t)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_STATUS_OFF);
    desc[2] = (((uint32_t)op->ivSize & 0xFFu) << 24) |
              (((uint32_t)op->keySlot & 0xFFu) << 16) |
              (((uint32_t)op->tagSize & 0xFFu) << 8) |
              ((V2X_FCE_REQ_FLAG_SIGNAL | V2X_FCE_REQ_FLAG_AUTO_POP) & 0xFFu);

    desc[3] = (uint32_t)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_TAG_OFF);
    desc[4] = op->inputSize;
    desc[5] = (uint32_t)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_INPUT_OFF);
    desc[6] = (uint32_t)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_OUTPUT_OFF);

    /* Words 7-10: IV inlined (up to 16 bytes, little endian). */
    for (i = 0u; (i < op->ivSize) && (i < 16u); i++)
    {
        desc[7u + (i / 4u)] |= ((uint32_t)op->iv[i]) << (8u * (i % 4u));
    }

    desc[11] = op->aadSize;
    desc[12] = ((op->aad != NULL) && (op->aadSize != 0u))
                   ? (uint32_t)(V2X_FCE_DDR_SCRATCH_BASE + V2X_FCE_DDR_AAD_OFF)
                   : 0u;

    /* Publish the descriptor into the FCE slot buffer (volatile, in order). */
    for (i = 0u; i < V2X_FCE_GCM_DESC_WORDS; i++)
    {
        slot[i] = desc[i];
    }

#if V2X_FCE_DEBUG
    {
        V2X_FCE_TRACE("[FCE] GCM desc -> slot A @%08x:", (unsigned)V2X_FCE_SLOT_A_BASE(handle->mu));
        for (i = 0u; i < V2X_FCE_GCM_DESC_WORDS; i++)
        {
            V2X_FCE_TRACE(" %08x", (unsigned)desc[i]);
        }
        V2X_FCE_TRACE("\r\n");
    }
#endif

    __DSB();

    /*
     * Step 2: header-only PUSH_REQUEST (0xB0 | slot bitmap). Sent through the
     * MU transmit registers; the FCE returns the request id in response word 2.
     * A set low nibble in the request id indicates the request was rejected.
     */
    msg[0] = V2X_FCE_CMD_HEADER((uint8_t)(V2X_FCE_CMD_PUSH_REQUEST | V2X_FCE_SLOT_A),
                                V2X_FCE_PUSH_REQUEST_SIZE);

    status = V2X_FCE_Transceive(handle->mu, msg, V2X_FCE_PUSH_REQUEST_SIZE, resp,
                                (uint8_t)(V2X_FCE_CMD_PUSH_REQUEST | V2X_FCE_SLOT_A));
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Reference prime_push() returns response word 2 as the request id and
     * treats a set low nibble as an error. */
    reqId = resp[2];
    V2X_FCE_TRACE("[FCE] PUSH resp: w1=0x%08x w2(reqId)=0x%08x\r\n",
                  (unsigned)resp[1], (unsigned)reqId);
    if ((reqId & 0xFu) != 0u)
    {
        V2X_FCE_TRACE("[FCE] PUSH rejected: req_id=0x%08x\r\n", (unsigned)reqId);
        return kStatus_Fail;
    }

    /*
     * Step 3: poll GET_REQUEST_STATUS until the request is no longer ongoing,
     * then evaluate the completion status the FCE wrote to the Status Address.
     */
    for (i = 0u; i < V2X_FCE_GCM_STATUS_RETRIES; i++)
    {
        msg[0] = V2X_FCE_CMD_HEADER(V2X_FCE_CMD_GET_REQUEST_STATUS, V2X_FCE_GET_REQUEST_STATUS_SIZE);
        /* Word 1 must carry the request id returned by the PUSH; otherwise the
         * FCE cannot match the query and reports an invalid status. */
        msg[1] = reqId;

        status = V2X_FCE_Transceive(handle->mu, msg, V2X_FCE_GET_REQUEST_STATUS_SIZE, resp,
                                    V2X_FCE_CMD_GET_REQUEST_STATUS);
        if (status != kStatus_Success)
        {
            return status;
        }

        /* Response word 2 carries the per-request status (Table 21). */
        reqStatus = resp[2];
        if (reqStatus != V2X_FCE_REQ_ONGOING)
        {
            break;
        }
    }

    V2X_FCE_TRACE("[FCE] GET_REQ_STATUS done: polls=%u reqStatus=0x%08x\r\n",
                  (unsigned)i, (unsigned)reqStatus);

    if (reqStatus == V2X_FCE_REQ_ONGOING)
    {
        V2X_FCE_TRACE("[FCE] GCM timeout: request still ongoing after %u polls\r\n",
                      (unsigned)V2X_FCE_GCM_STATUS_RETRIES);
        return kStatus_Timeout;
    }

    __DSB();

    V2X_FCE_TRACE("[FCE] DDR status word=0x%08x (status=0x%02x) slotW15=0x%08x\r\n",
                  (unsigned)*ddrStatus, (unsigned)V2X_FCE_RESP_STATUS(*ddrStatus),
                  (unsigned)slot[15]);

    /* The FCE writes the final GCM completion status to the DDR Status word. */
    if (V2X_FCE_RESP_STATUS(*ddrStatus) != V2X_FCE_STATUS_SUCCESS)
    {
        V2X_FCE_TRACE("[FCE] GCM completion failed: ddrStatus=0x%08x\r\n", (unsigned)*ddrStatus);
        return kStatus_Fail;
    }

    /* Copy the produced ciphertext/plaintext and TAG back to the caller. */
    if (op->inputSize != 0u)
    {
        (void)memcpy(op->output, ddrOutput, op->inputSize);
    }
    if (op->tagSize != 0u)
    {
        (void)memcpy(op->tag, ddrTag, op->tagSize);
    }

    return kStatus_Success;
}



status_t V2X_FCE_AesGcmEncrypt(v2x_fce_handle_t *handle, const v2x_fce_gcm_op_t *op)
{
    return V2X_FCE_AesGcm(handle, op, V2X_FCE_REQ_AES_GCM_ENCRYPT);
}

status_t V2X_FCE_AesGcmDecrypt(v2x_fce_handle_t *handle, const v2x_fce_gcm_op_t *op)
{
    return V2X_FCE_AesGcm(handle, op, V2X_FCE_REQ_AES_GCM_DECRYPT);
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
