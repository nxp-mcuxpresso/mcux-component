/*
 * hse_UpdateNvmKeys_example.h
 *
 @brief Header file for Update NVM keys example in crypto services
 * @details Contains the API's for demonstrating the use of Update NVM keys crypto service in DemoApp
 */
/*--------------------------------------------------------------------------*/
/* Copyright 2022 NXP                                                       */
/*                                                                          */
/* License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025 */
/*--------------------------------------------------------------------------*/
#ifndef SERVICES_INC_HSE_UPDATENVMKEYS_EXAMPLE_H_
#define SERVICES_INC_HSE_UPDATENVMKEYS_EXAMPLE_H_

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
#include "hse_host_mac.h"
#include "hse_host_cipher.h"

/******************************************************************************
 * Function:    HSE_UpdateNvmKey_Example
 * Description: Example of how to update a NVM key with SU rights
 *              (must be imported in an authenticated key container)
 *              Import RSA keys, Authorization Request/Response,
 *              RSA Sign generation, Erase Key
 *****************************************************************************/
#ifdef HSE_SPT_GMAC
hseSrvResponse_t HSE_UpdateNvmKey_Example(void);
#endif


#ifdef __cplusplus
}
#endif


#endif /* SERVICES_INC_HSE_UPDATENVMKEYS_EXAMPLE_H_ */
