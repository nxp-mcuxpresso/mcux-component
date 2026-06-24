/*
 * hse_sessionkeys_example.h
 *
 @brief Header file for Session keys in crypto services
 * @details Contains the API's for demonstrating the use of Session keys crypto service in DemoApp
 */

#ifndef SERVICES_INC_HSE_SESSIONKEYS_EXAMPLE_H_
#define SERVICES_INC_HSE_SESSIONKEYS_EXAMPLE_H_
/*--------------------------------------------------------------------------*/
/* Copyright 2022 NXP                                                       */
/*                                                                          */
/* License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025 */
/*--------------------------------------------------------------------------*/
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
#include "hse_host_kdf.h"
#include "hse_host_ecc.h"
#include "hse_host_sign.h"
#include "hse_host_cipher.h"
#include "hse_host_aead.h"

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

#ifdef HSE_SPT_KEY_DERIVE
hseSrvResponse_t HSE_SessionKeys_Example(void);
#endif

#endif /* SERVICES_INC_HSE_SESSIONKEYS_EXAMPLE_H_ */
