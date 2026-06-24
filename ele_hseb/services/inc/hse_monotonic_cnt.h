/*============================================================================*/

/*==================================================================================================
*
*   Copyright 2022 NXP.
*
*   License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025
==================================================================================================*/
/*============================================================================*/

/*=============================================================================
  Description
  ============================================================================*/
/**
 *   @file    hse_monotonic_cnt.h
 *
 *   @brief   This is a monotonic counter wrapper file.
 */
#ifndef MONOTONIC_COUNTER_H
#define MONOTONIC_COUNTER_H
/*=============================================================================
 *                               INCLUDE FILES
 =============================================================================*/
#include "hse_common_types.h"
#include "hse_srv_monotonic_cnt.h"
#include "hse_srv_responses.h"
/*=============================================================================
                                 MACROS
  ============================================================================*/

/*=============================================================================
 *                               TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
 =============================================================================*/

/*=============================================================================
 *                               GLOBAL VARIABLES
 =============================================================================*/

/*=============================================================================
                                 GLOBAL FUNCTION
  ============================================================================*/

hseSrvResponse_t MonotonicCnt_Increment(uint32_t CounterIndex,
                                        uint32_t CounterNewValue);
hseSrvResponse_t MonotonicCnt_Read(uint32_t CounterIndex,
                                   HOST_ADDR CounterValue);
hseSrvResponse_t MonotonicCnt_Config(uint32_t CounterIndex,
                                     uint8_t RPBitSize);
void monotonicCounterService(void);
hseSrvResponse_t monotonicCounterserviceSelect(void);

#endif /* MONOTONIC_COUNTER_H */
