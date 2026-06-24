/*
 *@file hse_get_key_info.h
 *@brief This file contains functions to get the information about the keys
 *@details Contains the API's for demonstrating how to get the information about the key by just using its key handle
 */
/*--------------------------------------------------------------------------*/
/* Copyright 2022 NXP                                                       */
/*                                                                          */
/* License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025 */
/*--------------------------------------------------------------------------*/
#ifndef SERVICES_INC_HSE_GET_KEY_INFO_H_
#define SERVICES_INC_HSE_GET_KEY_INFO_H_

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

/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/
/*
 * @brief MACRO defining the MAX counter value for RAM keys
 */
#define HSE_RAM_KEY_MAX_COUNTER_VALUE 0xFFFFFFFF  // For RAM keys counter value is always forced to 0xfffffff
/*
 * @brief MACRO defining the SMR value of the RAM keys
 */
#define HSE_SMR_VALUE_RAM_KEYS 0x00           //For RAM keys SMR value is always forced to 0

/*
 * ============================================================================
 *                               LOCAL FUNCTIONS DECLARATION
 * ============================================================================
*/

hseSrvResponse_t HSE_Get_Key_Info_Example(void);
hseSrvResponse_t HSE_Get_Key_Info(hseKeyHandle_t KeyHandle, hseKeyInfo_t *KeyINFO);

#ifdef __cplusplus
}
#endif

#endif /* SERVICES_INC_HSE_GET_KEY_INFO_H_ */
