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
*
*   License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025
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
