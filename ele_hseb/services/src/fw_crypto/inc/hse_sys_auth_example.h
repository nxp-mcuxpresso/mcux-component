/*
 * @file hse_sys_auth_example.h
 * @brief Header file for SYS_AUTH examples in Crypto services
 * @details Contains the API's for demonstrating the use of SYS_AUTH services in demo APP
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
#ifndef SERVICES_INC_HSE_SYS_AUTH_EXAMPLE_H_
#define SERVICES_INC_HSE_SYS_AUTH_EXAMPLE_H_

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
#include "hse_host_sys_authorization.h"
#include "hse_host_sign.h"
#include "hse_host_rsa_cipher.h"
/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/

#define CHALLENGE_LENGTH            (32UL)
#define SIGN_LENGTH_MAX             (512UL)
#define PSS_SALT_LENGTH             (20UL)
#define CUST_SUPER_USER_MODE(hseStatus) \
    ( HSE_STATUS_CUST_SUPER_USER == \
(hseStatus & (HSE_STATUS_CUST_SUPER_USER|HSE_STATUS_OEM_SUPER_USER)))

#define OEM_SUPER_USER_MODE(hseStatus) \
    (HSE_STATUS_OEM_SUPER_USER == \
        (hseStatus & (HSE_STATUS_CUST_SUPER_USER|HSE_STATUS_OEM_SUPER_USER)))

#define IN_FIELD_USER_MODE(hseStatus) \
  (0UL == (hseStatus & (HSE_STATUS_CUST_SUPER_USER|HSE_STATUS_OEM_SUPER_USER)))
#ifdef HSE_SPT_RSA

/*
 * ============================================================================
 *                               LOCAL FUNCTIONS DECLARATION
 * ============================================================================
*/

hseSrvResponse_t SetCustAuthorizationKey(void);
hseSrvResponse_t AuthorizeCust(void);
hseSrvResponse_t HSE_SysAuthorization_Example(void);
hseSrvResponse_t Grant_SuperUser_Rights(void);
#endif

#ifdef __cplusplus
}
#endif
#endif /* SERVICES_INC_HSE_SYS_AUTH_EXAMPLE_H_ */
