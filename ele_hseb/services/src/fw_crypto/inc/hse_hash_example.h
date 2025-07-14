/*
 * @file hse_header_example.h
 * @brief Header file for HASH examnples in Crypto services
 * @details Contains the API's for demonstarting the use of HASH crypto services in demo APP
 */

/*==================================================================================================
*
*   Copyright-2022 NXP.
*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/

#ifndef SERVICES_INC_HSE_HASH_EXAMPLE_H_
#define SERVICES_INC_HSE_HASH_EXAMPLE_H_

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) Catalog formatting,Host Import keys and Demo App Services header files will be comman in all examples
* 2) Further example specific files will be included
==================================================================================================*/
#include "hse_b_catalog_formatting.h"
#include "hse_host_import_key.h"
#include "hse_demo_app_services.h"
//#include "host_stm.h"
#include "fsl_common.h"
  
/*
 * ============================================================================
 *                                       LOCAL MACROS
 * ============================================================================
 */
#define NUMBER_OF_ASYNC_REQ         (3U)
#define HASH_BUFFER_SIZE                 (512U)

/*=============================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
=============================================================================*/

//Argument for the async hash callback function
typedef struct
{
    uint8_t u8MuInstance;
    uint8_t u8MuChannel;
} hashCallbackParams_t;



hseSrvResponse_t HSE_HashDataBlocking
(   uint8_t u8MuInstance,
    hseAccessMode_t accessMode,
    uint32_t streamId,
    hseHashAlgo_t hashAlgo,
    const uint8_t *pInput,
    uint32_t inputLength,
    uint8_t *pHash,
    uint32_t *pHashLength
);
hseSrvResponse_t HSE_HashDataNonBlocking
(
    uint8_t u8MuInstance,
    hseAccessMode_t accessMode,
    uint32_t streamId,
    hseHashAlgo_t hashAlgo,
    const uint8_t *pInput,
    uint32_t inputLength,
    uint8_t *pHash,
    uint32_t *pHashLength
);

hseSrvResponse_t HSE_HashAsync_Example(void);


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_INC_HSE_HASH_EXAMPLE_H_ */
