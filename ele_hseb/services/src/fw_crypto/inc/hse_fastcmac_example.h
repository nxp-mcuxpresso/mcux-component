/*
 * hse_fastcmac_example.h
* @brief Header file for Fast CMAC example in crypto services
 * @details Contains the API's for demonstrating the use of Fast CMAC crypto service in DemoApp
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

#ifndef SERVICES_INC_HSE_FASTCMAC_EXAMPLE_H_
#define SERVICES_INC_HSE_FASTCMAC_EXAMPLE_H_

#ifdef __cplusplus
extern "C"
{
#endif
/*==================================================================================================
*                                        INCLUDE FILES
* 1) Catalog formatting,Host Import keys and Demo App Services header files will be comman in all examples
* 2) Further example specific files will be included
==================================================================================================*/
#include "hse_b_catalog_formatting.h"
#include "hse_host_import_key.h"
#include "hse_demo_app_services.h"
#include "hse_monotonic_cnt.h"
#include "hse_host_cmac_with_counter.h"
/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/
#define BUFFER_SIZE                 (512U)
#define MAX_REQS_FOR_FAST_CMAC        (50U)

/*==================================================================================================
 *                                      LOCAL VARIABLES
 ==================================================================================================*/
extern hseKeyHandle_t AesNVMKeyHandle;

#ifdef HSE_SPT_MONOTONIC_COUNTERS
/******************************************************************************
 * Function:    HSE_FastCmacwithCounter_Example
 * Description: Example of Fast CMAC with  sent Synchronously
 *
 *****************************************************************************/
 hseSrvResponse_t HSE_FastCmacwithCounter_Example(void);
#endif



#ifdef __cplusplus
}
#endif

#endif /* SERVICES_INC_HSE_FASTCMAC_EXAMPLE_H_ */
