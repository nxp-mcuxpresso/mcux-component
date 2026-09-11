/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Public API for the EdgeLock V2X / FCE (Fast Crypto Engine) firmware service.
 *
 * This component is a sibling of ele_crypto and v2x_crypto. ele_crypto drives
 * the ELE HSM firmware over an ELE MU; v2x_crypto drives the V2X-FH firmware
 * over the V2X-FH message unit; v2x_fce drives the V2X/FCE service (an
 * LTC-AES core coupled with an AHB-DMA engine on the V2X enclave) over its
 * dedicated message unit. This driver talks to the MU directly with the
 * generic MU hardware driver (fsl_mu) using the enclave message framing.
 *
 * As with the sibling components, every entry point that talks to the MU takes
 * the MU register base. There is no fixed SDK base-address macro for the
 * V2X/FCE MU on i.MX943; the caller selects the instance (the Linux DTS node
 * v2x_mu2 is at 0x47320000).
 *
 * FCE command IDs (see the EdgeLock Secure Enclave User Guide, V2X/FCE):
 *   FCE_PING                 0x01
 *   FCE_SERVICE_OPEN         0x10
 *   FCE_SERVICE_CLOSE        0x11
 *   FCE_GET_INFO             0x18
 *   FCE_LOAD_AES_PLAIN_KEY   0x41
 *   FCE_LOAD_HMAC_PLAIN_KEY  0x45
 *   FCE_PUSH_REQUEST         0xB# (AES-GCM authenticate encrypt/decrypt, ...)
 *   FCE_GET_REQUEST_STATUS   0xC0
 */

#ifndef V2X_FCE_H_
#define V2X_FCE_H_

#include "fsl_common.h"
#include "fsl_mu.h"

/*!
 * @addtogroup v2x_fce
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief v2x_fce driver version 1.2.0. */
#define FSL_V2X_FCE_DRIVER_VERSION (MAKE_VERSION(1, 2, 0))

/*! @brief Number of AES plain-key slots exposed by the FCE (0..7). */
#define V2X_FCE_AES_KEY_SLOTS  (8u)
/*! @brief Number of HMAC plain-key slots exposed by the FCE (0..7). */
#define V2X_FCE_HMAC_KEY_SLOTS (8u)

/*! @brief Handle for an open V2X/FCE service.
 *
 * Populated by V2X_FCE_Open() and passed to every subsequent call. The caller
 * treats this as opaque; the fields are managed by the driver.
 */
typedef struct _v2x_fce_handle
{
    MU_Type *mu;        /*!< V2X/FCE MU register base used by this service. */
    uint32_t serviceId; /*!< Service handle returned by FCE_SERVICE_OPEN. */
} v2x_fce_handle_t;

/*! @brief Parameters for a single AES-GCM authenticated operation.
 *
 * Buffers referenced by the DMA engine should be 64-bit aligned. For encrypt,
 * @c output receives the ciphertext and @c tag receives the authentication tag.
 * For decrypt, @c input is the ciphertext, @c tag is the tag to verify, and
 * @c output receives the recovered plaintext.
 */
typedef struct _v2x_fce_gcm_op
{
    uint8_t keySlot;        /*!< AES key slot previously loaded (0..7). */
    const uint8_t *iv;      /*!< Initialization vector (nonce). */
    uint32_t ivSize;        /*!< IV length in bytes (12 recommended for GCM). */
    const uint8_t *aad;     /*!< Additional authenticated data (may be NULL). */
    uint32_t aadSize;       /*!< AAD length in bytes (0 if none). */
    const uint8_t *input;   /*!< Plaintext (encrypt) or ciphertext (decrypt). */
    uint32_t inputSize;     /*!< Input length in bytes (<= 0x1fffc0). */
    uint8_t *output;        /*!< Ciphertext (encrypt) or plaintext (decrypt). */
    uint8_t *tag;           /*!< Auth tag out (encrypt) or in (decrypt). */
    uint32_t tagSize;       /*!< Tag length in bytes (typically 16). */
} v2x_fce_gcm_op_t;

/*! @brief SHA algorithm selector for the FCE HASH / HMAC operations. */
typedef enum _v2x_fce_sha_algo
{
    kV2X_FCE_Sha256 = 1u, /*!< SHA-256, 32-byte digest. */
    kV2X_FCE_Sha384 = 2u, /*!< SHA-384, 48-byte digest. */
    kV2X_FCE_Sha512 = 3u, /*!< SHA-512, 64-byte digest. */
} v2x_fce_sha_algo_t;

/*! @brief Parameters for a single HMAC generate/verify operation.
 *
 * Buffers referenced by the DMA engine should be 64-bit aligned. For HMAC
 * generate, @c digest receives the computed MAC. For HMAC verify, @c digest
 * carries the expected MAC to check; the call fails if it does not match.
 */
typedef struct _v2x_fce_hmac_op
{
    uint8_t keySlot;             /*!< HMAC key slot previously loaded (0..7). */
    v2x_fce_sha_algo_t shaAlgo;  /*!< SHA algorithm to use. */
    const uint8_t *input;        /*!< Message to authenticate. */
    uint32_t inputSize;          /*!< Message length in bytes. */
    uint8_t *digest;             /*!< MAC out (generate) or in (verify). */
    uint32_t digestSize;         /*!< MAC length in bytes (algorithm digest size). */
} v2x_fce_hmac_op_t;

/*! @brief Parameters for a single HASH generate operation (keyless).
 *
 * Buffers referenced by the DMA engine should be 64-bit aligned.
 */
typedef struct _v2x_fce_hash_op
{
    v2x_fce_sha_algo_t shaAlgo;  /*!< SHA algorithm to use. */
    const uint8_t *input;        /*!< Message to hash. */
    uint32_t inputSize;          /*!< Message length in bytes. */
    uint8_t *digest;             /*!< Digest out. */
    uint32_t digestSize;         /*!< Digest length in bytes (algorithm digest size). */
} v2x_fce_hash_op_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Send a PING to the V2X/FCE firmware service.
 *
 * Header-only liveness probe (command 0x01). Useful to confirm the
 * enclave answers on the FCE MU before attempting a Service Open.
 *
 * @param mu V2X/FCE MU register base.
 * @return kStatus_Success if the firmware answered the ping.
 */
status_t V2X_FCE_Ping(MU_Type *mu);

/*!
 * @brief Open the V2X/FCE service on the given MU (FCE_SERVICE_OPEN, 0x10).
 *
 * @param mu     V2X/FCE MU register base.
 * @param handle On success, receives the initialized service handle.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
status_t V2X_FCE_Open(MU_Type *mu, v2x_fce_handle_t *handle);

/*!
 * @brief Close a previously opened V2X/FCE service (FCE_SERVICE_CLOSE, 0x11).
 *
 * @param handle Service handle returned by V2X_FCE_Open.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
status_t V2X_FCE_Close(v2x_fce_handle_t *handle);

/*!
 * @brief Load a plain AES key into an AES key slot (FCE_LOAD_AES_PLAIN_KEY, 0x41).
 *
 * @param handle  Service handle.
 * @param keySlot Destination AES key slot (0..V2X_FCE_AES_KEY_SLOTS-1).
 * @param key     Plain key bytes.
 * @param keySize Key length in bytes (16, 24 or 32).
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
status_t V2X_FCE_LoadAesPlainKey(v2x_fce_handle_t *handle, uint8_t keySlot, const uint8_t *key, uint32_t keySize);

/*!
 * @brief Load a plain HMAC key into an HMAC key slot (FCE_LOAD_HMAC_PLAIN_KEY, 0x45).
 *
 * @param handle  Service handle.
 * @param keySlot Destination HMAC key slot (0..V2X_FCE_HMAC_KEY_SLOTS-1).
 * @param shaAlgo SHA algorithm the key is bound to (must match the MAC op).
 * @param key     Plain key bytes.
 * @param keySize Key length in bytes.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
status_t V2X_FCE_LoadHmacPlainKey(v2x_fce_handle_t *handle, uint8_t keySlot, v2x_fce_sha_algo_t shaAlgo, const uint8_t *key, uint32_t keySize);

/*!
 * @brief AES-GCM authenticated encrypt (FCE_PUSH_REQUEST authenticate-encrypt).
 *
 * Produces @c op->output ciphertext and @c op->tag using the key previously
 * loaded into @c op->keySlot.
 *
 * @param handle Service handle.
 * @param op     AES-GCM operation descriptor.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
status_t V2X_FCE_AesGcmEncrypt(v2x_fce_handle_t *handle, const v2x_fce_gcm_op_t *op);

/*!
 * @brief AES-GCM authenticated decrypt (FCE_PUSH_REQUEST authenticate-decrypt).
 *
 * Verifies @c op->tag and, on success, writes the recovered plaintext to
 * @c op->output. Returns kStatus_Fail if authentication fails.
 *
 * @param handle Service handle.
 * @param op     AES-GCM operation descriptor.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
status_t V2X_FCE_AesGcmDecrypt(v2x_fce_handle_t *handle, const v2x_fce_gcm_op_t *op);

/*!
 * @brief Query V2X/FCE firmware/engine information (FCE_GET_INFO, 0x18).
 *
 * @param handle Service handle.
 * @param buffer 64-bit aligned buffer that receives the info structure.
 * @param size   Size of buffer in bytes.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
/*!
 * @brief HMAC generate (FCE_PUSH_REQUEST HMAC generate).
 *
 * Computes the HMAC of @c op->input using the key previously loaded into
 * @c op->keySlot and writes it to @c op->digest.
 *
 * @param handle Service handle.
 * @param op     HMAC operation descriptor.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
status_t V2X_FCE_HmacGenerate(v2x_fce_handle_t *handle, const v2x_fce_hmac_op_t *op);

/*!
 * @brief HMAC verify (FCE_PUSH_REQUEST HMAC verify).
 *
 * Verifies @c op->digest against the HMAC of @c op->input computed with the key
 * previously loaded into @c op->keySlot. Returns kStatus_Fail if the MAC does
 * not match.
 *
 * @param handle Service handle.
 * @param op     HMAC operation descriptor.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
status_t V2X_FCE_HmacVerify(v2x_fce_handle_t *handle, const v2x_fce_hmac_op_t *op);

/*!
 * @brief HASH generate (FCE_PUSH_REQUEST HASH generate).
 *
 * Computes the SHA digest of @c op->input and writes it to @c op->digest. No
 * key slot is used.
 *
 * @param handle Service handle.
 * @param op     HASH operation descriptor.
 * @return kStatus_Success on success, kStatus_Fail otherwise.
 */
status_t V2X_FCE_HashGenerate(v2x_fce_handle_t *handle, const v2x_fce_hash_op_t *op);

status_t V2X_FCE_GetInfo(v2x_fce_handle_t *handle, uint8_t *buffer, uint32_t size);

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* V2X_FCE_H_ */
