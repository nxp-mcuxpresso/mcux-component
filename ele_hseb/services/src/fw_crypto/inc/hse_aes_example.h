/*
 * hse_aes_example.h
 * @brief Header file for AES example in crypto services
 * @details Contains the API's for demonstrating the use of AES crypto service in DemoApp
 */

/*==================================================================================================
*
*   Copyright-2022 NXP.
*
*   License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025
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
