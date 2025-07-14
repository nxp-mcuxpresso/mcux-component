/**
*   @file    hse_host_kdf_examples.h
*

*   @brief   KDF service examples.
*   @details This file contains declaration of functions or variables used in KDF service  request to HSE.
*
*   @addtogroup hse_crypto
*   @{
*/
/*==================================================================================================
*
*   Copyright 2022 NXP.*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/
#ifndef HSE_HOST_KDF_EXAMPLES_H
#define HSE_HOST_KDF_EXAMPLES_H

#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) Catalog formatting,Host Import keys and Demo App Services header files will be comman in all examples
* 2) Further example specific files will be included
==================================================================================================*/
#include "hse_host_kdf.h"

/*=============================================================================
                            CONSTANTS
=============================================================================*/

/*=============================================================================
                            DEFINES AND MACROS
=============================================================================*/

/*=============================================================================
                            ENUMS
=============================================================================*/

/*=============================================================================
                            STRUCTURES AND OTHER TYPEDEFS
=============================================================================*/

/*=============================================================================
                            GLOBAL VARIABLE DECLARATIONS
=============================================================================*/

/*=============================================================================
                            FUNCTION PROTOTYPES
=============================================================================*/
#ifdef HSE_SPT_HKDF
hseSrvResponse_t HSE_HKDFReq_Example(void);
#endif

#ifdef HSE_SPT_PBKDF2
hseSrvResponse_t HSE_PBKDF2Req_Example(void);
#endif

#ifdef HSE_SPT_KDF_TLS12_PRF
hseSrvResponse_t HSE_TLS12_PRFReq_Example(void);
#endif

#ifdef HSE_SPT_KEY_DERIVE
hseSrvResponse_t HSE_KDF_Examples(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* HSE_HOST_KDF_EXAMPLES_H */
/** @} */
