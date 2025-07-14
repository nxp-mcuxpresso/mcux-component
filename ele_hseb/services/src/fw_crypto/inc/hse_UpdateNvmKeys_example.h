/*
 * hse_UpdateNvmKeys_example.h
 *
 @brief Header file for Update NVM keys example in crypto services
 * @details Contains the API's for demonstrating the use of Update NVM keys crypto service in DemoApp
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
