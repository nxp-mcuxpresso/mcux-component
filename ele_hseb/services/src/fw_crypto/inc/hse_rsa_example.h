/*
 * @file hse_rsa_example.h
 * @brief Header file for RSA examples in crypto services
 * @details Contains the API's for demonstrating the use of RSA crypto service in demo APP
 */
/*--------------------------------------------------------------------------*/
/* Copyright 2022 NXP                                                       */
/*                                                                          */
/* NXP Proprietary. This software is owned or controlled by NXP and may     */
/* only be used strictly in accordance with the applicable license terms.   */
/* By expressly accepting such terms or by downloading, installing,         */
/* activating and/or otherwise using the software, you are agreeing that    */
/* you have read, and that you agree to comply with and are bound by, such  */
/* license terms. If you do not agree to be bound by the applicable license */
/* terms, then you may not retain, install, activate or otherwise use the   */
/* software.                                                                */
/*--------------------------------------------------------------------------*/
#ifndef SERVICES_INC_HSE_RSA_EXAMPLE_H_
#define SERVICES_INC_HSE_RSA_EXAMPLE_H_

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
#include "global_defs.h"
#include "hse_host_rsa_cipher.h"
#include "fsl_common.h"

/*==================================================================================================
                                     ENUMS
==================================================================================================*/
/*
 * brief RSA Algo execution status
 */
typedef enum
{
    RSA_EXECUTED_NONE = 0,
    RSA_NOPADD_EXAMPLE_SUCCESSFUL = 1,
    RSA_OAEP_EXAMPLE_SUCCESSFUL = 2,
    RSA_PKCS1V15_EXAMPLE_SUCCESSFUL = 4,
    RSA_ALL_EXAMPLE_SUCCESSFUL =  8
} RsaExampleStatus;

/*
 * brief RSA Algo execution status
 */
typedef enum
{
    RSA_STARTED_NONE = 0,
    RSA_NOPADD_EXAMPLE_STARTED = 1,
    RSA_OAEP_EXAMPLE_STARTED = 2,
    RSA_PKCS1V15_EXAMPLE_STARTED = 4,
    RSA_ALL_EXAMPLE_STARTED =  8
} RsaExampleStarted;


/*
 * ============================================================================
 *                               FUNCTIONS DECLARATION
 * ============================================================================
*/

hseSrvResponse_t HSE_RSA_Examples(void);
#ifdef __cplusplus
}
#endif
#endif /* SERVICES_INC_HSE_RSA_EXAMPLE_H_ */
