/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ELE S100 backend implementation.
 *
 * This file is compiled only for devices that use the ELE S100 firmware
 * variant (selected via FSL_FEATURE_ELE_IS_S100).  It provides the same
 * public API as ele_crypto.c (S400) but targets the S100 MU message
 * formats.
 *
 * Key differences vs. S400:
 *   - Cipher (0x62) : 15-word format; key type/size appended.
 *   - MAC   (0x52) : 13-word format; key type/size appended.
 *   - AEAD  (0x65) : 19-word format (opcode changes from 0x64).
 *   - Sign  (0x72) : 12-word format; private-key-size field added.
 *   - Generic plaintext-key ops use HSM commands + PLAINTEXT flag instead
 *     of the S400-only 0xC8-0xCE commands.
 *   - Key Derivation (0x58 HKDF) is new in S100.
 *   - S400-only commands (FastMAC, KeyBlob, OTFAD) return kStatus_Fail.
 *
 * All NVM manager interactions (0xE0-0xE7) are shared and implemented in
 * ele_nvm_manager.c.
 */

#if defined(FSL_FEATURE_ELE_IS_S100) && (FSL_FEATURE_ELE_IS_S100 > 0)

#include <string.h>
#include "ele_crypto.h"
#include "ele_crypto_internal.h"
#include "ele_crypto_s100_internal.h"

/*******************************************************************************
 * ELE_Aead  (S100 - 19-word format, opcode 0x65)
 *
 * Matches the S400 API: ele_hsm_cipher_t carries IV, input, output, key_id,
 * mode (algorithm), flags and sizes.  The additional AAD pointer and size are
 * passed as aad / aad_size parameters (words 9-10).
 * iv_gen_flags is ORed into the Flags field (word 3 [15:0]).
 * On return, conf->output_size is updated with the actual output byte count
 * reported by ELE in resp[2].
 ******************************************************************************/
status_t ELE_Aead(S3MU_Type *mu,
                  uint32_t cipherHandleID,
                  ele_hsm_cipher_t *conf,
                  uint32_t aad,
                  size_t aad_size,
                  iv_generation_t iv_gen_flags)
{
    if (mu == NULL || conf == NULL)
    {
        return kStatus_InvalidArgument;
    }

    uint32_t msg[CIPHER_AEAD_S100_SIZE] = {0u};
    uint32_t resp[MSG_RESPONSE_MAX]     = {0u};
    uint32_t tag_addr = 0u;

    uint16_t flags = AEAD_S100_FLAG_ONE_SHOT | (uint16_t)iv_gen_flags;
    
    /* Size of output data in bytes. If not provided the input size is used */
    uint32_t out_size = (conf->output_size != NULL) ? (uint32_t)*conf->output_size : conf->input_size;

    /* For encrypt we append the tag to the output buffer. */
    /* For decrypt we expect tag to be appended at the end of the cipher data */
    if (conf->mode == kHSM_Encrypt)
    {
        flags |= AEAD_S100_FLAG_ENCRYPT;
        tag_addr = (uint32_t)(conf->output + (out_size - AEAD_TAG_SIZE));
    }
    else if (conf->mode == kHSM_Decrypt)
    {
        tag_addr = (uint32_t)(conf->input + (conf->input_size - AEAD_TAG_SIZE));
    }

    /* For GCM the tag is appended to / stripped from the output buffer by ELE.
     * ele_hsm_cipher_t has no separate tag field; pass address 0 (ELE-managed). */
    msg[0]  = CIPHER_AEAD_S100_HDR;
    msg[1]  = cipherHandleID;
    msg[2]  = (uint32_t)conf->alg;                     /* algorithm / AEAD alg */
    msg[3]  = ((uint32_t)conf->iv_size << SHIFT_16) | (uint32_t)flags;
    msg[4]  = (uint32_t)(uintptr_t)conf->iv;           /* IV input address */
    msg[5]  = 0u;                                      /* IV output address (ELE-generated IV) */
    msg[6]  = conf->keyID;                             /* opaque key ID */
    msg[7]  = tag_addr;                                /* tag address - ELE appends tag to output */
    msg[8]  = AEAD_TAG_SIZE;                           /* tag size - only supported 16 bytes */
    msg[9]  = aad;                                     /* AAD buffer address */
    msg[10] = (uint32_t)aad_size;
    msg[11] = (uint32_t)(uintptr_t)conf->input;
    msg[12] = conf->input_size;
    msg[13] = (uint32_t)(uintptr_t)conf->output;
    msg[14] = out_size;
    msg[15] = 0u; /* reserved */
    msg[16] = 0u; /* key type[15:0] | reserved[15:0] */
    msg[17] = 0u; /* reserved */
    msg[18] = S3MU_ComputeMsgCrc(msg, CIPHER_AEAD_S100_SIZE - 1u);

    status_t status = S3MU_SendMessage(mu, msg, CIPHER_AEAD_S100_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = ele_mu_get_response(mu, resp);
    if (status != kStatus_Success)
    {
        return status;
    }
    if ((resp[0] != CIPHER_AEAD_S100_RSP_HDR) || ((resp[1] & 0xFFu) != RESPONSE_SUCCESS))
    {
        return kStatus_Fail;
    }
    /* Write back actual output size reported by ELE */
    if (conf->output_size != NULL)
    {
        *conf->output_size = resp[2];
        if (conf->mode == kHSM_Encrypt)
        {
            *conf->output_size += AEAD_TAG_SIZE; /* append tag */
        }
    }

    /* Check verify status */
    if (resp[3] != AEAD_S100_VERIFY_SUCCESS)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

/*******************************************************************************
 * ELE_KeyDerivation  (S100 new - HKDF via 0x58)
 *
 * Performs HKDF key derivation.  The derived key is stored in the key store
 * identified by keyHandleID.
 *
 * @param mu           MU peripheral base address.
 * @param keyHandleID  Key management service handle.
 * @param algorithm    Derivation algorithm (use HKDF_SHA256_* constants).
 * @param plaintext    If true, ikm and okm buffers are in user memory;
 *                     otherwise ikm_key_id / okm_key_id identify keystore keys.
 * @param ikm_key_id   Input keying material key ID (opaque) or IKM address (plaintext).
 * @param okm_key_id   Output keying material key ID (used only when plaintext=false).
 * @param salt         Optional salt buffer (NULL = zeroed salt per RFC 5869).
 * @param salt_size    Salt size in bytes.
 * @param info         Optional context / label buffer.
 * @param info_size    Info size in bytes.
 * @param ikm_size     IKM size in bytes (plaintext mode only).
 * @param okm_size     Desired OKM size in bytes.
 *
 * @return kStatus_Success on success.
 ******************************************************************************/
status_t ELE_KeyDerivation(S3MU_Type *mu,
                           uint32_t keyHandleID,
                           uint32_t algorithm,
                           bool plaintext,
                           uint32_t ikm_key_id,
                           uint32_t okm_key_id,
                           const uint8_t *salt,
                           uint32_t salt_size,
                           const uint8_t *info,
                           uint32_t info_size,
                           uint32_t ikm_size,
                           uint32_t okm_size)
{
    if (mu == NULL)
    {
        return kStatus_InvalidArgument;
    }

    uint32_t msg[KEY_DERIVE_S100_SIZE] = {0u};
    uint32_t resp[MSG_RESPONSE_MAX]    = {0u};

    uint32_t flags = plaintext ? KEY_DERIVE_FLAG_PLAINTEXT : 0u;

    msg[0] = KEY_DERIVE_S100_HDR;
    msg[1] = keyHandleID;
    msg[2] = flags;
    msg[3] = ikm_key_id; /* opaque key ID or IKM address LSB */
    msg[4] = okm_key_id; /* output key ID (opaque) or OKM address LSB */
    msg[5] = (uint32_t)(uintptr_t)salt;
    msg[6] = (uint32_t)(uintptr_t)info;
    msg[7] = (ikm_size << SHIFT_16) | (salt_size & 0xFFFFu);
    msg[8] = (info_size << SHIFT_16) | (okm_size & 0xFFFFu);
    msg[9] = S3MU_ComputeMsgCrc(msg, KEY_DERIVE_S100_SIZE - 1u);

    status_t status = S3MU_SendMessage(mu, msg, KEY_DERIVE_S100_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = ele_mu_get_response(mu, resp);
    if (status != kStatus_Success)
    {
        return status;
    }
    if ((resp[0] != KEY_DERIVE_S100_RSP_HDR) || ((resp[1] & 0xFFu) != RESPONSE_SUCCESS))
    {
        return kStatus_Fail;
    }
    return kStatus_Success;
}

/*******************************************************************************
 * ELE_GenericHmac  (S100: route through MAC HSM command with plaintext flag)
 ******************************************************************************/
status_t ELE_GenericHmac(S3MU_Type *mu,
                         const uint8_t *msg_in,
                         uint16_t size,
                         uint8_t *out,
                         uint32_t *out_size,
                         const uint8_t *key,
                         uint16_t key_size,
                         mac_algo_t alg)
{
    if (mu == NULL || key == NULL || msg_in == NULL || out == NULL)
    {
        return kStatus_InvalidArgument;
    }

    uint32_t msg[MAC_ONE_GO_S100_SIZE] = {0u};
    uint32_t resp[MSG_RESPONSE_MAX]    = {0u};

    /* PLAINTEXT key flag, HMAC generation flag */
    uint16_t flags    = MAC_PLAINTEXT_KEY_FLAG | 0x1;
    uint32_t mac_size = (out_size != NULL) ? *out_size : 32u; /* default 32 bytes (HMAC-SHA256) */

    msg[0] = MAC_ONE_GO_S100_HDR;
    msg[1] = 0u;                                                   /* no session handle for plaintext op */
    msg[2] = (uint32_t)(uintptr_t)key;                            /* key address (plaintext) */
    msg[3] = (uint32_t)(uintptr_t)msg_in;
    msg[4] = (uint32_t)(uintptr_t)out;
    msg[5] = (uint32_t)size;
    msg[6] = ((uint32_t)flags << SHIFT_16) | ((uint32_t)mac_size & MAC_ONE_GO_MAC_SIZE_MASK);
    msg[7] = (uint32_t)alg;
    msg[8] = (0x1100u << SHIFT_16) | ((uint32_t)key_size); /* HMAC type | key size in bits */
    msg[9] = 0u; /* reserved */
    msg[10] = 0u; /* reserved */
    msg[11] = 0u; /* reserved */
    msg[12] = S3MU_ComputeMsgCrc(msg, MAC_ONE_GO_S100_SIZE - 1u);

    status_t status = S3MU_SendMessage(mu, msg, MAC_ONE_GO_S100_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = ele_mu_get_response(mu, resp);
    if (status != kStatus_Success)
    {
        return status;
    }
    if ((resp[0] != MAC_ONE_GO_S100_RSP_HDR) || ((resp[1] & 0xFFu) != RESPONSE_SUCCESS))
    {
        return kStatus_Fail;
    }
    if (out_size != NULL)
    {
        /* Response word 2 is the verification status (verify only); the
         * generated MAC output size is reported in response word 3 [15:0]. */
        *out_size = resp[3] & 0xFFFFu;
    }
    return kStatus_Success;
}

/*******************************************************************************
 * ELE_GenericCipher  (S100: route through Cipher HSM command with plaintext flag)
 ******************************************************************************/
status_t ELE_GenericCipher(S3MU_Type *mu, ele_generic_cipher_t *conf)
{
    if (mu == NULL || conf == NULL)
    {
        return kStatus_InvalidArgument;
    }

    uint32_t msg[CIPHER_ONE_GO_S100_SIZE] = {0u};
    uint32_t resp[MSG_RESPONSE_MAX]       = {0u};

    /* PLAINTEXT flag: word 2 = key address. Mode value for enc/dec is different,
       so we have to translate to S100 value (0x0 - decrypt or 0x1 - encrypt) */
    uint16_t flags = CIPHER_PLAINTEXT_KEY_FLAG | ((conf->mode == kDecrypt) ? 0x0u : 0x1u);

    msg[0]  = CIPHER_ONE_GO_S100_HDR;
    msg[1]  = 0u; /* no session handle for plaintext op */
    msg[2]  = conf->key; /* plaintext key address */
    msg[3]  = conf->iv;
    msg[4]  = ((uint32_t)flags << SHIFT_16) | (conf->iv_size & 0xFFFFu);
    msg[5]  = (uint32_t)conf->algo;
    msg[6]  = conf->data;
    msg[7]  = conf->output;
    msg[8]  = conf->size;
    msg[9]  = conf->size; /* output size == input size for cipher */
    msg[10] = ((uint32_t)kKeyType_AES << SHIFT_16) | (conf->key_size & 0xFFFFu);
    msg[11] = 0u; /* reserved */
    msg[12] = 0u; /* reserved */
    msg[13] = 0u; /* reserved */
    msg[14] = S3MU_ComputeMsgCrc(msg, CIPHER_ONE_GO_S100_SIZE - 1u);


    status_t status = S3MU_SendMessage(mu, msg, CIPHER_ONE_GO_S100_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = ele_mu_get_response(mu, resp);
    if (status != kStatus_Success)
    {
        return status;
    }
    if ((resp[0] != CIPHER_ONE_GO_S100_RSP_HDR) || ((resp[1] & 0xFFu) != RESPONSE_SUCCESS))
    {
        return kStatus_Fail;
    }
    return kStatus_Success;
}

/*******************************************************************************
 * ELE_GenericAead  (S100: route through AEAD 0x65 with plaintext flag)
 ******************************************************************************/
status_t ELE_GenericAead(S3MU_Type *mu, ele_generic_aead_t *conf)
{
    if (mu == NULL || conf == NULL)
    {
        return kStatus_InvalidArgument;
    }

    uint32_t msg[CIPHER_AEAD_S100_SIZE] = {0u};
    uint32_t resp[MSG_RESPONSE_MAX]     = {0u};

    uint16_t flags = AEAD_S100_FLAG_ONE_SHOT | AEAD_S100_FLAG_PLAINTEXT_KEY;
    if (conf->mode == kEncrypt)
    {
        flags |= AEAD_S100_FLAG_ENCRYPT;
    }

    msg[0]  = CIPHER_AEAD_S100_HDR;
    msg[1]  = 0u; /* no session handle for plaintext op */
    msg[2]  = (uint32_t)conf->algo;
    msg[3]  = (conf->iv_size << SHIFT_16) | (uint32_t)flags;
    msg[4]  = conf->iv;
    msg[5]  = 0u; /* IV output (not used for plaintext-key one-shot) */
    msg[6]  = conf->key;                                          /* plaintext key address */
    msg[7]  = conf->tag;
    msg[8]  = ((uint32_t)(conf->key_size) << SHIFT_16) | (conf->tag_size & 0xFFFFu);
    msg[9]  = conf->aad;
    msg[10] = conf->aad_size;
    msg[11] = conf->data;
    msg[12] = conf->size;
    msg[13] = conf->output;
    msg[14] = conf->size; /* output size == input size */
    msg[15] = 0u;
    msg[16] = (kKeyType_AES << SHIFT_16); /* AES key type[15:0] | reserved[15:0] */
    msg[17] = 0u;
    msg[18] = S3MU_ComputeMsgCrc(msg, CIPHER_AEAD_S100_SIZE - 1u);

    status_t status = S3MU_SendMessage(mu, msg, CIPHER_AEAD_S100_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = ele_mu_get_response(mu, resp);
    if (status != kStatus_Success)
    {
        return status;
    }
    if ((resp[0] != CIPHER_AEAD_S100_RSP_HDR) || ((resp[1] & 0xFFu) != RESPONSE_SUCCESS))
    {
        return kStatus_Fail;
    }
    return kStatus_Success;
}

/*******************************************************************************
 * ELE_GenericEcdsa  (S100: route through Sign 0x72 with plaintext key flag)
 ******************************************************************************/
status_t ELE_GenericEcdsa(S3MU_Type *mu, ele_generic_ecdsa_t *conf)
{
    if (mu == NULL || conf == NULL)
    {
        return kStatus_InvalidArgument;
    }

    uint32_t msg[SIGN_CMD_S100_SIZE] = {0u};
    uint32_t resp[MSG_RESPONSE_MAX]  = {0u};

    uint8_t flags = SIGN_PLAINTEXT_KEY_FLAG;

    msg[0] = SIGN_CMD_S100_HDR;
    msg[1] = 0u; /* no session handle for plaintext op */
    msg[2] = (uint32_t)(uintptr_t)conf->key;                     /* plaintext key address */
    msg[3] = conf->digest;
    msg[4] = conf->signature;
    msg[5] = conf->digest_size;
    msg[6] = ((uint32_t)flags << SHIFT_16) | (conf->signature_size & SIGN_SIGNATURE_SIZE_MASK);
    msg[7] = (uint32_t)conf->algo;
    msg[8] = 0u; /* salt length = 0 for ECDSA */
    msg[9] = conf->key_size; /* private key size */
    msg[10] = 0u; /* reserved */
    msg[11] = S3MU_ComputeMsgCrc(msg, SIGN_CMD_S100_SIZE - 1u);

    status_t status = S3MU_SendMessage(mu, msg, SIGN_CMD_S100_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = ele_mu_get_response(mu, resp);
    if (status != kStatus_Success)
    {
        return status;
    }
    if ((resp[0] != SIGN_CMD_S100_RSP_HDR) || ((resp[1] & 0xFFu) != RESPONSE_SUCCESS))
    {
        return kStatus_Fail;
    }
    conf->signature_size = resp[2] & SIGN_SIGNATURE_SIZE_MASK;
    return kStatus_Success;
}

/*******************************************************************************
 * Shared functions - same MU protocol on S100 and S400
 ******************************************************************************/
/*
 * NOTE: The following shared functions are compiled from ele_crypto.c
 * for both S100 and S400 variants and are NOT redefined here:
 *   ELE_LoadFw, ELE_Hash, ELE_Hash_Init, ELE_Hash_Update, ELE_Hash_Finish,
 *   ELE_StartRng, ELE_GetTrngState, ELE_RngGetRandom,
 *   ELE_OpenCipherService, ELE_CloseCipherService,
 *   ELE_OpenMacService,  ELE_CloseMacService,
 *   ELE_OpenSignService,  ELE_CloseSignService,
 *   ELE_OpenVerifyService, ELE_CloseVerifyService,
 *   ELE_Verify
 */


status_t ELE_GenericRsa(S3MU_Type *mu, ele_generic_rsa_t *conf)
{
    (void)mu;
    (void)conf;
    return kStatus_Fail; /* Not supported on ELE S100 */
}

status_t ELE_GenericRsaKeygen(S3MU_Type *mu, ele_generic_rsa_t *conf)
{
    (void)mu;
    (void)conf;
    return kStatus_Fail; /* Not supported on ELE S100 */
}

status_t ELE_FastMacStart(S3MU_Type *mu, const uint8_t *key)
{
    (void)mu;
    (void)key;
    return kStatus_Fail; /* Not supported on ELE S100 */
}

status_t ELE_FastMacProceed(
    S3MU_Type *mu, const uint8_t *msg, uint8_t *mac, uint16_t msgSize, uint16_t flags, uint32_t *verifStatus)
{
    (void)mu;
    (void)msg;
    (void)mac;
    (void)msgSize;
    (void)flags;
    (void)verifStatus;
    return kStatus_Fail; /* Not supported on ELE S100 */
}

status_t ELE_FastMacEnd(S3MU_Type *mu)
{
    (void)mu;
    return kStatus_Fail; /* Not supported on ELE S100 */
}

status_t ELE_GenerateKeyBlob(
    S3MU_Type *mu, uint32_t KeyID, generate_key_blob_input_t *Conf, uint8_t *OutputBuffer, uint16_t OutputBufferSize)
{
    (void)mu;
    (void)KeyID;
    (void)Conf;
    (void)OutputBuffer;
    (void)OutputBufferSize;
    return kStatus_Fail; /* Not supported on ELE S100 (RT1180 OTFAD/IEE only) */
}

status_t ELE_LoadKeyBlob(S3MU_Type *mu, uint32_t KeyID, uint8_t *BlobAddress)
{
    (void)mu;
    (void)KeyID;
    (void)BlobAddress;
    return kStatus_Fail; /* Not supported on ELE S100 (RT1180 only) */
}

status_t ELE_EnableOtfad(S3MU_Type *mu, uint8_t OtfadID)
{
    (void)mu;
    (void)OtfadID;
    return kStatus_Fail; /* Not supported on ELE S100 (RT1180 OTFAD only) */
}

#endif /* FSL_FEATURE_ELE_IS_S100 */
