/*--------------------------------------------------------------------------*/
/* Copyright 2025 NXP                                                       */
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

#ifndef HSE_HOST_HASH_H
#define HSE_HOST_HASH_H

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/

#include "hse_interface.h"

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

#ifdef HSE_SPT_STREAM_CTX_IMPORT_EXPORT

hseSrvResponse_t StreamCtxImport(hseStreamId_t streamId, const uint8_t* pStreamingContext);
hseSrvResponse_t StreamCtxExport(hseStreamId_t streamId, uint8_t* pStreamingContext);
hseSrvResponse_t StreamCtxOp(hseStreamContextOp_t op, hseStreamId_t streamId, uint8_t* pStreamingContext);
hseSrvResponse_t StreamCtxOpMuInstance(hseStreamContextOp_t op, hseStreamId_t streamId, uint8_t* pStreamingContext, uint8_t muInstance);

#endif /* HSE_SPT_STREAM_CTX_IMPORT_EXPORT */

#ifdef __cplusplus
}
#endif

#endif /* HSE_HOST_HASH_H */

/** @} */
