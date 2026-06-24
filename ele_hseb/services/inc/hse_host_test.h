/**
 *   @file    hse_host_test.h
 *
 *   @brief   Function prototypes to test SHE  Commands.
 *   @details
 *
 *   @addtogroup [SECURITY_FIRMWARE_UNITTEST]
 *   @{
 */
/*==================================================================================================
*   Copyright 2022 NXP.
*
*   License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025
==================================================================================================*/
/*==================================================================================================
==================================================================================================*/

#ifndef _HOST_TEST_H_
#define _HOST_TEST_H_
#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
==================================================================================================*/
#include "std_typedefs.h"
#include "hse_common_types.h"
#include "hse_srv_random.h"
#include "hse_srv_responses.h"
/*==================================================================================================
*                                          CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/

/*==================================================================================================
*                                             ENUMS
==================================================================================================*/

/*==================================================================================================
*                                STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
*                                GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/
/*==================================================================================================
*                                    FUNCTION PROTOTYPES
==================================================================================================*/
extern uint8_t MASTER_ECU_KEY[16];
hseSrvResponse_t Test_ECB_ENC_DEC(void);
hseSrvResponse_t Test_CBC_ENC_DEC(void);

hseSrvResponse_t Test_SHE_LOAD_KEYS(void);
hseSrvResponse_t Test_SHE_LOAD_PLAIN_KEY(void);
hseSrvResponse_t Test_CMAC_GENERATE_VERIFY(void);
hseSrvResponse_t Test_CMD_EXTEND_SEED(void);
hseSrvResponse_t Test_RANDOM_NUM_REQ(void);
hseSrvResponse_t Test_SHE_GET_ID(void);
hseSrvResponse_t Test_EXPORT_RAM_KEY(void);
hseSrvResponse_t Test_GET_RAND_NUM_REQ(void);
hseSrvResponse_t Test_CMD_TRNG_RND(void);
hseSrvResponse_t Test_CMD_DEBUG_CHAL_AUTH(void);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef _HOST_TEST_H_ */

/** @} */
