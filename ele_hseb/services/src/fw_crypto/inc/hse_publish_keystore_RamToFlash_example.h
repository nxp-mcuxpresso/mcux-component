/*
 * hse_publish_keystore_RamToFlash_example.h
 * @brief Header file for publish_keystore_RamToFlash example in crypto services
 * @details Contains the API's for demonstrating the use of publish_keystore_RamToFlash_example crypto service in DemoApp
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
#ifndef SERVICES_SRC_FW_CRYPTO_INC_HSE_PUBLISH_KEYSTORE_RAMTOFLASH_EXAMPLE_H_
#define SERVICES_SRC_FW_CRYPTO_INC_HSE_PUBLISH_KEYSTORE_RAMTOFLASH_EXAMPLE_H_

/*==================================================================================================
*                                        INCLUDE FILES
* 1) Catalog formatting,Host Import keys and Demo App Services header files will be comman in all examples
* 2) Further example specific files will be included
==================================================================================================*/
#include "hse_b_catalog_formatting.h"
#include "hse_host_import_key.h"
#include "hse_demo_app_services.h"
//#include "host_stm.h"
#include "hse_host_attrs.h"

/*==================================================================================================
                                     ENUMS
==================================================================================================*/
typedef enum
{
    PUBLISH_KEYSTORE_RAM_TO_FLASH_NOT_STARTED = 0,
    PUBLISH_KEYSTORE_RAM_TO_FLASH_STARTED = 1,
    PUBLISH_KEYSTORE_RAM_TO_FLASH_SUCCESSFUL = 2,
    PUBLISH_KEYSTORE_RAM_TO_FLASH_UNSUCCESSFUL = 4
}publishKeystoreRamtoFlashStatus;

/*
 * ============================================================================
 *                               LOCAL FUNCTIONS DECLARATION
 * ============================================================================
*/
#ifdef HSE_SPT_PUBLISH_NVM_KEYSTORE_RAM_TO_FLASH
hseSrvResponse_t HSE_publish_keystore_RamToFlash_example_(void);
#endif
#endif /* SERVICES_SRC_FW_CRYPTO_INC_HSE_PUBLISH_KEYSTORE_RAMTOFLASH_EXAMPLE_H_ */
