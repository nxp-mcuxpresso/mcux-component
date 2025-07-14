/*
 * hse_ecc_example.h
 *
  * @brief Header file for ECC Example in crypto services
 * @details Contains the API's for demonstrating the use of ECC crypto service in DemoApp
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

#ifndef SERVICES_INC_HSE_ECC_EXAMPLE_H_
#define SERVICES_INC_HSE_ECC_EXAMPLE_H_
/*==================================================================================================
*
*   Copyright 2019 - 2022 NXP.
*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/

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
#include "hse_host_ecc.h"
#include "hse_host_sign.h"

typedef struct ecdsaVerifTestCase_tag
{
    uint32_t       curveId;
    uint32_t       hashAlgo;
    const uint8_t *pMsg;
    uint8_t        msgLen;
    const uint8_t *pPubKey;
    const uint8_t *r;
    uint16_t       rLen;
    const uint8_t *s;
    uint16_t       sLen;
}ecdsaVerifTestCase_t;

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

 #ifdef HSE_SPT_ECC_COMPRESSED_KEYS
hseSrvResponse_t HSE_Ecdsa_Example(void);

hseSrvResponse_t TestEccPublicKeyExportSuite(void);

hseSrvResponse_t TestEcdsaVerifTestCase(ecdsaVerifTestCase_t* pTestCase);

#endif



#ifdef __cplusplus
}
#endif



 #endif /* SERVICES_INC_HSE_ECC_EXAMPLE_H_ */
