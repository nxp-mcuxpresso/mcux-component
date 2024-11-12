/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SYSTICK_TIMER_H_
#define _FSL_SYSTICK_TIMER_H_

#include "fsl_common.h"

/*!
 * @defgroup systick_timer SysTick Timer
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The tick information. */
typedef struct _systick_tick
{
    uint32_t swTick; /*!< How many SysTick interrupts happen. */
    uint32_t hwTick; /*!< How many SysTick clock after pervious interrupt. */
} systick_tick_t;

/*! @brief The time information. */
typedef struct _systick_time
{
    uint32_t ms; /*!< Milli-second. */
    uint32_t us; /*!< Micro-second. */
} systick_time_t;

/*!
 * @brief Function prototype of the Systick handler which is called by the SYSTICK ISR.
 */
typedef void (*systick_timer_callback_t)(void);

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name Initialization
 * @{
 */

/*!
 * @brief Initialize the timer.
 *
 * This function registers the callback, configures and starts the SysTick to work.
 *
 * @param intPeriod_tick   Number of ticks between two SysTick interrupts.
 * @param srcClkFreq_hz    SysTick source clock frequency.
 * @param callback         Callback function when SysTick interrupt happens.
 * @return kStatus_Success, else failed.
 */
status_t SYSTICK_Timer_Init(uint32_t intPeriod_tick, uint32_t srcClkFreq_hz, systick_timer_callback_t callback);

/*!
 * @brief Deinitialize the timer.
 */
void SYSTICK_Timer_Deinit(void);

/*! @} */

/*!
 * @name Tick APIs
 * @{
 */

/*!
 * @brief Convert time information in tick format to time format.
 *
 * @param tick The input tick to convert.
 * @param time The output time result.
 */
void SYSTICK_Timer_Tick2Time(const systick_tick_t *tick, systick_time_t *time);

/*!
 * @brief Get current time in tick format.
 *
 * @param tick The structure to get result.
 */
void SYSTICK_Timer_GetTick(systick_tick_t *tick);

/*!
 * @brief This function compares two times in systick_tick_t.
 *
 * @param tick1 Structure that holds the time.
 * @param tick2 Structure that holds the time.
 * @return Return true if time1 is greater than time2, else false.
 */
bool SYSTICK_Timer_CompareTick(const systick_tick_t *tick1, const systick_tick_t *tick2);

/*! @} */

/*!
 * @name Time APIs
 * @{
 */

/*!
 * @brief Get current time in time format.
 *
 * @param time The structure to get result.
 */
void SYSTICK_Timer_GetTime(systick_time_t *time);

/*!
 * @brief Add time (in us) to a time in systick_time_t.
 *
 * @param resultTime Structure that holds the added result time.
 * @param time       Structure that holds the time
 * @param us         Time in us to add
 */
void SYSTICK_Timer_AddTime(systick_time_t *resultTime, const systick_time_t *time, uint32_t us);

/*!
 * @brief This function compares two times in systick_time_t.
 *
 * @param time1 Structure that holds the time.
 * @param time2 Structure that holds the time.
 * @return Return true if time1 is greater than time2, else false.
 */
bool SYSTICK_Timer_CompareTime(const systick_time_t *time1, const systick_time_t *time2);

/*! @} */

/*!
 * @name Misc
 * @{
 */

/*!
 * @brief SysTick Handler
 */
void SysTick_Handler(void);

/*! @} */

/*! @} */

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_SYSTICK_TIMER_H_ */
