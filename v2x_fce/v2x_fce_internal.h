/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Internal message definitions for the EdgeLock V2X / FCE (Fast Crypto Engine)
 * firmware service reachable over its dedicated message unit.
 *
 * Wire format reference: EdgeLock Secure Enclave User Guide, PRIME / FCE
 * chapter. The FCE service reuses the same enclave message framing as the ELE
 * HSM and V2X-FH services (fsl_s3mu).
 *
 * Message header (word 0) is little endian and packs four bytes:
 *   Bits  7-0  : Ver     (protocol version)
 *   Bits 15-8  : Size    (message size in 32-bit words, including header/CRC)
 *   Bits 23-16 : Command (opcode, see V2X_FCE_CMD_* below)
 *   Bits 31-24 : Tag     (request/response tag)
 *
 * Response word 1 packs:
 *   Bits  7-0  : Status  (0x00 success, 0x29 failure)
 *   Bits 15-8  : Rating
 *   Bits 31-16 : Rating extension
 */

#ifndef V2X_FCE_INTERNAL_H_
#define V2X_FCE_INTERNAL_H_

/* Message header tags. Same enclave framing as the ELE HSM / V2X services. */
//#define V2X_FCE_MSG_TAG_CMD  (0x17u) /* request  header tag */
//#define V2X_FCE_MSG_TAG_RESP (0xe1u) /* response header tag */

/*
 * The request header Tag is a per-message sequence counter. The very
 * first request uses V2X_FCE_MSG_TAG_CMD_BASE and each subsequent
 * request increments it (ping 0x1a, service-open 0x1b, ...). The
 * firmware echoes back the response with Tag = request Tag +
 * V2X_FCE_TAG_RESP_OFFSET (ping resp 0xe4, open resp 0xe5, ...).
 */
#define V2X_FCE_MSG_TAG_CMD_BASE (0x1au) /* first request header tag  */
#define V2X_FCE_TAG_RESP_OFFSET  (0xCAu) /* response tag = req + this  */

/* Protocol version reported in the Ver field of the header. */
//#define V2X_FCE_MSG_VERSION (0x07u)
#define V2X_FCE_MSG_VERSION (0x02u)

/* Response status codes (word 1, bits 7-0). */
#define V2X_FCE_STATUS_SUCCESS (0xd6u)
#define V2X_FCE_STATUS_FAILURE (0x29u)

/*
 * FCE command opcodes. These are the raw 8-bit Command values placed in header
 * bits 23-16 (see the EdgeLock Secure Enclave User Guide, PRIME / FCE).
 */
#define V2X_FCE_CMD_PING                (0x01u)
#define V2X_FCE_CMD_SERVICE_OPEN        (0x10u)
#define V2X_FCE_CMD_SERVICE_CLOSE       (0x11u)
#define V2X_FCE_CMD_GET_INFO            (0x18u)
#define V2X_FCE_CMD_LOAD_AES_PLAIN_KEY  (0x41u)
#define V2X_FCE_CMD_LOAD_HMAC_PLAIN_KEY (0x45u)
#define V2X_FCE_CMD_PUSH_REQUEST        (0xB0u) /* AES-GCM authenticate request */

#define V2X_FCE_CMD_GET_REQUEST_STATUS  (0xC0u)


/*
 * FCE Request opcodes (see PRIME / FCE User Guide, "FCE Request IDs",
 * Table 22). These are the Command values used in the header of a request
 * descriptor that is copied into the SHE0 MU slot buffer, not the S3MU
 * command opcodes above. PUSH_REQUEST then signals the FCE to process the
 * slot.
 */
#define V2X_FCE_REQ_AES_GCM_ENCRYPT (0x68u) /* GCM authenticate + encrypt */
#define V2X_FCE_REQ_AES_GCM_DECRYPT (0x69u) /* GCM verify + decrypt       */

/*
 * PUSH_REQUEST low-nibble slot bitmap (see Table 17). The command ID is
 * V2X_FCE_CMD_PUSH_REQUEST OR-ed with the slot bit(s) to push.
 */
#define V2X_FCE_SLOT_A (0x1u) /* bit 0 */
#define V2X_FCE_SLOT_B (0x2u) /* bit 1 */
#define V2X_FCE_SLOT_C (0x4u) /* bit 2 */
#define V2X_FCE_SLOT_D (0x8u) /* bit 3 */

/*
 * FCE request slot buffers. An AES-GCM Oneshot request descriptor (Table 27)
 * is written by the host directly into one of the four 16-word (64-byte) FCE
 * slot buffers and then triggered with a header-only PUSH_REQUEST carrying the
 * slot bitmap. Each slot exposes words 0..14 for the descriptor; word 15 is
 * reserved (for slot D it holds the last request ID written back by the FCE).
 */
/*
 * The FCE request/response slot buffers are not a fixed SoC address: they live
 * inside the message-buffer window of the FCE MU itself, at MU register base +
 * V2X_FCE_MU_BUF_OFFSET. Each slot is V2X_FCE_SLOT_STRIDE bytes
 * (V2X_FCE_SLOT_WORDS words) apart. Derive the slot base from the MU pointer so
 * the driver works for whichever V2X MU instance the caller opens.
 */
#define V2X_FCE_MU_BUF_OFFSET (0x8000u) /* slot window offset from MU register base */
#define V2X_FCE_SLOT_STRIDE   (0x40u)   /* 64 bytes (16 words) per slot              */
#define V2X_FCE_SLOT_WORDS    (0x10u)   /* 16 words per slot (0..14 + reserved)      */

/* Base address of slot N (0 = A, 1 = B, ...) inside the given MU's buffer window. */
#define V2X_FCE_SLOT_BASE(mu, slot) \
    ((uint32_t)((uintptr_t)(mu)) + V2X_FCE_MU_BUF_OFFSET + ((uint32_t)(slot) * V2X_FCE_SLOT_STRIDE))
#define V2X_FCE_SLOT_A_BASE(mu) V2X_FCE_SLOT_BASE((mu), 0u) /* words 0..14 = slot A, w15 reserved  */
#define V2X_FCE_SLOT_B_BASE(mu) V2X_FCE_SLOT_BASE((mu), 1u) /* words 0..14 = slot B, w15 reserved  */
#define V2X_FCE_SLOT_C_BASE(mu) V2X_FCE_SLOT_BASE((mu), 2u) /* words 0..14 = slot C, w15 reserved  */
#define V2X_FCE_SLOT_D_BASE(mu) V2X_FCE_SLOT_BASE((mu), 3u) /* words 0..14 = slot D, w15 last reqid */

/*
 * FCE Request Flags (word 2, bits 7-0; see Table 23). Bit 0 signal mode,
 * bit 1 auto-POP, bits 5-4 hash selection.
 */
#define V2X_FCE_REQ_FLAG_SIGNAL   (0x1u)
#define V2X_FCE_REQ_FLAG_AUTO_POP (0x2u)


/*
 * Build the 32-bit message header word from its fields.
 * size is the total message length in 32-bit words (header + payload + CRC).
 */
#define V2X_FCE_BUILD_HEADER(tag, cmd, size, ver)         \
    ((uint32_t)(((uint32_t)(tag) & 0xFFu) << 24) |        \
     ((uint32_t)((uint32_t)(cmd) & 0xFFu) << 16) |        \
     ((uint32_t)((uint32_t)(size) & 0xFFu) << 8) |        \
     ((uint32_t)((uint32_t)(ver) & 0xFFu)))

/* Convenience: request header with the standard command tag and version. */
#define V2X_FCE_CMD_HEADER(cmd, size) \
    V2X_FCE_BUILD_HEADER(V2X_FCE_MSG_TAG_CMD_BASE, (cmd), (size), V2X_FCE_MSG_VERSION)

/*
 * S3MU command message word counts (the header "Size" field), matching the
 * PRIME / FCE User Guide request tables. Each count includes the header word
 * and, where present, the trailing CRC word.
 */
#define V2X_FCE_PING_SIZE           (0x1u)  /* Ping:     header only            */
#define V2X_FCE_SERVICE_OPEN_SIZE   (0x1u)  /* Table 5:  header only            */
#define V2X_FCE_SERVICE_CLOSE_SIZE  (0x1u)  /* Table 7:  header only            */
#define V2X_FCE_GET_INFO_SIZE       (0x1u)  /* Table 9:  header only            */
#define V2X_FCE_LOAD_AES_KEY_SIZE   (0xBu)  /* Table 13: hdr + fields + 8 key + CRC = 11 */
#define V2X_FCE_LOAD_HMAC_KEY_SIZE  (0x25u) /* Table 15: hdr + fields + 33 key + CRC = 37 */
#define V2X_FCE_PUSH_REQUEST_SIZE   (0x1u)  /* Table 18: header only            */

/*

 * AES-GCM Oneshot request descriptor word count (Table 27). This descriptor is
 * written into the SHE0 MU slot buffer and then triggered with PUSH_REQUEST;
 * it is not an S3MU command message. 13 words:
 *   W0 Header, W1 Status address, W2 [IVbytes|KeySlot|TAGbytes|Flags],
 *   W3 TAG address, W4 Payload byte length, W5 Input address,
 *   W6 Output address, W7-10 IV, W11 AAD byte length, W12 AAD address.
 */
#define V2X_FCE_GCM_DESC_WORDS (0xDu)

/*
 * GET_REQUEST_STATUS is a 2-word S3MU command (Table 20). Response word 2
 * carries the per-request status; while the request is still being processed
 * the FCE reports V2X_FCE_REQ_ONGOING. The host polls up to
 * V2X_FCE_GCM_STATUS_RETRIES times before giving up.
 */
#define V2X_FCE_GET_REQUEST_STATUS_SIZE (0x2u)    /* PRIME_GET_REQ_STATUS_SZ 0x8 bytes = 2 words */
#define V2X_FCE_REQ_ONGOING             (0x10D6u) /* request still being processed               */
#define V2X_FCE_GCM_STATUS_RETRIES      (10000u)  /* status poll bound                           */

/*
 * AES-GCM DMA scratch buffers in DDR.
 *
 * The FCE / V2X engine is an A55-side bus master and cannot resolve the
 * CM33-private TCM aliases; translating them to the CM33 secure/non-secure
 * global aliases still fails PUSH_REQUEST with status 0x29. Instead the GCM
 * descriptor addresses (Status/TAG/Input/Output/AAD) point into a plain DDR
 * scratch region that the FCE addresses directly with the same SoC-global
 * physical address the CM33 uses. The driver stages the caller input/AAD into
 * this region, runs the request, then copies the ciphertext/plaintext and TAG
 * back out to the caller buffers.
 *
 * Layout inside the scratch region (each sub-buffer 64-bit aligned):
 *   +0x00000  Status word (4 bytes)
 *   +0x00040  TAG    (up to 16 bytes)
 *   +0x00100  Input  (payload)
 *   +0x08100  Output (payload)
 *   +0x10100  AAD
 */
/* Base of the plain DDR scratch region the FCE AHB-DMA engine uses for the
 * GCM Status/TAG/Input/Output/AAD buffers. Configurable via the
 * V2X_FCE_DDR_SCRATCH_BASE Kconfig option (emitted without a CONFIG_
 * prefix); defaults to the i.MX943 EVK window when not configured. */
#ifndef V2X_FCE_DDR_SCRATCH_BASE
#define V2X_FCE_DDR_SCRATCH_BASE (0x86000000u)
#endif
#define V2X_FCE_DDR_STATUS_OFF   (0x00000u)
#define V2X_FCE_DDR_TAG_OFF      (0x00040u)
#define V2X_FCE_DDR_INPUT_OFF    (0x00100u)
#define V2X_FCE_DDR_OUTPUT_OFF   (0x08100u)
#define V2X_FCE_DDR_AAD_OFF      (0x10100u)


/* Extract fields from a response header/status word. */
#define V2X_FCE_RESP_STATUS(word1) ((uint8_t)((word1) & 0xFFu))
#define V2X_FCE_RESP_RATING(word1) ((uint8_t)(((word1) >> 8) & 0xFFu))
#define V2X_FCE_HDR_TAG(word0)     ((uint8_t)(((word0) >> 24) & 0xFFu))
#define V2X_FCE_HDR_CMD(word0)     ((uint8_t)(((word0) >> 16) & 0xFFu))

#endif /* V2X_FCE_INTERNAL_H_ */
