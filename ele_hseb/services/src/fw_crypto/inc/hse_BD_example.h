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
*   License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025
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
