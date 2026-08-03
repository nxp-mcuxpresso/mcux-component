/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * S100-specific ELE command definitions.
 *
 * This header supplements ele_crypto_internal.h for devices that use the
 * ELE S100 firmware variant (e.g. i.MX RT2660).  The S100 differs from
 * S400 in the following ways:
 *
 * - Plaintext-key crypto is NOT a separate "Generic" API (0xC8-0xCE).
 *   Instead a PLAINTEXT flag (bit 3 of the Flags field) is added to the
 *   regular HSM commands 0x52 (MAC), 0x62 (Cipher), 0x65 (AEAD), 0x72
 *   (Sign).  When this flag is set the key identifier field carries a
 *   host-memory address instead of an opaque key ID.
 *
 * - The AEAD opcode changes from 0x64 to 0x65 and the message layout is
 *   completely redesigned (19 words vs 13 words).
 *
 * - The Cipher (0x62), MAC (0x52) and Sign (0x72) commands gain extra
 *   words for key type/size so the S100 message sizes differ from S400.
 *
 * - A new KEY DERIVATION command (0x58, HKDF) is available.
 *
 * - The NVM Manager protocol (0xE0-0xE7) is identical to S400.
 *
 * - S400-only commands (Generic 0xC8-0xCE, Fast MAC 0xD0-D2,
 *   Generate/Load Key Blob 0xAF/0xA7, Enable OTFAD 0xC6) are absent.
 */

#ifndef ELE_CRYPTO_S100_INTERNAL_H
#define ELE_CRYPTO_S100_INTERNAL_H

#include "ele_crypto_internal.h"

/*******************************************************************************
 * S100 MAC one-go (0x52) - 13-word format
 * Adds key-type/key-size fields and three reserved words compared to S400.
 ******************************************************************************/
#define MAC_ONE_GO_S100_HDR      (0x17520D07u) /* cmd 0x52, 13 words, ver 0x07 */
#define MAC_ONE_GO_S100_SIZE     (0xDu)
#define MAC_ONE_GO_S100_RSP_HDR  (0xe1520407u) /* response same as S400 */

/* Bit 3 of the 16-bit Flags field: supply plaintext key address instead of
 * an opaque key ID.  Set simultaneously with the key address in word 2. */
#define MAC_PLAINTEXT_KEY_FLAG (0x0008u)

/*******************************************************************************
 * S100 Cipher one-go (0x62) - 15-word format
 * Adds key-type/key-size fields and three reserved words.
 ******************************************************************************/
#define CIPHER_ONE_GO_S100_HDR     (0x17620F07u) /* cmd 0x62, 15 words, ver 0x07 */
#define CIPHER_ONE_GO_S100_SIZE    (0xFu)
#define CIPHER_ONE_GO_S100_RSP_HDR (0xe1620307u) /* response same as S400 */

/* Bit 3 of the 16-bit Flags field: plaintext key. */
#define CIPHER_PLAINTEXT_KEY_FLAG (0x0008u)

/*******************************************************************************
 * S100 AEAD (0x65) - 19-word format  [replaces S400 0x64]
 *
 * Word layout:
 *   0  : header  Tag(0x17) Cmd(0x65) Size(0x13) Ver(0x07)
 *   1  : cipher handle (opaque) or reserved (plaintext)
 *   2  : algorithm
 *   3  : IV size[15:0] | Flags[15:0]
 *   4  : IV input address
 *   5  : IV output address (when ELE generates IV, GCM only)
 *   6  : key ID (opaque) or key address LSB (plaintext)
 *   7  : TAG address
 *   8  : key size[15:0] | tag size[15:0]
 *   9  : AAD address
 *   10 : AAD size
 *   11 : input address
 *   12 : input size
 *   13 : output address
 *   14 : output size
 *   15 : reserved
 *   16 : key type[15:0] | reserved[15:0]
 *   17 : reserved
 *   18 : CRC
 *
 * Response (4 words):
 *   0  : Tag(0xE1) Cmd(0x65) Size(0x5) Ver(0x07)
 *   1  : response indicator
 *   2  : output size
 *   3  : verify status (0x5A3CC3A5 = success, 0x2B4DD4B2 = failure)
 *   4  : CRC
 ******************************************************************************/
#define CIPHER_AEAD_S100_HDR      (0x17651307u) /* cmd 0x65, 19 words, ver 0x07 */
#define CIPHER_AEAD_S100_SIZE     (0x13u)
#define CIPHER_AEAD_S100_RSP_HDR  (0xe1650407u)

/* Flags field (lower 16 bits of word 3) */
#define AEAD_S100_FLAG_ENCRYPT         (0x0001u) /* 1=encrypt, 0=decrypt */
#define AEAD_S100_FLAG_RBG_IV          (0x0002u) /* ELE generates full IV internally (GCM) */
#define AEAD_S100_FLAG_COUNTER_IV      (0x0004u) /* Caller supplies 4-byte counter seed (GCM) */
#define AEAD_S100_FLAG_PLAINTEXT_KEY   (0x0008u) /* word 6 = key address, not key ID */
#define AEAD_S100_FLAG_ONE_SHOT        (0x0010u) /* must always be set */

/* Verify status returned in response word 3 */
#define AEAD_S100_VERIFY_SUCCESS (0x5A3CC3A5u)
#define AEAD_S100_VERIFY_FAILURE (0x2B4DD4B2u)

/* TAG size macro */
#define AEAD_TAG_SIZE (16u)

/*******************************************************************************
 * S100 Sign (0x72) - 12-word format
 * Adds private-key-size / keypair-security-size field compared to S400 (10w).
 ******************************************************************************/
#define SIGN_CMD_S100_HDR     (0x17720C07u) /* cmd 0x72, 12 words, ver 0x07 */
#define SIGN_CMD_S100_SIZE    (0xCu)
#define SIGN_CMD_S100_RSP_HDR (0xe1720307u) /* response same as S400 */

/* Bit 3 of the 8-bit Flags field (word 6 bits [7:0]): plaintext key. */
#define SIGN_PLAINTEXT_KEY_FLAG (0x08u)

/*******************************************************************************
 * S100 Key Derivation (0x58) - HKDF  [new command, absent in S400]
 *
 * Word layout:
 *   0 : header  Tag(0x17) Cmd(0x58) Size(0xA) Ver(0x07)
 *   1 : key management handle
 *   2 : flags
 *   3 : input key ID (opaque) or IKM address LSB (plaintext, flag bit 0)
 *   4 : output key ID
 *   5 : salt address
 *   6 : info address
 *   7 : IKM size[15:0] | salt size[15:0]
 *   8 : info size[15:0] | OKM size[15:0]
 *   9 : CRC
 *
 * Flags bit 0: 0 = opaque key in keystore, 1 = plaintext (IKM/OKM in user mem)
 ******************************************************************************/
#define KEY_DERIVE_S100_HDR       (0x17580A07u) /* cmd 0x58, 10 words, ver 0x07 */
#define KEY_DERIVE_S100_SIZE      (0xAu)
#define KEY_DERIVE_S100_RSP_HDR   (0xe1580207u)

#define KEY_DERIVE_FLAG_PLAINTEXT (0x01u) /* IKM and OKM are in user memory */

/* Supported HKDF algorithms */
#define HKDF_SHA256_EXTRACT_EXPAND (0x08000109u) /* full HKDF */
#define HKDF_SHA256_EXTRACT_ONLY   (0x08000409u)
#define HKDF_SHA256_EXPAND_ONLY    (0x08000509u)

/*******************************************************************************
 * S100: commands NOT available (S400 only)
 * These macros are intentionally absent.  Attempting to compile code that
 * references them will produce a clear build error, prompting the developer
 * to use the S100 equivalents above.
 *
 *  GENERIC_CIPHER       (0xC8) -> use CIPHER_ONE_GO_S100 + CIPHER_PLAINTEXT_KEY_FLAG
 *  GENERIC_AEAD         (0xC9) -> use CIPHER_AEAD_S100   + AEAD_S100_FLAG_PLAINTEXT_KEY
 *  GENERIC_ASYMMETRIC   (0xCA) -> use SIGN_CMD_S100      + SIGN_PLAINTEXT_KEY_FLAG
 *  GENERIC_RSA_KEYGEN   (0xCB) -> use KEY_GEN (0x42) with plaintext flag
 *  HMAC_GENERIC         (0xCE) -> use MAC_ONE_GO_S100    + MAC_PLAINTEXT_KEY_FLAG
 *  FAST_MAC_START/END   (0xD0-D2) -> not supported
 *  GENERATE_KEY_BLOB    (0xAF) -> not supported (RT1180 OTFAD/IEE only)
 *  LOAD_KEY_BLOB        (0xA7) -> not supported
 *  ENABLE_OTFAD         (0xC6) -> not supported
 *  CIPHER_AEAD          (0x64) -> replaced by CIPHER_AEAD_S100 (0x65)
 ******************************************************************************/

#endif /* ELE_CRYPTO_S100_INTERNAL_H */
