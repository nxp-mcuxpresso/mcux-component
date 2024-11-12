/*
 * Copyright  2016 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_TIMER_H_
#define _FSL_TIMER_H_

#include "fsl_lptmr.h"

/**********************************************************************************************************************
 * Definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * API
 *********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize the timer
 * Initialize the lptmr for delay function.
 */
void Timer_Init(void);
/*!
 * @brief Delay for ms
 * This API will wait for some milliseconds deceided by user.
 */
void Timer_Delay_ms(uint32_t timeValue);

/*!
 * @brief Get delay for ms
 * This API will get the current time with unit of milliseconds.
 */
uint32_t Timer_GetTime(void);

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_TIMER_H_ */
