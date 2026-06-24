/**
*   @file    hse_host_wrappers.h
*

*   @brief   Wrapper function declaration.
*   @details This file contains declaration of Wrapper and helper functions.
*
*   @addtogroup host_wrappers
*   @{
*/
/*==================================================================================================
*
*   Copyright 2022 NXP.
*
*   License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025
==================================================================================================*/

#ifndef HOST_WRAPPERS_H
#define HOST_WRAPPERS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "hse_common_types.h"
#include "std_typedefs.h"
#include "hse_host.h"
/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/

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
                                    STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
                                    GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
                                        FUNCTION PROTOTYPES
==================================================================================================*/
hseSrvResponse_t SheLoadMasterEcuKey(void);
hseSrvResponse_t SheLoadNVMKey_ECB(void);
hseSrvResponse_t SheLoadNVMKey_CBC(void);
hseSrvResponse_t SheLoadNVMKey_CMAC(void);
hseSrvResponse_t EraseAllSheKeys(void);
hseSrvResponse_t Load_Relevant_She_Keys(void);

hseSrvResponse_t SysAuth_User(void);
hseSrvResponse_t HostKdf(uint32_t inputLength, const uint8_t *pInput, uint32_t *pHashLength, uint8_t *pHash, hseSGTOption_t inputSgtType);

hseSrvResponse_t HSEConfigKeyCatalogs(void);

#ifdef __cplusplus
}
#endif

#endif /* HOST_WRAPPERS_H */

/** @} */
