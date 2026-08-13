/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Internal wire-format definitions for the EdgeLock V2X base (debug) message
 * protocol, ported from the Linux driver:
 *   drivers/firmware/imx/v2x_base_msg.{c,h}  (v2x_base_msg.h opcodes/sizes)
 *
 * The V2X base service reuses the same enclave message framing and S3MU
 * hardware driver as ele_crypto / v2x_crypto. The 32-bit message header word
 * is little endian and packs four bytes:
 *   Bits  7-0  : Ver     (protocol version, V2X_BASE_MSG_VERSION)
 *   Bits 15-8  : Size    (message size in 32-bit words, including header)
 *   Bits 23-16 : Command (opcode, see V2X_BASE_CMD_* below)
 *   Bits 31-24 : Tag     (request/response tag)
 *
 * Response word 1 packs:
 *   Bits  7-0  : Status
 *   Bits 15-8  : Rating
 *   Bits 31-16 : Rating extension
 *
 * NOTE: The Linux *_MSG_SZ defines are in BYTES; here we express the same
 * message lengths in 32-bit WORDS (bytes / 4).
 */

#ifndef V2X_BASE_INTERNAL_H_
#define V2X_BASE_INTERNAL_H_

/* Enclave message framing tags. */
#define V2X_BASE_MSG_TAG_CMD  (0x17u) /* V2X_DEBUG_MU_MSG_CMD_TAG */
#define V2X_BASE_MSG_TAG_RESP (0xe1u) /* V2X_DEBUG_MU_MSG_RSP_TAG / V2X_DBG_MU_MSG_RSP_TAG */

/* Protocol version reported in the Ver field of the header. */
#define V2X_BASE_MSG_VERSION (0x02u) /* V2X_DEBUG_MU_MSG_VERS */

/* Command opcodes (header bits 23-16). */
#define V2X_BASE_CMD_DBG_DUMP    (0x02u) /* V2X_DBG_DUMP_REQ    */
#define V2X_BASE_CMD_PWR_STATE   (0x04u) /* V2X_PWR_STATE       */
#define V2X_BASE_CMD_GET_FW_VER  (0x08u) /* V2X_DBG_GET_FW_VER  */
#define V2X_BASE_CMD_START_RNG   (0x0eu) /* V2X_START_RNG_REQ   */

/* Message sizes, expressed in 32-bit WORDS (Linux defines were bytes). */
#define V2X_BASE_START_RNG_REQ_WORDS  (0x04u / 4u) /* 0x04 bytes -> 1 word  */
#define V2X_BASE_START_RNG_RSP_WORDS  (0x0Cu / 4u) /* 0x0C bytes -> 3 words */
#define V2X_BASE_PWR_STATE_REQ_WORDS  (0x08u / 4u) /* 0x08 bytes -> 2 words */
#define V2X_BASE_PWR_STATE_RSP_WORDS  (0x08u / 4u) /* 0x08 bytes -> 2 words */
#define V2X_BASE_GET_FW_VER_REQ_WORDS (0x04u / 4u) /* 0x04 bytes -> 1 word  */
#define V2X_BASE_GET_FW_VER_RSP_WORDS (0x10u / 4u) /* 0x10 bytes -> 4 words */
#define V2X_BASE_DBG_DUMP_REQ_WORDS   (0x08u / 4u) /* 0x08 bytes -> 2 words */
#define V2X_BASE_DBG_DUMP_RSP_WORDS   (0x5Cu / 4u) /* 0x5C bytes -> 23 words */

/* Power-state actions (payload word for V2X_BASE_CMD_PWR_STATE). */
#define V2X_BASE_PWR_ON_REQ  (0x1u) /* V2X_PWR_ON_REQ  */
#define V2X_BASE_PWR_OFF_REQ (0x4u) /* V2X_PWR_OFF_REQ */

/* Failure indications returned in the power-state response. */
#define V2X_BASE_PERM_DENIED_FAIL_IND (0xF329u) /* V2X_PERM_DENIED_FAIL_IND */
#define V2X_BASE_INVAL_OPS_FAIL_IND   (0xC029u) /* V2X_INVAL_OPS_FAIL_IND   */

/* Debug-dump framing. */
#define V2X_BASE_MAX_DBG_DMP_PKT    (15u) /* V2X_MAX_DBG_DMP_PKT    */
#define V2X_BASE_NON_DUMP_BUFFER_SZ (3u)  /* V2X_NON_DUMP_BUFFER_SZ */

/* start_rng: TRNG initialization-in-progress bits (response data[1]). */
#define V2X_BASE_PTRNG_IN_PROGRESS (0x1u) /* P-TRNG at bit 0 */
#define V2X_BASE_STRNG_IN_PROGRESS (0x2u) /* S-TRNG at bit 1 */

/* get_fw_version: field extraction from response data[1]. */
#define V2X_BASE_FW_MAJOR_SHIFT (16u)
#define V2X_BASE_FW_MAJOR_MASK  (0xFFu)
#define V2X_BASE_FW_MINOR_SHIFT (4u)
#define V2X_BASE_FW_MINOR_MASK  (0xFFFu)
#define V2X_BASE_FW_PATCH_MASK  (0xFu)

/* Response status success code. The V2X debug-MU (like the rest of the ELE
 * S3MU response protocol) reports success with the indication byte 0xD6 in
 * the low 8 bits of response word 1, not 0x00. */
#define V2X_BASE_STATUS_SUCCESS (0xD6u)

/* Build the 32-bit message header word from its fields. */
#define V2X_BASE_BUILD_HEADER(tag, cmd, size, ver)        \
    ((uint32_t)(((uint32_t)(tag) & 0xFFu) << 24) |        \
     ((uint32_t)((uint32_t)(cmd) & 0xFFu) << 16) |        \
     ((uint32_t)((uint32_t)(size) & 0xFFu) << 8) |        \
     ((uint32_t)((uint32_t)(ver) & 0xFFu)))

/* Convenience: request header with the standard command tag and version. */
#define V2X_BASE_CMD_HEADER(cmd, size) \
    V2X_BASE_BUILD_HEADER(V2X_BASE_MSG_TAG_CMD, (cmd), (size), V2X_BASE_MSG_VERSION)

/* Extract fields from a response header/status word. */
#define V2X_BASE_HDR_TAG(word0)     ((uint8_t)(((word0) >> 24) & 0xFFu))
#define V2X_BASE_HDR_CMD(word0)     ((uint8_t)(((word0) >> 16) & 0xFFu))
#define V2X_BASE_RESP_STATUS(word1) ((uint8_t)((word1) & 0xFFu))


/* ---------------------------------------------------------------------------
 * EdgeLock enclave (ELE) V2X-firmware bring-up wire format.
 *
 * On i.MX943 the V2X firmware core is not auto-started: the enclave must first
 * authenticate/release the V2X FW image, otherwise the V2X debug MU never
 * answers. These commands ride the same enclave message framing as the rest of
 * the ELE base API (fsl_ele_base_api.h): 32-bit little-endian header packing
 * Tag(31-24) | Command(23-16) | Size(15-8) | Ver(7-0). The ELE command tag is
 * 0x17 and the protocol version is 0x06 (cf. GET_FW_STATUS = 0x17C50106).
 * -------------------------------------------------------------------------- */

/* ELE enclave framing for the FW bring-up commands. */

/* ELE command opcodes (header bits 23-16). */

/* ELE request sizes in 32-bit WORDS (including the header word).
 * FW auth carries the image address as data[0]=lower32, data[1]=upper32,
 * data[2]=address (mirrors the Linux driver), so header + 3 data = 4 words.
 * Get-state carries no payload, so just the header word. */

/* V2X firmware state (low byte of the ELE get-state response data word). */

/* Build an ELE command header with the enclave command tag and version. */

#endif /* V2X_BASE_INTERNAL_H_ */
