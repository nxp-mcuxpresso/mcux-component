/*
 * Copyright 2024-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ELE S400 (ELE400/ELE401) backend - S400-specific crypto functions.
 *
 * This file is compiled only when building for devices
 * that use the ELE S400/ELE400/ELE401 firmware variant.
 * Common functions (session/key management, Hash, RNG, Open/Close services,
 * ELE_Mac, ELE_Cipher, ELE_Sign, ELE_Verify) are in ele_crypto.c.
 *
 * Functions provided here:
 *   ELE_GenericHmac, ELE_Aead,
 *   ELE_GenericCipher, ELE_GenericAead, ELE_GenericEcdsa,
 *   ELE_GenericRsa, ELE_GenericRsaKeygen,
 *   ELE_EnableOtfad,
 *   ELE_FastMacStart, ELE_FastMacProceed, ELE_FastMacEnd,
 *   ELE_GenerateKeyBlob, ELE_LoadKeyBlob
 */

#if !defined(FSL_FEATURE_ELE_IS_S100)

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "ele_crypto.h"
#include "ele_crypto_internal.h"
#include "fsl_s3mu.h"
#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
#include "fsl_cache.h"
#endif /* ELE_CACHE_HANDLING */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static status_t construct_dek_key_blob_input(generate_key_blob_input_t *conf, uint32_t *payload);
#if defined(FSL_FEATURE_SOC_OTFAD_COUNT) && (FSL_FEATURE_SOC_OTFAD_COUNT > 0)
static status_t construct_otfad_key_blob_input(generate_key_blob_input_t *conf, uint32_t *payload);
#endif
#if defined(FSL_FEATURE_SOC_IEE_COUNT) && (FSL_FEATURE_SOC_IEE_COUNT > 0)
static status_t construct_iee_key_blob_input(generate_key_blob_input_t *conf, uint32_t *payload);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * brief ELE HMAC Generic
 *
 * This function computes HMAC (One Go) of a given message using EdgeLock Enclave.
 *
 * param mu MU peripheral base address
 * param msg pointer where input message data can be found, may be zero
 * param size size of input data in bytes, may be zero
 * param out pointer where the MAC buffer must be written by ELE (size depends on used alg)
 * param out_size pointer to the resulting output size of the chosen HMAC
 * param key pointer where the buffer with key in plaintext can be found
 * param key_size size of key in bytes
 * param alg define algorithm used for HMAC, see mac_algo_t
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail,
 *        kStatus_InvalidArgument if invalid argument
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_GenericHmac(S3MU_Type *mu,
                         const uint8_t *msg,
                         uint16_t size,
                         uint8_t *out,
                         uint32_t *out_size,
                         const uint8_t *key,
                         uint16_t key_size,
                         mac_algo_t alg)
{
    status_t status                  = kStatus_Fail;
    uint32_t tmsg[HMAC_GENERIC_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]     = {0u};
    uint16_t outSize                 = 0u;

    /* Check argument validity */
    if (mu == NULL || out == NULL || out_size == NULL || key == NULL)
    {
        return kStatus_InvalidArgument;
    }

    /* Currently ELE supports only SHA256 for MAC */
    switch (alg)
    {
        case kELE_Hmac256:
            outSize = 256u / 8u;
            break;
        default:
            status = kStatus_InvalidArgument;
            break;
    }

    if (status == kStatus_InvalidArgument)
    {
        return status;
    }

    tmsg[0] = HMAC_GENERIC_HDR;                         // HASH Command Header
    tmsg[1] = ADD_OFFSET((uint32_t)msg);                // Input msg address
    tmsg[2] = ADD_OFFSET((uint32_t)out);                // Output MAC address
    tmsg[3] = ADD_OFFSET((uint32_t)key);                // Key address
    tmsg[4] = size;                                     // Input size
    tmsg[5] = (uint32_t)key_size << SHIFT_16 | outSize; // Output size
    tmsg[6] = (uint32_t)alg;                            // Alg
    tmsg[7] = S3MU_ComputeMsgCrc(tmsg, HMAC_GENERIC_SIZE - 1u);
    // CRC sum of all the words of the message (excluding the CRC itself)

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, HMAC_GENERIC_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Wait for response from Security Sub-System */
    if (ele_mu_get_response(mu, rmsg) != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == HMAC_GENERIC_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        *out_size = (uint32_t)outSize;

#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
        DCACHE_InvalidateByRange((uint32_t)out, *out_size);
#endif /* defined(ELE_CACHE_HANDLING) */

        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Open ELE AEAD Cipher
 *
 * This function encrypt or decrypt data usign crypto AES AEAD with keys in EdgeLock Enclave service.
 *
 * param mu MU peripheral base address
 * param cipherHandleID unique Cipher handle ID obtained by calling ELE_OpenCipherService()
 * param conf Pointer where HSM cipher configuration structure can be found
 * param aad pointer where a additional authenticated data can be found
 * param aad_size size of aad data in bytes
 * param iv_gen_flags Relevant for GCM only. Specifies mode of generating IV for GCM, for CCM must be 0
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_Aead(S3MU_Type *mu,
                  uint32_t cipherHandleID,
                  ele_hsm_cipher_t *conf,
                  uint32_t aad,
                  size_t aad_size,
                  iv_generation_t iv_gen_flags)
{
    status_t status                 = kStatus_Fail;
    uint32_t tmsg[CIPHER_AEAD_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]    = {0u};

    uint32_t flags = (uint32_t)iv_gen_flags | (uint32_t)conf->mode; // IV generation flags | Mode

    tmsg[0]  = CIPHER_AEAD;                                         // GENERIC_AEAD Command Header
    tmsg[1]  = cipherHandleID;                                      // Cipher handle ID
    tmsg[2]  = conf->keyID;                                         // key ID
    tmsg[3]  = ADD_OFFSET(conf->iv);                                // Initialization vector address
    tmsg[4]  = flags << SHIFT_16 | conf->iv_size;                   // Flags | Initialization vector  byte length
    tmsg[5]  = (uint32_t)conf->alg;                                 // Algorithm identifier
    tmsg[6]  = ADD_OFFSET(aad);                                     // Additional authentication data address
    tmsg[7]  = aad_size;                                            // Additional authentication data size
    tmsg[8]  = ADD_OFFSET(conf->input);                             // Input data address
    tmsg[9]  = ADD_OFFSET(conf->output);                            // Output data address
    tmsg[10] = conf->input_size;                                    // Input byte length
    tmsg[11] = (uint32_t)*conf->output_size;                        // Output byte length
    tmsg[12] = S3MU_ComputeMsgCrc(tmsg, CIPHER_AEAD_SIZE - 1u);
    // CRC sum of all the words of the message (excluding the CRC itself)

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, CIPHER_AEAD_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Check that response corresponds to the sent command */
    if (rmsg[0] == CIPHER_AEAD_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        /* read output size from ELE */
        if (conf->output_size != NULL)
        {
            *conf->output_size = rmsg[2];
        }

#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
        DCACHE_InvalidateByRange((uint32_t)conf->output, conf->input_size);
#endif /* defined(ELE_CACHE_HANDLING) */

        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Open ELE Generic Cipher
 *
 * This function encrypt or decrypt data using generic crypto cipher EdgeLock Enclave service.
 *
 * param mu MU peripheral base address
 * param conf Pointer where generic cipher configuration structure can be found
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 */
status_t ELE_GenericCipher(S3MU_Type *mu, ele_generic_cipher_t *conf)
{
    status_t status                    = kStatus_Fail;
    uint32_t tmsg[GENERIC_CIPHER_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]       = {0u};

    tmsg[0] = GENERIC_CIPHER;                                         // GENERIC_CIPHER Command Header
    tmsg[1] = ADD_OFFSET(conf->data);                                 // Source data address
    tmsg[2] = ADD_OFFSET(conf->output);                               // Destination data address
    tmsg[3] = conf->size;                                             // Data byte length
    tmsg[4] = ADD_OFFSET(conf->key);                                  // Cipher key address
    tmsg[5] = conf->key_size;                                         // Cipher key byte length
    tmsg[6] = ADD_OFFSET(conf->iv);                                   // Initialization vector address
    tmsg[7] = conf->iv_size;                                          // Initialization vector size
    tmsg[8] = (uint32_t)conf->mode << SHIFT_8 | (uint32_t)conf->algo; // Algorithm | Mode
    tmsg[9] = S3MU_ComputeMsgCrc(tmsg, GENERIC_CIPHER_SIZE - 1u);
    // CRC sum of all the words of the message (excluding the CRC itself)

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, GENERIC_CIPHER_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
    DCACHE_InvalidateByRange((uint32_t)conf->output, conf->size);
#endif /* defined(ELE_CACHE_HANDLING) */

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == GENERIC_CIPHER_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Open ELE Generic Cipher
 *
 * This function encrypt or decrypt data using generic crypto AES AEAD EdgeLock Enclave service.
 *
 * param mu MU peripheral base address
 * param conf Pointer where generic AEAD configuration structure can be found
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 */
status_t ELE_GenericAead(S3MU_Type *mu, ele_generic_aead_t *conf)
{
    status_t status                  = kStatus_Fail;
    uint32_t tmsg[GENERIC_AEAD_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]     = {0u};

    tmsg[0]  = GENERIC_AEAD;                                           // GENERIC_AEAD Command Header
    tmsg[1]  = ADD_OFFSET(conf->data);                                 // Source data address
    tmsg[2]  = ADD_OFFSET(conf->output);                               // Destination data address
    tmsg[3]  = conf->size;                                             // Data byte length
    tmsg[4]  = ADD_OFFSET(conf->key);                                  // Cipher key address
    tmsg[5]  = conf->key_size;                                         // Cipher key byte length
    tmsg[6]  = ADD_OFFSET(conf->iv);                                   // Initialization vector address
    tmsg[7]  = conf->iv_size;                                          // Initialization vector size
    tmsg[8]  = ADD_OFFSET(conf->aad);                                  // Additional authentication data address
    tmsg[9]  = conf->aad_size;                                         // Additional authentication data size
    tmsg[10] = ADD_OFFSET(conf->tag);                                  // Tag data address
    tmsg[11] = conf->tag_size;                                         // Tag data size
    tmsg[12] = (uint32_t)conf->mode << SHIFT_8 | (uint32_t)conf->algo; // Algorithm | Mode
    tmsg[13] = S3MU_ComputeMsgCrc(tmsg, GENERIC_AEAD_SIZE - 1u);
    // CRC sum of all the words of the message (excluding the CRC itself)

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, GENERIC_AEAD_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
    DCACHE_InvalidateByRange((uint32_t)conf->output, conf->size);
    DCACHE_InvalidateByRange((uint32_t)conf->tag, conf->tag_size);
#endif /* defined(ELE_CACHE_HANDLING) */

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == GENERIC_AEAD_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}


/*!
 * brief ELE Generic ECDSA
 *
 * This function sign or verify data using generic ECDSA EdgeLock Enclave service.
 *
 * param mu MU peripheral base address
 * param conf Pointer where generic ECDSA configuration structure can be found
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 */
status_t ELE_GenericEcdsa(S3MU_Type *mu, ele_generic_ecdsa_t *conf)
{
    status_t status                        = kStatus_Fail;
    uint32_t tmsg[GENERIC_ASYMMETRIC_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]           = {0u};

    tmsg[0]  = GENERIC_ASYMMETRIC;          // GENERIC_ASYMMETRIC Command Header
    tmsg[1]  = 0u;
    tmsg[2]  = conf->algo;                  // Algorithm
    tmsg[3]  = conf->key_size << SHIFT_16 | conf->mode;
    tmsg[4]  = ADD_OFFSET(conf->digest);    // Key byte length
    tmsg[5]  = ADD_OFFSET(conf->signature); // Signature address to verify/generate
    tmsg[6]  = conf->digest_size;
    tmsg[7]  = conf->signature_size;        // Signature byte length
    tmsg[8]  = ADD_OFFSET(conf->key);
    tmsg[9]  = 0u;
    tmsg[10] = conf->key_size;
    tmsg[11] = 0u;
    tmsg[12] = 0u;
    tmsg[13] = 0u;
    tmsg[14] = 0u;
    tmsg[15] = S3MU_ComputeMsgCrc(tmsg, GENERIC_ASYMMETRIC_SIZE - 1u);
    // CRC sum of all the words of the message (excluding the CRC itself)

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, GENERIC_ASYMMETRIC_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Check that response corresponds to the sent command */
    if (rmsg[0] == GENERIC_ASYMMETRIC_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        /* Get Verify status from ELE */
        conf->verify_status = (generic_verify_status_t)rmsg[2];
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}


/*!
 * brief ELE Generic RSA
 *
 * This function sign, verify, encrypt or decrypt data using generic RSA EdgeLock Enclave service.
 *
 * param mu MU peripheral base address
 * param conf Pointer where generic RSA configuration structure can be found
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail,
 *        kStatus_InvalidArgument if invalid argument
 */
status_t ELE_GenericRsa(S3MU_Type *mu, ele_generic_rsa_t *conf)
{
    status_t status                        = kStatus_Fail;
    uint32_t tmsg[GENERIC_ASYMMETRIC_SIZE] = {0u};
    uint32_t rmsg[6u]                      = {0u};

    uint32_t digestOrPlain, signatureOrCipher, pubOrPrivExp;
    uint16_t digestOrPlain_size, signatureOrCipher_size, pubOrPrivExp_size;

    /* Check argument validity */
    if (mu == NULL || conf == NULL)
    {
        return kStatus_InvalidArgument;
    }

    /* Check if salt length is valid */
    if (conf->salt_size < 0 && conf->salt_size != GENERIC_ASYMMETRIC_SALT_SIZE_EQ_HASH_SIZE)
    {
        return kStatus_InvalidArgument;
    }

    /* INT31-C: Validate size fields before narrowing conversions */
    assert(conf->pub_exponent_size <= 0xFFFFU);
    assert(conf->priv_exponent_size <= 0xFFFFU);
    assert(conf->modulus_size <= 0xFFFFU);

    /* Set inputs accoring to requested operation */
    if (conf->mode == kEncryption || conf->mode == kDecryption)
    {
        /* Check flag validity; bit 0 cannot be set for this mode */
        if (((uint32_t)conf->flags & GENERIC_RSA_INPUT_TYPE_FLAG_MASK) == (uint32_t)kFlagActualMsg)
        {
            return kStatus_InvalidArgument;
        }

        digestOrPlain          = (uint32_t)conf->plaintext;
        signatureOrCipher      = (uint32_t)conf->ciphertext;
        digestOrPlain_size     = (uint16_t)conf->plaintext_size;
        signatureOrCipher_size = (uint16_t)conf->ciphertext_size;

        if (conf->mode == kEncryption)
        {
            pubOrPrivExp = (uint32_t)conf->pub_exponent;
            pubOrPrivExp_size = (uint16_t)conf->pub_exponent_size;
        }
        else if (conf->mode == kDecryption)
        {
            pubOrPrivExp      = (uint32_t)conf->priv_exponent;
            pubOrPrivExp_size = (uint16_t)conf->priv_exponent_size;
        }
        else
        {
            return kStatus_InvalidArgument;
        }
    }
    else if (conf->mode == kSignGen || conf->mode == kVerification)
    {
        digestOrPlain          = (uint32_t)conf->digest;
        signatureOrCipher      = (uint32_t)conf->signature;
        digestOrPlain_size     = (uint16_t)conf->digest_size;
        signatureOrCipher_size = (uint16_t)conf->signature_size;

        if (conf->mode == kVerification)
        {
            pubOrPrivExp = (uint32_t)conf->pub_exponent;
            pubOrPrivExp_size = (uint16_t)conf->pub_exponent_size;
        }
        else if (conf->mode == kSignGen)
        {
            pubOrPrivExp      = (uint32_t)conf->priv_exponent;
            pubOrPrivExp_size = (uint16_t)conf->priv_exponent_size;
        }
        else
        {
            return kStatus_InvalidArgument;
        }
    }
    else
    {
        return kStatus_InvalidArgument;
    }

    uint32_t saltSize;
    if (conf->salt_size == GENERIC_ASYMMETRIC_SALT_SIZE_EQ_HASH_SIZE)
    {
        /* The MU always expects -1 in two's complement */
        saltSize = NEGATIVE_ONE_TWOS_COMPLEMENT_16_BIT;
    }
    else
    {
        /* Positive numbers can be cast */
        saltSize = (uint32_t)conf->salt_size;
    }

    tmsg[0] = GENERIC_ASYMMETRIC;   // GENERIC_ASYMMETRIC Command Header
    tmsg[1] = 0u;                   // Reserved
    tmsg[2] = (uint32_t)conf->algo; // Algorithm
    tmsg[3] =
        conf->key_size << SHIFT_16 | (uint32_t)conf->flags << SHIFT_8 | (uint32_t)conf->mode; // Key size | Flags | Mode
    tmsg[4]  = ADD_OFFSET(digestOrPlain);                                                     // Digest or plaintext
    tmsg[5]  = ADD_OFFSET(signatureOrCipher); // Signature (sign/verif) or Cipher (enc/dec) address
    tmsg[6]  = digestOrPlain_size;            // Digest (sign/verif) or Plaintext (enc/dec) size
    tmsg[7]  = signatureOrCipher_size;        // Signature (sign/verif) or Cipher (enc/dec) size
    tmsg[8]  = ADD_OFFSET(conf->modulus);     // Modulus address
    tmsg[9]  = ADD_OFFSET(pubOrPrivExp);      // Public (enc) or Private (dec) exponent address
    tmsg[10] = (uint32_t)pubOrPrivExp_size << SHIFT_16 | conf->modulus_size; // Exponent and modulus size
    tmsg[11] = 0u;                                                           // Reserved
    tmsg[12] = 0u;                                                           // Reserved
    tmsg[13] = ADD_OFFSET(conf->label);                                      // Label address
    tmsg[14] = saltSize << SHIFT_16 | conf->label_size;                      // Salt and label size
    tmsg[15] = S3MU_ComputeMsgCrc(tmsg, GENERIC_ASYMMETRIC_SIZE - 1u);
    // CRC sum of all the words of the message (excluding the CRC itself)

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, GENERIC_ASYMMETRIC_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == GENERIC_ASYMMETRIC_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        if (conf->mode == kVerification)
        {
            /* Get Verify status from ELE */
            conf->verify_status = (generic_verify_status_t)rmsg[3];
        }

        if (conf->mode == kSignGen)
        {
#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
            DCACHE_InvalidateByRange((uint32_t)signatureOrCipher, signatureOrCipher_size);
#endif /* defined(ELE_CACHE_HANDLING) */
        }

        if (conf->mode == kDecryption)
        {
            /* Get Verify status from ELE */
            conf->out_plaintext_len = rmsg[2];
#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
            DCACHE_InvalidateByRange((uint32_t)digestOrPlain, conf->out_plaintext_len);
#endif /* defined(ELE_CACHE_HANDLING) */
        }

        if (conf->mode == kEncryption)
        {
#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
            DCACHE_InvalidateByRange((uint32_t)signatureOrCipher, signatureOrCipher_size);
#endif /* defined(ELE_CACHE_HANDLING) */
        }

        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief ELE Generic RSA key gen
 *
 * This function generates RSA key-pair using generic RSA EdgeLock Enclave service.
 *
 * param mu MU peripheral base address
 * param conf Pointer where generic RSA configuration structure can be found
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 */
status_t ELE_GenericRsaKeygen(S3MU_Type *mu, ele_generic_rsa_t *conf)
{
    status_t status                        = kStatus_Fail;
    uint32_t tmsg[GENERIC_RSA_KEYGEN_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]           = {0u};

    tmsg[0]  = GENERIC_RSA_KEYGEN;                                          // GENERIC_RSA_KEYGEN Command Header
    tmsg[1]  = 0u;                                                          // MSB Buffer address (not used)
    tmsg[2]  = ADD_OFFSET(conf->modulus);                                   // Modulus address
    tmsg[3]  = ADD_OFFSET(conf->priv_exponent);                             // Key adress
    tmsg[4]  = ADD_OFFSET(conf->pub_exponent);                              // Public exponent address
    tmsg[5]  = conf->priv_exponent_size << SHIFT_16 | conf->modulus_size;   // Exponent and modulus size
    tmsg[6]  = conf->pub_exponent_size;                                     // Signature address to verify/generate
    tmsg[7]  = conf->key_size << SHIFT_16 | (uint32_t)GENERIC_RSA_KEY_PAIR; // Key size | Key type (RSA)
    tmsg[8]  = 0u;                                                          // Reserved
    tmsg[9]  = 0u;                                                          // Reserved
    tmsg[10] = S3MU_ComputeMsgCrc(tmsg, GENERIC_RSA_KEYGEN_SIZE - 1u);
    // CRC sum of all the words of the message (excluding the CRC itself)

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, GENERIC_RSA_KEYGEN_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Check that response corresponds to the sent command */
    if (rmsg[0] == GENERIC_RSA_KEYGEN_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
        DCACHE_InvalidateByRange((uint32_t)conf->modulus, conf->modulus_size);
        DCACHE_InvalidateByRange((uint32_t)conf->priv_exponent, conf->priv_exponent_size);
        DCACHE_InvalidateByRange((uint32_t)conf->pub_exponent, conf->pub_exponent_size);
#endif /* defined(ELE_CACHE_HANDLING) */

        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}


/*!
 * brief Enable an instance of OTFAD.
 *
 * param mu MU peripheral base address
 * param OtfadID ID of the OTFAD instance to enable - used only if there are
 * multiple instances on the SoC
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_EnableOtfad(S3MU_Type *mu, uint8_t OtfadID)
{
    status_t status                  = kStatus_Fail;
    uint32_t tmsg[ENABLE_OTFAD_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]     = {0u};

    /****************** Enable OTFAD message ***********************/
    tmsg[0] = ENABLE_OTFAD;      // Enable OTFAD message Command Header
#if defined(FSL_FEATURE_SOC_OTFAD_COUNT) && (FSL_FEATURE_SOC_OTFAD_COUNT > 1)
    tmsg[1] = (uint32_t)OtfadID; // ID of the OTFAD instance
#else
    (void)OtfadID; /* Prevent an unused variable warning */
#endif

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, ENABLE_OTFAD_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == ENABLE_OTFAD_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}


/*!
 * brief Fast Mac Start
 *
 * This command is used to enter in "Fast MAC" operation mode. This is the first step of the fast MAC API.
 * During this step, ELE will copy key into internal memory in order to accelerate future usage of the key.
 * ELE will also enter in a special mode where only Fast MAC will be accepted. All other commands will be rejected.
 *
 * param base MU peripheral base address
 * param key  Pointer to key data buffer, which is expected to be 64 Bytes long consisting of two 256 bit keys.
 *            The key size is hardcoded to 256 bits and other key sizes are not supported.
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_FastMacStart(S3MU_Type *mu, const uint8_t *key)
{
    status_t status                    = kStatus_Fail;
    uint32_t tmsg[FAST_MAC_START_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]       = {0u};

    tmsg[0] = FAST_MAC_START;            // FAST_MAC_START Command Header
    tmsg[1] = ADD_OFFSET((uint32_t)key); // HMAC key address
    tmsg[2] = 0x0u;                      // Reserved
    tmsg[3] = 0x0u;                      // Reserved

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, FAST_MAC_START_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Check that response corresponds to the sent command */
    if (rmsg[0] == FAST_MAC_START_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Fast Mac Proceed
 *
 * This command is used to proceed with a Fast MAC generation. The user gives as input the message buffer and size,
 * and ELE output to the User's MAC buffer the computed MAC. ELE use the key given in Start API.
 *
 * param base MU peripheral base address.
 * param msg pointer where input message data can be found.
 * param mac pointer to a buffer where the MAC data are written by ELE.
 *           If doing a OneShot operation with internal verification enabled,
 *           this buffer must hold the expected MAC value.
 * param msgSize size of message in bytes. If doing a Preload operation with
 *               internal verification enabled, this must be the length of the
 *               input message + the length of the concatenated expected MAC.
 *               note: If oneshot, limit is UIN16_MAX, otherwise 512 Bytes.
 * param flags the flags specifying Fast MAC Proceed behavior.
 *             See the FAST_MAC_* macros in ele_crypto.h.
 * param verifStatus returns the verification status after MAC computation if
 *                   the internal verification flag was enabled. May be NULL.
 *                   See the FAST_MAC_CHECK_VERIFICATION_* macros for
 *                   checking the returned status.
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_FastMacProceed(
    S3MU_Type *mu, const uint8_t *msg, uint8_t *mac, uint16_t msgSize, uint16_t flags, uint32_t *verifStatus)
{
    status_t status                      = kStatus_Fail;
    uint32_t tmsg[FAST_MAC_PROCEED_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]         = {0u};

    tmsg[0] = FAST_MAC_PROCEED;          // FAST_MAC_PROCEED Command Header
    tmsg[1] = ADD_OFFSET((uint32_t)msg); // Message address
    tmsg[2] = ADD_OFFSET((uint32_t)mac); // output Mac buffer address
    tmsg[3] = ((uint32_t)flags << 16u) | (uint32_t)msgSize;

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, FAST_MAC_PROCEED_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    if (NULL != verifStatus)
    {
        *verifStatus = rmsg[2];
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == FAST_MAC_PROCEED_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
        DCACHE_InvalidateByRange((uint32_t)mac, 32u);
#endif /* defined(ELE_CACHE_HANDLING) */
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Fast Mac End
 *
 * This command is used to exit from "Fast MAC" mode.
 *
 * param base MU peripheral base address
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_FastMacEnd(S3MU_Type *mu)
{
    status_t status                  = kStatus_Fail;
    uint32_t tmsg[FAST_MAC_END_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]     = {0u};

    tmsg[0] = FAST_MAC_END; // FAST_MAC_END Command Header
    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, FAST_MAC_END_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Check that response corresponds to the sent command */
    if (rmsg[0] == FAST_MAC_END_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}


#if defined(FSL_FEATURE_SOC_OTFAD_COUNT) && (FSL_FEATURE_SOC_OTFAD_COUNT > 0)
static status_t construct_otfad_key_blob_input(generate_key_blob_input_t *conf, uint32_t *payload)
{
    if ((conf->otfad.start_addr > conf->otfad.end_addr) || (conf->key == NULL) || (conf->ctr == NULL))
    {
        return kStatus_InvalidArgument;
    }

    /* The OTFAD key and counter */
    (void)memcpy((void *)&payload[0], (void *)conf->key, 16u);
    (void)memcpy((void *)&payload[4], (void *)conf->ctr, 8u);

    /* Address range */
    payload[6] = (conf->otfad.start_addr & OTFAD_RGD_W0_SRTADDR_MASK);
    payload[7] = (conf->otfad.end_addr & OTFAD_RGD_W1_ENDADDR_MASK);

    /* Flags in lower part of end address word */
    payload[7] |= OTFAD_RGD_W1_RO(conf->otfad.read_only) | OTFAD_RGD_W1_ADE(conf->otfad.enable_decryption) |
                  OTFAD_RGD_W1_VLD(conf->otfad.valid);

    /* Reserved and CRC fields */
    payload[8] = 0x0u;
    payload[9] = 0x0u; // CRC disabled due to side-channel vulnerability

    return kStatus_Success;
}
#endif /* FSL_FEATURE_SOC_OTFAD_COUNT */



static status_t construct_dek_key_blob_input(generate_key_blob_input_t *conf, uint32_t *payload)
{
    /* For DEK, just copy the key to the payload */
    (void)memcpy((void *)payload, (void *)conf->key, (size_t)conf->size);

    return kStatus_Success;
}


#if defined(FSL_FEATURE_SOC_IEE_COUNT) && (FSL_FEATURE_SOC_IEE_COUNT > 0)
static status_t construct_iee_key_blob_input(generate_key_blob_input_t *conf, uint32_t *payload)
{
    /* Set bypass, key size, mode, page offset */
    payload[0] =
        IEE_REGATTR_BYP(conf->iee.bypass) | IEE_REGATTR_MD(conf->iee.mode) | IEE_REGATTR_KS(conf->iee.key_size);
    payload[1] = IEE_REGPO_PGOFF(conf->iee.page_offset);

    /* Zero out the key buffers in case keys are shorter than 256-bit */
    (void)memset(&payload[2], 0, 32u);
    (void)memset(&payload[10], 0, 32u);

    /* Set the keys */
    if ((conf->iee.rand_keys == kBlob_IEE_UseInputKeys) && (conf->algorithm == kBlob_Algo_AES_CTR))
    {
        if ((conf->key == NULL) || (conf->ctr == NULL))
        {
            return kStatus_InvalidArgument;
        }

        /* contf->key is an AES key of size conf->size, conf->ctr is 128-bit counter*/
        (void)memcpy((void *)&payload[2], (void *)conf->key, (size_t)conf->size);
        (void)memcpy((void *)&payload[10], (void *)conf->ctr, 16u);
    }

    if ((conf->iee.rand_keys == kBlob_IEE_UseInputKeys) && (conf->algorithm == kBlob_Algo_AES_XTS))
    {
        if (conf->key == NULL)
        {
            return kStatus_InvalidArgument;
        }

        /* conf->key is either a 256-bit or 512-bit key */
        if (conf->size == kBlob_Size_256)
        {
            (void)memcpy((void *)&payload[2], (void *)conf->key, (size_t)conf->size);
        }
        else if (conf->size == kBlob_Size_512)
        {
            const size_t key_size_half = (size_t)conf->size / 2u;

            (void)memcpy((void *)&payload[2], (void *)conf->key, key_size_half);
            (void)memcpy((void *)&payload[10], (void *)(conf->key + key_size_half), key_size_half);
        }
        else
        {
            return kStatus_InvalidArgument;
        }
    }

    /* Region lock and region number */
    payload[18] = (((uint32_t)conf->iee.lock << SHIFT_8) & KEY_BLOB_PAYLOAD_IEE_LOCK_MASK) |
                  (((uint32_t)conf->iee.region_number) & KEY_BLOB_PAYLOAD_IEE_REGION_MASK);

    /* CRC */
    payload[19] = 0x0u; // CRC disabled due to side-channel vulnerability

    return kStatus_Success;
}
#endif /* FSL_FEATURE_SOC_IEE_COUNT */


static const uint32_t key_blob_header_template[] = {
    0x81000000u, /* Tag, Length, Version */
    0x00000000u, /* Reserved, Algorithm, Size, Flags */
};

/*!
 * brief Generate a key blob
 *
 * Used to encapsulate sensitive keys in a specific structure called a blob,
 * which provides both confidentiality and integrity protection.
 *
 * param mu MU peripheral base address
 * param KeyID key ID or OTFAD settings, depends on blob type, see baseline API documentation
 * param Conf is a pointer to the key blob generation configuration struct
 * param OutputBuffer is a pointer to the output buffer, where the key blob is placed, must be 64-bit aligned
 * param OutputBufferSize is the size of the output buffer
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail,
 * kStatus_InvalidArgument if invalid argument
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_GenerateKeyBlob(
    S3MU_Type *mu, uint32_t KeyID, generate_key_blob_input_t *Conf, uint8_t *OutputBuffer, uint16_t OutputBufferSize)
{
    status_t status                               = kStatus_Fail;
    uint32_t tmsg[GENERATE_KEY_BLOB_SIZE]         = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]                  = {0u};
    SDK_ALIGN(uint32_t payload_internal[22u], 8u) = {0u};

    /* Copy the blob input header template */
    (void)memcpy(payload_internal, key_blob_header_template, sizeof(key_blob_header_template));

    /* Set payload length */
    const uint32_t payload_size =
        (Conf->blob_type == kBlob_Type_IEE) ? KEY_BLOB_IEE_PAYLOAD_LENGTH : (uint32_t)Conf->size;
    const uint32_t payload_length_with_hdr = KEY_BLOB_PAYLOAD_HDR_LENGTH + payload_size;

    payload_internal[0] |= ((uint32_t)payload_length_with_hdr << SHIFT_8) & KEY_BLOB_PAYLOAD_LENGTH_MASK;

    /* Set algorithm, size, blob type */
    payload_internal[1] |= ((uint32_t)Conf->algorithm << SHIFT_16) & KEY_BLOB_PAYLOAD_ALGORITHM_MASK;
    payload_internal[1] |= ((uint32_t)Conf->size << SHIFT_8) & KEY_BLOB_PAYLOAD_SIZE_MASK;
    payload_internal[1] |= ((uint32_t)Conf->blob_type) & KEY_BLOB_PAYLOAD_TYPE_MASK;

    /* Form the key blob input payload */
    switch (Conf->blob_type)
    {
        case kBlob_Type_DEK:
            status = construct_dek_key_blob_input(Conf, &payload_internal[2]);
            break;
#if defined(FSL_FEATURE_SOC_OTFAD_COUNT) && (FSL_FEATURE_SOC_OTFAD_COUNT > 0)
        case kBlob_Type_OTFAD:
            status = construct_otfad_key_blob_input(Conf, &payload_internal[2]);
            break;
#endif
#if defined(FSL_FEATURE_SOC_IEE_COUNT) && (FSL_FEATURE_SOC_IEE_COUNT > 0)
        case kBlob_Type_IEE:
            status = construct_iee_key_blob_input(Conf, &payload_internal[2]);
            break;
#endif
        default:
            status = kStatus_Fail;
            break;
    }
    if (status != kStatus_Success)
    {
        (void)memset(payload_internal, 0, sizeof(payload_internal));
        return status;
    }

    /****************** Generate key blob message ***********************/
    tmsg[0] = GENERATE_KEY_BLOB;                                     // Generate Key Blob Command Header
    tmsg[1] = KeyID;                                                 // Key ID or OTFAD settings
    tmsg[2] = 0u;                                                    // Input address extension - unused
    tmsg[3] = ADD_OFFSET((uint32_t)payload_internal);                // Input
    tmsg[4] = 0u;                                                    // Output address extension - unused
    tmsg[5] = ADD_OFFSET((uint32_t)OutputBuffer);                    // Output
    tmsg[6] = ((uint32_t)Conf->iee.rand_keys << SHIFT_8) |
              (uint32_t)OutputBufferSize;                            // Random keys flag | Output buffer size
    tmsg[7] = S3MU_ComputeMsgCrc(tmsg, GENERATE_KEY_BLOB_SIZE - 1u); // CRC

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, GENERATE_KEY_BLOB_SIZE);
    if (status != kStatus_Success)
    {
        (void)memset(payload_internal, 0, sizeof(payload_internal));
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);

    /* Clear the payload to not leave key material in memory */
    (void)memset(payload_internal, 0, sizeof(payload_internal));

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == GENERATE_KEY_BLOB_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
#if defined(ELE_CACHE_HANDLING) && (ELE_CACHE_HANDLING > 0u)
        DCACHE_InvalidateByRange((uint32_t)OutputBuffer, OutputBufferSize);
#endif /* defined(ELE_CACHE_HANDLING) */
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Import keys from key blob to corresponding HW block - currently only IEE supported
 *
 * param mu MU peripheral base address
 * param KeyID identical to the one used for ELE_GenerateKeyBlob()
 * param BlobAddress pointer to the blob generated with ELE_GenerateKeyBlob()
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail,
 * kStatus_InvalidArgument if invalid argument
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_LoadKeyBlob(S3MU_Type *mu, uint32_t KeyID, uint8_t *BlobAddress)
{
    status_t status                   = kStatus_Fail;
    uint32_t tmsg[LOAD_KEY_BLOB_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]      = {0u};

    /****************** Load key blob message ***********************/
    tmsg[0] = LOAD_KEY_BLOB;                     // Load Key Blob Command Header
    tmsg[1] = KeyID;                             // Key ID
    tmsg[2] = 0u;                                // Address extension - unused
    tmsg[3] = ADD_OFFSET((uint32_t)BlobAddress); // Blob address

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, LOAD_KEY_BLOB_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == LOAD_KEY_BLOB_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}


#endif /* !FSL_FEATURE_ELE_IS_S100 */
