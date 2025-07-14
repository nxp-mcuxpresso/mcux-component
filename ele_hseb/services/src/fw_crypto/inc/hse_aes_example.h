/*
 * hse_aes_example.h
 * @brief Header file for AES example in crypto services
 * @details Contains the API's for demonstrating the use of AES crypto service in DemoApp
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

#ifndef SERVICES_INC_HSE_AES_EXAMPLE_H_
#define SERVICES_INC_HSE_AES_EXAMPLE_H_

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
#include "hse_demo_app_services.h"
#include "hse_host_import_key.h"
//#include "host_stm.h"
#include "hse_host_mac.h"
#include "hse_host_cipher.h"

/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/
#define BUFFER_SIZE                 (512U)
#define MAX_REQS_FOR_FAST_CMAC        (50U)
/*==================================================================================================
 *                                      LOCAL VARIABLES
 ==================================================================================================*/
extern hseKeyHandle_t AesNVMKeyHandle;
/*==================================================================================================
                                        LOCAL FUNCTION PROTOTYPES
  ==================================================================================================*/
/******************************************************************************
 * Function:    HSE_Aes_Example
 * Description:  Example of AES requests sent synchronously.
 *****************************************************************************/
hseSrvResponse_t HSE_Aes_Example(void);
#ifdef __cplusplus
}
#endif

#endif /* SERVICES_INC_HSE_AES_EXAMPLE_H_ */
