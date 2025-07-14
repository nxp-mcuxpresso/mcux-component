/*
 * @file   hse_aead_example.h
 * @brief Header file for AEAD example in crypto services
 * @details Contains the API's for demonstrating the use of AEAD crypto service in DemoApp
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
#ifndef SERVICES_INC_HSE_AEAD_EXAMPLE_H_
#define SERVICES_INC_HSE_AEAD_EXAMPLE_H_

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) Catalog formatting,Host Import keys and Demo App Services header files will be comman in all examples
* 2) Further example specific files will be included
==================================================================================================*/
#include "hse_host_import_key.h"
#include "hse_demo_app_services.h"
#include "hse_host_aead.h"

/*==================================================================================================
                                     ENUMS
==================================================================================================*/
/*
 * brief AEAD algorithms execution status
 */
typedef enum
{
    AEAD_EXECUTED_NONE = 0,
    AEAD_CCM_EXAMPLE_SUCCESSFUL = 1,
    AEAD_GCM_EXAMPLE_SUCCESSFUL = 2,
} AeadExampleStatus;
/*
 * brief AEAD algorithms start status
 */
typedef enum
{
    AEAD_STARTED_NONE = 0,
    AEAD_CCM_EXAMPLE_STARTED = 1,
    AEAD_GCM_EXAMPLE_STARTED = 2,
} AeadExampleStarted;
/*
 * ============================================================================
 *                               LOCAL FUNCTIONS DECLARATION
 * ============================================================================
*/
hseSrvResponse_t HseAeadExamples(void);

#ifdef __cplusplus
}
#endif
#endif /* SERVICES_INC_HSE_AEAD_EXAMPLE_H_ */
