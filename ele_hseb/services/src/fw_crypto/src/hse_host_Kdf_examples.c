/**
*   @file    hse_host_Kdf_examples.c
*
*   @brief   Example of KDF cryptographic services.
*   @details Example of sending crypto services requests to HSE over MU (sym/asym; sync/async).
*
*   @addtogroup hse_crypto
*   @{
*/
/*==================================================================================================
*
*   Copyright 2022 NXP.
*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/

#ifdef __cplusplus
extern "C"{
#endif

/*=============================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
=============================================================================*/
#include "hse_host_Kdf_examples.h"
#include "hse_keys_allocator.h"
#include "global_defs.h"
#include "hse_host_import_key.h"

/*=============================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
=============================================================================*/

/*=============================================================================
*                          LOCAL MACROS
=============================================================================*/

/*=============================================================================
*                          LOCAL CONSTANTS
=============================================================================*/
#ifdef HSE_SPT_HKDF
static const uint8_t HKDF_PRK[] = {0xda, 0x8c, 0x8a, 0x73, 0xc7, 0xfa, 0x77, 0x28, 0x8e, 0xc6, 0xf5, 0xe7, 0xc2, 0x97, 0x78, 0x6a, 0xa0, 0xd3, 0x2d, 0x01};
static const uint8_t HKDF_derivedKey[] = 
{
  0x10, 0x85, 0xA4, 0xBD, 0x95, 0xCD, 0x61, 0x19, 0x85, 0xCD, 0x6C, 0xCD, 0x5F, 0x6B, 0xE4, 0xF3, 0x17, 0xB3, 0x96, 0xF6, 0x86, 
  0x1B, 0x47, 0x7C, 0x69, 0x78, 0x43, 0xAE, 0x62, 0x42, 0xB5, 0xDE, 0x8A, 0x11, 0x3F, 0x4A, 0x53, 0x55, 0x15, 0x92, 0x74, 0x05
};
#endif

#ifdef HSE_SPT_PBKDF2
static const uint8_t Pbkdf2_SrcPwd[] = "sourceSecretPasswordToBeUsedWithPbkdf2Algo";
static const uint8_t PBKDF2_Salt[] = "saltToBeUsedWithPbkdf2Algo";
static const uint8_t PBKDF2_derived_arr[] = 
{
  0x8a, 0x9d, 0xf4, 0x9a, 0xad, 0x2d, 0x94, 0xdd, 0x14, 0xbe, 0xa9, 0x16, 0x92, 0xdd, 0x64, 0x25, 
  0x74, 0xa1, 0x73,  0xe5, 0x82, 0x5a, 0x6c, 0xdb, 0xe1, 0x5d, 0x95, 0xbf, 0xf7, 0x69, 0xb8, 0x2d
}; /* "8a9df49aad2d94dd14bea91692dd642574a173e5825a6cdbe15d95bff769b82d" */
                                          
#endif

#ifdef HSE_SPT_KDF_TLS12_PRF
static const uint8_t pre_master_secret[] = 
{
  0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
  0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A
};
static const uint8_t TLS12_derived_Aes192Key[] = 
{
  0x55, 0xA8, 0x23, 0xF5, 0x95, 0x08, 0x29, 0x0B, 0x2A, 0x8F, 0x89, 0x55,
  0xE8, 0xA3, 0xE9, 0x3D, 0xA8, 0x71, 0x9E, 0xCC, 0xA8, 0xB7, 0x05, 0x43
};

static const uint8_t iv_block_ref[32] = 
{   
  0xBA, 0x0D, 0x04, 0x97, 0x4F, 0x52, 0x5D, 0x09, 0x49, 0x54, 0x7E, 0x27, 0x5C, 0xC0, 0xAB, 0xB3, 
  0xCB, 0x72, 0x87, 0x65, 0xF2, 0x7A, 0x7F, 0x35, 0xED, 0xD6, 0xC2, 0xB9, 0x9C, 0x67, 0x6A, 0xB2
};

static const uint8_t client_verify_data_ref[] = { 0xEA, 0xA3, 0xCA, 0x1, 0x59, 0x22, 0xD6, 0x65, 0x67, 0x8, 0x85, 0xD4};
static const uint8_t server_verify_data_ref[] = { 0x46, 0x2, 0x39, 0x64, 0x8C, 0x39, 0x1E, 0xE1, 0x4, 0x5F, 0x5D, 0xF6};    

/*static char label_extended_master_secret[] = { 'e', 'x', 't', 'e', 'n', 'd', 'e', 'd', ' ', 'm', 'a', 's', 't', 'e', 'r', ' ', 's', 'e', 'c', 'r', 'e', 't' };
static const uint8_t session_hash[] = 
{ 
  0x0A, 0x2D, 0xCA, 0x6B, 0xC3, 0xA8, 0x57, 0x39, 0xCC, 0x4D, 0x26, 0x92, 0xE5, 0x6F, 0x18, 0x2B,
  0x9D, 0x56, 0x02, 0xE6, 0x8D, 0x04, 0xC4, 0x07, 0x7B, 0x66, 0x4E, 0x6F, 0x95, 0xB6, 0xFE, 0x64
}; 
*/ 

static char label_master_secret[]  = { 'm', 'a', 's', 't', 'e', 'r', ' ', 's', 'e', 'c', 'r', 'e', 't' };
static const uint8_t ClientServerHelloRandom[] = 
{
  0x61 ,0x78 ,0x05 ,0x4E ,0x7D ,0x2E ,0x8F ,0xA9 ,0xAD ,0x62 ,0x61 ,0x00 ,0xA6 ,0x20 ,0x2D , 0x55, 
  0x9A ,0x0D ,0x20 ,0x81 ,0x0E ,0xB5 ,0xAB ,0x4D ,0xB0 ,0x60 ,0x0F ,0x16 ,0x6E ,0x95 , 0x2C ,0x8B, 
  0x60 ,0xDD ,0xAE ,0x40 ,0x89 ,0xF9 ,0xC3 ,0xC3 ,0x73 ,0x20 ,0x28 ,0x00 ,0x95 ,0x5A ,0xA0 ,0x43, 
  0x39 ,0x57 ,0xAA ,0x75 ,0xA6 ,0xEF ,0x4C ,0x77 ,0x87 ,0xBD ,0x49 ,0x0E ,0x2B ,0x93 ,0x7E ,0xF0 
};  

static char label_key_expansion[] = { 'k', 'e', 'y', ' ', 'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n' };
/*To store the IV data generated during key expansion */
static uint8_t iv_block[32] = { 0 };
static const uint8_t ServerClientRandom[] = 
{
  0xae, 0x6c, 0x80, 0x6f, 0x8a, 0xd4, 0xd8, 0x07, 0x84, 0x54, 0x9d, 0xff, 0x28, 0xa4, 0xb5, 0x8f,
  0xd8, 0x37, 0x68, 0x1a, 0x51, 0xd9, 0x28, 0xc3, 0xe3, 0x0e, 0xe5, 0xff, 0x14, 0xf3, 0x98, 0x68,
  0x62, 0xe1, 0xfd, 0x91, 0xf2, 0x3f, 0x55, 0x8a, 0x60, 0x5f, 0x28, 0x47, 0x8c, 0x58, 0xcf, 0x72,
  0x63, 0x7b, 0x89, 0x78, 0x4d, 0x95, 0x9d, 0xf7, 0xe9, 0x46, 0xd3, 0xf0, 0x7b, 0xd1, 0xb6, 0x16,
};

static char label_client_finished[] = { 'c', 'l', 'i', 'e', 'n', 't', ' ', 'f', 'i', 'n', 'i', 's', 'h', 'e', 'd' };
/* Computed in the "client finished" step */
static uint8_t client_verify_data[12] = { 0 };

static char label_server_finished[] = { 's', 'e', 'r', 'v', 'e', 'r', ' ', 'f', 'i', 'n', 'i', 's', 'h', 'e', 'd' };
/* Computed in the "server finished" step */
static uint8_t server_verify_data[12] = { 0 };

/* Hash of handshake messages, since we only focus on the key-derivation part of TLS 1.2 in this example, a mockup value is provided */
static const uint8_t mockup_handshake_messages_hash[] = 
{
  0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
  0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
  0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
  0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
};
#endif
/*=============================================================================
*                         LOCAL VARIABLES
=============================================================================*/

/*=============================================================================
*                         GLOBAL CONSTANTS
=============================================================================*/

/*=============================================================================
*                         GLOBAL VARIABLES
=============================================================================*/
#ifdef HSE_SPT_KEY_DERIVE
  /* Will be used for comparing with derived key informations as it should be non-zero */
  const hseKeyInfo_t extractedKeyInfo = { 0U };
#endif

#ifdef HSE_SPT_HKDF
uint8_t HKDF_pinfo_buffer[32];
/* HKDF scheme details */
hseHKDF_ExpandScheme_t hseHKDFScheme = 
{
  .kdfCommon = 
  {
    .srcKeyHandle = HSE_INVALID_KEY_HANDLE,
    .targetKeyHandle = HSE_INVALID_KEY_HANDLE,
    .keyMatLen = ARRAY_SIZE(HKDF_derivedKey),
    .kdfPrf = HSE_KDF_PRF_HMAC,
    .prfAlgo.hmacHash = HSE_KDF_SHA2_224,
    .infoLength = 0,
    .pInfo = (HOST_ADDR)HKDF_pinfo_buffer
  },
};
hseKeyInfo_t HKDF_EXPAND_INFO = 
{
  .keyType = HSE_KEY_TYPE_HMAC,
  .keyBitLen = ARRAY_SIZE(HKDF_PRK) * 8U,
  .keyFlags = (HSE_KF_USAGE_DERIVE | HSE_KF_ACCESS_EXPORTABLE)
};
hseKey_t HKDF_EXPAND_PRK = 
{
  .keyHandle = HSE_INVALID_KEY_HANDLE,
  .pKeyInfo = &HKDF_EXPAND_INFO,
  .keyValue = 
  {
    .pKey2 = (uint8_t *)HKDF_PRK,
    .keyLen2 = ARRAY_SIZE(HKDF_PRK),
  }
};
hseKeyImportParams_t HKDF_Expand_KeyImportParams = 
{ 
  .pKey = &HKDF_EXPAND_PRK 
};
#endif

#ifdef HSE_SPT_PBKDF2
/* PBKDF2 scheme details */
hsePBKDF2Scheme_t Pbkdf2_Scheme = 
{
  .srcKeyHandle = HSE_INVALID_KEY_HANDLE,
  .targetKeyHandle = HSE_INVALID_KEY_HANDLE,
  .keyMatLen = ARRAY_SIZE(PBKDF2_derived_arr),
  .hmacHash = HSE_KDF_SHA2_256,
  .iterations = 1000,
  .saltLength = ARRAY_SIZE(PBKDF2_Salt) - 1,
  .pSalt = (HOST_ADDR)PBKDF2_Salt,
};
hseKeyInfo_t PBKDF2_SECRET_PWD_INFO = 
{
  .keyType = HSE_KEY_TYPE_HMAC,
  .keyBitLen = (ARRAY_SIZE(Pbkdf2_SrcPwd) - 1) * 8U,
  .keyFlags = (HSE_KF_USAGE_DERIVE | HSE_KF_ACCESS_EXPORTABLE)
};
hseKey_t PBKDF2_SECRET_PWD = 
{
  .keyHandle = HSE_INVALID_KEY_HANDLE,
  .pKeyInfo = &PBKDF2_SECRET_PWD_INFO,
  .keyValue = 
  {
    .pKey2 = (uint8_t *)Pbkdf2_SrcPwd,
    .keyLen2 = ARRAY_SIZE(Pbkdf2_SrcPwd) - 1,
  },
};
hseKeyImportParams_t Pbkdf2_ImportSecretPwd = 
{
  .pKey = &PBKDF2_SECRET_PWD
};
#endif

#ifdef HSE_SPT_KDF_TLS12_PRF
/* TLS1.2 scheme details for server finish */
hseKdfTLS12PrfScheme_t hse_TLS12_PRF_ServerFinish = 
{
  .srcKeyHandle    = HSE_INVALID_KEY_HANDLE,
  .hmacHash        = HSE_KDF_SHA2_256,
  .labelLength     = ARRAY_SIZE(label_server_finished),
  .pLabel          = (HOST_ADDR)label_server_finished,
  .seedLength      = ARRAY_SIZE(mockup_handshake_messages_hash),
  .pSeed           = (HOST_ADDR)mockup_handshake_messages_hash,
  .targetKeyHandle = 0,
  .keyMatLength    = 0,
  .outputLength    = 12,
  .pOutput         = (HOST_ADDR)server_verify_data
};
/* TLS1.2 scheme details for client finish */
hseKdfTLS12PrfScheme_t hse_TLS12_PRF_ClientFinish = 
{
  .srcKeyHandle    = HSE_INVALID_KEY_HANDLE,
  .hmacHash        = HSE_KDF_SHA2_256,
  .labelLength     = ARRAY_SIZE(label_client_finished),
  .pLabel          = (HOST_ADDR)label_client_finished,
  .seedLength      = ARRAY_SIZE(mockup_handshake_messages_hash),
  .pSeed           = (HOST_ADDR)mockup_handshake_messages_hash,
  .targetKeyHandle = 0,
  .keyMatLength    = 0,
  .outputLength    = 12,
  .pOutput         = (HOST_ADDR)client_verify_data
};
/* TLS1.2 scheme details for key expantion */
hseKdfTLS12PrfScheme_t hse_TLS12_PRF_KeyExpansion = 
{
  .srcKeyHandle    = HSE_INVALID_KEY_HANDLE,
  .hmacHash        = HSE_KDF_SHA2_256,
  .labelLength     = ARRAY_SIZE(label_key_expansion),
  .pLabel          = (HOST_ADDR)label_key_expansion,
  .seedLength      = ARRAY_SIZE(ServerClientRandom),
  .pSeed           = (HOST_ADDR)ServerClientRandom,
  .targetKeyHandle = HSE_INVALID_KEY_HANDLE,
  .keyMatLength    = 96,
  .outputLength    = 32,
  .pOutput         = (HOST_ADDR)iv_block
};
/* TLS1.2 scheme details for master secret */
hseKdfTLS12PrfScheme_t hse_TLS12_PRF_ComputMasterSecret = 
{
  .srcKeyHandle    = HSE_INVALID_KEY_HANDLE,
  .hmacHash        = HSE_KDF_SHA2_256,
  .labelLength     = ARRAY_SIZE(label_master_secret),
  .pLabel          = (HOST_ADDR)label_master_secret,
  .seedLength      = ARRAY_SIZE(ClientServerHelloRandom),
  .pSeed           = (HOST_ADDR)ClientServerHelloRandom,
  .targetKeyHandle = HSE_INVALID_KEY_HANDLE,
  .keyMatLength    = 48,
  .outputLength    = 0,
  .pOutput         = (HOST_ADDR)NULL
};
/* TLS1.2 scheme details for extended master secret */
/*hseKdfTLS12PrfScheme_t hse_TLS12_PRF_ExtendedMasterSecret = 
{
  .srcKeyHandle    = HSE_INVALID_KEY_HANDLE,
  .hmacHash        = HSE_KDF_SHA2_256,
  .labelLength     = ARRAY_SIZE(label_extended_master_secret),
  .pLabel          = (HOST_ADDR)label_extended_master_secret,
  .seedLength      = ARRAY_SIZE(session_hash),
  .pSeed           = (HOST_ADDR)session_hash,
  .targetKeyHandle = HSE_INVALID_KEY_HANDLE,
  .keyMatLength    = 48,
  .outputLength    = 0,
  .pOutput         = (HOST_ADDR)NULL
};*/

hseKeyInfo_t TLS12_PRE_MASTER_PASSWORD_INFO = 
{
  .keyType   = HSE_KEY_TYPE_HMAC,
  .keyBitLen = ARRAY_SIZE(pre_master_secret) * 8U,
  .keyFlags  = (HSE_KF_USAGE_DERIVE | HSE_KF_ACCESS_EXPORTABLE)
};

hseKey_t TLS12_PRE_MASTER_PASSWORD = 
{
  .keyHandle = HSE_INVALID_KEY_HANDLE,
  .pKeyInfo  = &TLS12_PRE_MASTER_PASSWORD_INFO,
  .keyValue  = 
  {
    .pKey2   = (uint8_t *)pre_master_secret,
    .keyLen2 = ARRAY_SIZE(pre_master_secret),
  }
};
hseKeyImportParams_t TLS12_PRF_Import_PreMasterSecret = 
{
  .pKey = &TLS12_PRE_MASTER_PASSWORD
};
#endif
/*=============================================================================
*                         LOCAL FUNCTION PROTOTYPES
=============================================================================*/
#ifdef HSE_SPT_HKDF
static hseSrvResponse_t HSE_HKDF_Test
(
  hseKeyImportParams_t *pImportSecretParams,
  hseHKDF_ExpandScheme_t *pKdfScheme
);
#endif

#ifdef HSE_SPT_PBKDF2
static hseSrvResponse_t HSE_PBKDF2_Test
(
  hseKeyImportParams_t *pImportSecretParams,
  hsePBKDF2Scheme_t *pKdfScheme
);
#endif

#ifdef HSE_SPT_KDF_TLS12_PRF
static hseSrvResponse_t HSE_TLS12_PRF_Test
(
  hseKeyImportParams_t *pImportSecretParams,
  hseKdfTLS12PrfScheme_t *pKdfScheme
);
#endif
/*=============================================================================
*                         LOCAL FUNCTIONS
=============================================================================*/
#ifdef HSE_SPT_HKDF
static hseSrvResponse_t HSE_HKDF_Test
(
  hseKeyImportParams_t *pImportSecretParams,
  hseHKDF_ExpandScheme_t *pKdfScheme
)
{
  hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;
  uint16_t derivedHmacKeyBlockLenth = 42UL;
  uint8_t derivedHmacKeyBlock[42UL];
  hseKeyInfo_t extractedHmacKeyInfo;
  hseKeyHandle_t extractedHmacKeyHandle = HSE_INVALID_KEY_HANDLE;

  pImportSecretParams->authParams.pKeyHandle = NULL;
  pImportSecretParams->cipherParams.pKeyHandle = NULL;
  /* Import source key */
  hseResponse = ImportKeyReq(pImportSecretParams);
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  hseResponse = HSE_HKDFReq(pKdfScheme);
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  /* 
    Extract key from derived key and compare with expected derive key(derivedHmacKeyBlock/HKDF_derivedKey) to verify 
  */
  /* Declare the information about the 192 bits AES key to be extracted */
  memset(&extractedHmacKeyInfo, 0x00U, sizeof(hseKeyInfo_t));
  /* Hmac key type */
  extractedHmacKeyInfo.keyType = HSE_KEY_TYPE_HMAC;
  extractedHmacKeyInfo.keyBitLen = pKdfScheme->kdfCommon.keyMatLen * 8U;
  /* Usage flags for this key - Encrypt/Decrypt/Sign/Verify */
  extractedHmacKeyInfo.keyFlags = 
  (
    HSE_KF_USAGE_SIGN     |
    HSE_KF_USAGE_VERIFY   |
    HSE_KF_ACCESS_EXPORTABLE
  );
  /* Extract the 256 bits AES key from the remaining derived key material */
  hseResponse = HSEKeyDeriveExtractKeyReq
  (
    pKdfScheme->kdfCommon.targetKeyHandle,
    0U,
    /* we can perform any crypto operation with this Hmac key handle */
    &extractedHmacKeyHandle,
    RAM_KEY,
    extractedHmacKeyInfo
  );
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }    
  hseResponse = ExportPlainSymKeyReq
  (
    extractedHmacKeyHandle, 
    &extractedHmacKeyInfo, 
    derivedHmacKeyBlock,
    &derivedHmacKeyBlockLenth
  );
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  } 
  /* 
    Verify if the key is HMAC, the length and the flags.
    Verify if the derived value is equal to the expected value. 
  */
  if( (extractedHmacKeyInfo.keyType   == extractedKeyInfo.keyType)   ||
      (extractedHmacKeyInfo.keyBitLen == extractedKeyInfo.keyBitLen) ||
      (extractedHmacKeyInfo.keyFlags  == extractedKeyInfo.keyFlags)  ||
      (extractedHmacKeyHandle == HSE_INVALID_KEY_HANDLE)             ||
      (0 != memcmp(derivedHmacKeyBlock, HKDF_derivedKey, derivedHmacKeyBlockLenth)))
  {
    hseResponse = HSE_SRV_RSP_VERIFY_FAILED;
    goto exit;
  }
  exit:
    HKF_FreeKeySlot(&extractedHmacKeyHandle);
    return hseResponse;
}
hseSrvResponse_t HSE_HKDFReq_Example(void)
{
  hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;
  hseKeyHandle_t kdfSrcKey = HSE_INVALID_KEY_HANDLE; 
  hseKeyHandle_t kdfTargetSharedSecretKey = HSE_INVALID_KEY_HANDLE;
  /* Allocate key slot for source key */
  HKF_AllocKeySlot
  (
    RAM_KEY, 
    HKDF_Expand_KeyImportParams.pKey->pKeyInfo->keyType,
    HKDF_Expand_KeyImportParams.pKey->pKeyInfo->keyBitLen, 
    &kdfSrcKey
  );
  /* Allocate key slot for target key */
  HKF_AllocKeySlot
  (
    RAM_KEY, 
    HSE_KEY_TYPE_SHARED_SECRET, 
    hseHKDFScheme.kdfCommon.keyMatLen * 8U, 
    &kdfTargetSharedSecretKey
  );

  HKDF_Expand_KeyImportParams.pKey->keyHandle = kdfSrcKey;
  hseHKDFScheme.kdfCommon.srcKeyHandle = kdfSrcKey;
  hseHKDFScheme.kdfCommon.targetKeyHandle = kdfTargetSharedSecretKey;
  hseResponse = HSE_HKDF_Test
  (
    &HKDF_Expand_KeyImportParams, 
    &hseHKDFScheme
  );
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  exit:
    HKF_FreeKeySlot(&kdfSrcKey);
    HKF_FreeKeySlot(&kdfTargetSharedSecretKey);
    return hseResponse;
}
#endif

#ifdef HSE_SPT_PBKDF2
static hseSrvResponse_t HSE_PBKDF2_Test
(
  hseKeyImportParams_t *pImportSecretParams,
  hsePBKDF2Scheme_t *pKdfScheme
)
{
  hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;
  hseKeyHandle_t aesDerivedKeyHandle = HSE_INVALID_KEY_HANDLE;
  /* 192/8 = 24 byte of derived key since we are extracting AES 192 bit key */
  uint16_t derivedAes192KeyBlockLenth = 24UL;
  uint8_t derivedAes192KeyBlock[24UL];
  hseKeyInfo_t aes192KeyInfo;

  pImportSecretParams->authParams.pKeyHandle = NULL;
  pImportSecretParams->cipherParams.pKeyHandle = NULL;
  /* Import source password */
  hseResponse = ImportKeyReq(pImportSecretParams);
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }

  hseResponse = HSE_PBKDF2Req(pKdfScheme);
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  /*
    Extract key from derived key and compare with expected derive key(expectedKeyBlock/PBKDF2_derived) to verify 
  */
  /* Declare the information about the 192 bits AES key to be extracted */
  memset(&aes192KeyInfo, 0x00U, sizeof(hseKeyInfo_t));
  /* AES key type */
  aes192KeyInfo.keyType = HSE_KEY_TYPE_AES;
  /* Usage flags for this key - Encrypt/Decrypt/Sign/Verify */
  aes192KeyInfo.keyFlags = 
  (
    HSE_KF_USAGE_ENCRYPT |
    HSE_KF_USAGE_DECRYPT |
    HSE_KF_USAGE_SIGN    |
    HSE_KF_USAGE_VERIFY  |
    HSE_KF_ACCESS_EXPORTABLE
  );
  /* AES 192 bit */
  aes192KeyInfo.keyBitLen = 192U;
  /* Extract the 192 bits AES key from the remaining derived key material */
  hseResponse = HSEKeyDeriveExtractKeyReq
  (
    pKdfScheme->targetKeyHandle,
    0U,
    /* we can perform any crypto operation with this AES192 key handle */
    &aesDerivedKeyHandle,
    RAM_KEY,
    aes192KeyInfo
  );
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
            
  hseResponse = ExportPlainSymKeyReq
  (
    aesDerivedKeyHandle, 
    &aes192KeyInfo, 
    derivedAes192KeyBlock,
    &derivedAes192KeyBlockLenth
  );
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  /* 
    Verify if the key is AES, the length and the flags.
    Verify if the derived value is equal to the expected value. 
  */
  if( (aes192KeyInfo.keyType   == extractedKeyInfo.keyType)   ||
      (aes192KeyInfo.keyBitLen == extractedKeyInfo.keyBitLen) ||
      (aes192KeyInfo.keyFlags  == extractedKeyInfo.keyFlags)  ||
      (aesDerivedKeyHandle == HSE_INVALID_KEY_HANDLE)         ||
      (0 != memcmp(derivedAes192KeyBlock, PBKDF2_derived_arr, derivedAes192KeyBlockLenth)))
  {
    hseResponse = HSE_SRV_RSP_VERIFY_FAILED;
    goto exit;
  }
  exit:
    HKF_FreeKeySlot(&aesDerivedKeyHandle);
    return hseResponse;
}

hseSrvResponse_t HSE_PBKDF2Req_Example(void)
{
  hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;
  hseKeyHandle_t kdfSrcKey = HSE_INVALID_KEY_HANDLE; 
  hseKeyHandle_t kdfTargetSharedSecretKey = HSE_INVALID_KEY_HANDLE;
  /* Allocate key slot for source key */
  HKF_AllocKeySlot
  (
    RAM_KEY, 
    Pbkdf2_ImportSecretPwd.pKey->pKeyInfo->keyType,
    Pbkdf2_ImportSecretPwd.pKey->pKeyInfo->keyBitLen, 
    &kdfSrcKey
  );
  /* Allocate key slot for target key */
  HKF_AllocKeySlot
  (
    RAM_KEY, 
    HSE_KEY_TYPE_SHARED_SECRET, 
    Pbkdf2_Scheme.keyMatLen * 8U, 
    &kdfTargetSharedSecretKey
  );

  Pbkdf2_ImportSecretPwd.pKey->keyHandle = kdfSrcKey;
  Pbkdf2_Scheme.srcKeyHandle = kdfSrcKey;
  Pbkdf2_Scheme.targetKeyHandle = kdfTargetSharedSecretKey;
  hseResponse = HSE_PBKDF2_Test
  (
    &Pbkdf2_ImportSecretPwd, 
    &Pbkdf2_Scheme
  );
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  exit:
    HKF_FreeKeySlot(&kdfSrcKey);
    HKF_FreeKeySlot(&kdfTargetSharedSecretKey);
    return hseResponse;
}
#endif

#ifdef HSE_SPT_KDF_TLS12_PRF
static hseSrvResponse_t HSE_TLS12_PRF_Test
(
  hseKeyImportParams_t *pImportSecretParams,
  hseKdfTLS12PrfScheme_t *pKdfScheme
)
{
  hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;
  hseKeyHandle_t aesDerivedKeyHandle = HSE_INVALID_KEY_HANDLE;
  /* 192/8 = 24 byte of derived key since we are extracting AES 192 bit key */
  uint16_t derivedAes192KeyBlockLenth = 24UL;
  uint8_t derivedAes192KeyBlock[24UL];
  hseKeyInfo_t aes192KeyInfo;
  hseKeyHandle_t prfKeyExpansionKeyHandle = HSE_INVALID_KEY_HANDLE;

  pImportSecretParams->authParams.pKeyHandle = NULL;
  pImportSecretParams->cipherParams.pKeyHandle = NULL;
  /* Import pre-master secret */
  hseResponse = ImportKeyReq(pImportSecretParams);
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  hseResponse = HSE_TLS12_PRFReq(pKdfScheme);
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  /* 
    Generating the 96 bytes of Keys and 32 bytes of IV data. IV data will be copied in iv_block buffer using master key 
  */
  /* Allocate key slot for prfKeyExpansionKeyHandle */
  HKF_AllocKeySlot
  (
    RAM_KEY, 
    HSE_KEY_TYPE_SHARED_SECRET, 
    hse_TLS12_PRF_KeyExpansion.keyMatLength * 8U, 
    &prfKeyExpansionKeyHandle
  );

  hse_TLS12_PRF_KeyExpansion.srcKeyHandle    = pKdfScheme->targetKeyHandle;
  hse_TLS12_PRF_KeyExpansion.targetKeyHandle = prfKeyExpansionKeyHandle;
  hseResponse = HSE_TLS12_PRFReq(&hse_TLS12_PRF_KeyExpansion);
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  if(0 != memcmp(iv_block, iv_block_ref, sizeof(iv_block_ref)))
  {
    /* result is wrong */
    hseResponse = HSE_SRV_RSP_VERIFY_FAILED;
    goto exit;
  }
  /* 
    Extract key from derived key and compare with expected derive key to verify 
  */
  /* Declare the information about the 192 bits AES key to be extracted */
  memset(&aes192KeyInfo, 0x00U, sizeof(hseKeyInfo_t));
  /* AES key type */
  aes192KeyInfo.keyType = HSE_KEY_TYPE_AES;
  /* Usage flags for this key - Encrypt/Decrypt/Sign/Verify */
  aes192KeyInfo.keyFlags = 
  (
    HSE_KF_USAGE_ENCRYPT |
    HSE_KF_USAGE_DECRYPT |
    HSE_KF_USAGE_SIGN    |
    HSE_KF_USAGE_VERIFY  |
    HSE_KF_ACCESS_EXPORTABLE 
  );
  /* AES 192 bit */
  aes192KeyInfo.keyBitLen = 192U;
  /* Extract the 192 bits AES key from the remaining derived key material */
  hseResponse = HSEKeyDeriveExtractKeyReq
  (
    hse_TLS12_PRF_KeyExpansion.targetKeyHandle,
    0U,
    /* we can perform any crypto operation with this AES192 key handle */
    &aesDerivedKeyHandle,
    RAM_KEY,
    aes192KeyInfo
  );
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }  
  hseResponse = ExportPlainSymKeyReq
  (
    aesDerivedKeyHandle, 
    &aes192KeyInfo, 
    derivedAes192KeyBlock,
    &derivedAes192KeyBlockLenth
  );
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  /* 
    Verify if the key is AES, the length and the flags.
    Verify if the derived value is equal to the expected value. 
  */
  if( (aes192KeyInfo.keyType   == extractedKeyInfo.keyType)   ||
      (aes192KeyInfo.keyBitLen == extractedKeyInfo.keyBitLen) ||
      (aes192KeyInfo.keyFlags  == extractedKeyInfo.keyFlags)  ||
      (aesDerivedKeyHandle == HSE_INVALID_KEY_HANDLE)         ||
      (0 != memcmp(derivedAes192KeyBlock, TLS12_derived_Aes192Key, derivedAes192KeyBlockLenth)))
  {
    hseResponse = HSE_SRV_RSP_VERIFY_FAILED;
    goto exit;
  }
  /* Generating the clinet finish message using master key */
  hse_TLS12_PRF_ClientFinish.srcKeyHandle = pKdfScheme->targetKeyHandle; /* or kdfTargetSharedSecretKey */ 
  hseResponse = HSE_TLS12_PRFReq(&hse_TLS12_PRF_ClientFinish);
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  /* Verification against mockup reference */
  if(0 != memcmp(client_verify_data, client_verify_data_ref, sizeof(client_verify_data_ref)))
  {
    /* result is wrong */
    hseResponse = HSE_SRV_RSP_VERIFY_FAILED;
    goto exit;
  }
  /* Generating the server finish message using master key */
  hse_TLS12_PRF_ServerFinish.srcKeyHandle = pKdfScheme->targetKeyHandle; /* or kdfTargetSharedSecretKey */
  hseResponse = HSE_TLS12_PRFReq(&hse_TLS12_PRF_ServerFinish);
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  if(0 != memcmp(server_verify_data, server_verify_data_ref, sizeof(server_verify_data_ref)))
  {
    /* result is wrong */
    hseResponse = HSE_SRV_RSP_VERIFY_FAILED;
    goto exit;
  }
  exit:
    HKF_FreeKeySlot(&aesDerivedKeyHandle);
    HKF_FreeKeySlot(&prfKeyExpansionKeyHandle);
    return hseResponse;
}
hseSrvResponse_t HSE_TLS12_PRFReq_Example(void)
{
  hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;
  hseKeyHandle_t preMasterSrcKeyHandle = HSE_INVALID_KEY_HANDLE; 
  hseKeyHandle_t MasterSrcKeyHandle = HSE_INVALID_KEY_HANDLE;
  /* Allocate key slot for Pre-Master Secret */
  HKF_AllocKeySlot
  (
    RAM_KEY, 
    TLS12_PRF_Import_PreMasterSecret.pKey->pKeyInfo->keyType,
    TLS12_PRF_Import_PreMasterSecret.pKey->pKeyInfo->keyBitLen, 
    &preMasterSrcKeyHandle
  );
  /* Allocate key slot for Master Secret */
  HKF_AllocKeySlot
  (
    RAM_KEY, 
    HSE_KEY_TYPE_SHARED_SECRET, 
    BYTES_TO_BITS(hse_TLS12_PRF_ComputMasterSecret.keyMatLength),
    &MasterSrcKeyHandle
  );

  TLS12_PRF_Import_PreMasterSecret.pKey->keyHandle = preMasterSrcKeyHandle;
  hse_TLS12_PRF_ComputMasterSecret.srcKeyHandle    = preMasterSrcKeyHandle;
  hse_TLS12_PRF_ComputMasterSecret.targetKeyHandle = MasterSrcKeyHandle;
  hseResponse = HSE_TLS12_PRF_Test
  (
    &TLS12_PRF_Import_PreMasterSecret, 
    &hse_TLS12_PRF_ComputMasterSecret
  );
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  exit:
    HKF_FreeKeySlot(&preMasterSrcKeyHandle);
    HKF_FreeKeySlot(&MasterSrcKeyHandle);
    return hseResponse;
}
#endif
/******************************************************************************
 * Function:    HSE_KDF_Examples
 * Description: Key derivation examples using HSE
 *              AES key import
 *              Key derivation (PBKDF2, HKDF, TLS 1.2),
 *              Key extraction - AES, HMAC keys from derived key material
 *****************************************************************************/
#ifdef HSE_SPT_KEY_DERIVE
hseSrvResponse_t HSE_KDF_Examples(void)
{
  hseSrvResponse_t hseResponse = HSE_SRV_RSP_GENERAL_ERROR;

  #ifdef HSE_SPT_PBKDF2
  hseResponse = HSE_PBKDF2Req_Example();
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  #endif

  #ifdef HSE_SPT_HKDF
  hseResponse = HSE_HKDFReq_Example();
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
  #endif

  #ifdef HSE_SPT_KDF_TLS12_PRF
    #if 0 /* Disable TLS12 on K3 pending investigation*/
  hseResponse = HSE_TLS12_PRFReq_Example();
  if(HSE_SRV_RSP_OK != hseResponse)
  {
    goto exit;
  }
    #endif
  #endif

#if defined(HSE_SPT_PBKDF2) || defined(HSE_SPT_HKDF) || defined(HSE_SPT_KDF_TLS12_PRF)
exit:
#else
  hseResponse = HSE_SRV_RSP_NOT_SUPPORTED;
#endif
  return hseResponse;
}
#endif



#ifdef __cplusplus
}
#endif
/** @} */
