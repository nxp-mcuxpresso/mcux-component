/*
 * hse_BD_example.h
 *
 *
 * @brief Header file for Burmester Desmedt Example in crypto services
 * @details Contains the API's for demonstrating the use of Burmester Desmedt crypto service in DemoApp
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

#ifndef SERVICES_INC_HSE_BD_EXAMPLE_H_
#define SERVICES_INC_HSE_BD_EXAMPLE_H_

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
#include "hse_host_ecc.h"
#include "hse_host_sign.h"
#include "hse_host_kdf.h"


/******************************************************************************
 * Function:    HSE_BurmesterDesmedt_Example
 * Description: Example of Burmester Desmedt sent Synchronously
 *
 *****************************************************************************/
hseSrvResponse_t HSE_BurmesterDesmedt_Example(void);


#ifdef __cplusplus
}
#endif

#endif /* SERVICES_INC_HSE_BD_EXAMPLE_H_ */
