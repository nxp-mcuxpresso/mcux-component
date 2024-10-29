/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_systick_timer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_systickSrcClkFreq_khz;
static uint32_t s_systickSrcClkFreq_mhz;
static uint32_t s_systickIntPeriod_tick;
static systick_timer_callback_t s_systickTimerCallback;
static volatile uint32_t s_systickTickCount;

/*******************************************************************************
 * Code
 ******************************************************************************/
static bool SYSTICK_Timer_Compare(uint32_t major1, uint32_t minor1, uint32_t major2, uint32_t minor2)
{
    if ((major1 > major2) || ((major1 == major2) && (minor1 > minor2)))
    {
        return true;
    }

    return false;
}

/*
 * @brief Initialize the timer.
 *
 * This function registers the callback, configures and starts the SysTick to work.
 *
 * @param intPeriod_tick   Number of ticks between two SysTick interrupts, should be 1~0x1000000.
 * @param srcClkFreq_hz    SysTick source clock frequency.
 * @param callback         Callback function when SysTick interrupt happens.
 * @return kStatus_Success, else failed.
 */
status_t SYSTICK_Timer_Init(uint32_t intPeriod_tick, uint32_t srcClkFreq_hz, systick_timer_callback_t callback)
{
    status_t status;

    if ((intPeriod_tick == 0UL) || (srcClkFreq_hz == 0UL))
    {
        return kStatus_InvalidArgument;
    }

    s_systickTimerCallback  = callback;
    s_systickTickCount      = 0U;
    s_systickSrcClkFreq_khz = srcClkFreq_hz / 1000UL;
    s_systickSrcClkFreq_mhz = srcClkFreq_hz / 1000000UL;
    s_systickIntPeriod_tick = intPeriod_tick;

    if (0UL != SysTick_Config(intPeriod_tick))
    {
        status = kStatus_InvalidArgument;
    }
    else
    {
        status = kStatus_Success;
    }

    return status;
}

/*
 * @brief Deinitialize the timer.
 */
void SYSTICK_Timer_Deinit(void)
{
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
    SysTick->VAL = 0UL;

    s_systickTimerCallback = NULL;
    s_systickTickCount     = 0U;
}

/*
 * @brief Convert time information in tick format to time format.
 *
 * @param tick The input tick to convert.
 * @param time The output time result.
 */
void SYSTICK_Timer_Tick2Time(const systick_tick_t *tick, systick_time_t *time)
{
    /* Convert tick to time. */
    time->ms = (uint32_t)(uint64_t)(((uint64_t)tick->swTick * (uint64_t)s_systickIntPeriod_tick) /
                                    (uint64_t)s_systickSrcClkFreq_khz);
    time->us = tick->hwTick / s_systickSrcClkFreq_mhz;

    while (time->us >= 1000UL)
    {
        time->ms++;
        time->us -= 1000UL;
    }
}

/*
 * @brief Get current time in tick format.
 *
 * @param tick The structure to get result.
 */
void SYSTICK_Timer_GetTick(systick_tick_t *tick)
{
    uint32_t checkSwTick;

    tick->swTick = s_systickTickCount;

    while (true)
    {
        tick->hwTick = SysTick->VAL;

        checkSwTick = s_systickTickCount;

        /*
         * If the new read s_systickTickCount is the same with the previous one,
         * it means no new systick interrupt happens in this period.
         */
        if (checkSwTick != tick->swTick)
        {
            tick->swTick = checkSwTick;
        }
        else
        {
            break;
        }
    }

    tick->hwTick = s_systickIntPeriod_tick - tick->hwTick;
}

/*
 * @brief This function compares two times in systick_tick_t.
 *
 * @param tick1 Structure that holds the time.
 * @param tick2 Structure that holds the time.
 * @return Return true if time1 is greater than time2, else false.
 */
bool SYSTICK_Timer_CompareTick(const systick_tick_t *tick1, const systick_tick_t *tick2)
{
    return SYSTICK_Timer_Compare(tick1->swTick, tick1->hwTick, tick2->swTick, tick2->hwTick);
}

/*
 * @brief Get current time in time format.
 *
 * @param time The structure to get result.
 */
void SYSTICK_Timer_GetTime(systick_time_t *time)
{
    systick_tick_t tick;

    SYSTICK_Timer_GetTick(&tick);
    SYSTICK_Timer_Tick2Time(&tick, time);
}

/*
 * @brief Add time (in us) to a time in systick_time_t.
 *
 * @param resultTime Structure that holds the added result time.
 * @param time       Structure that holds the time
 * @param us         Time in us to add
 */
void SYSTICK_Timer_AddTime(systick_time_t *resultTime, const systick_time_t *time, uint32_t us)
{
    resultTime->us = time->us + us;
    resultTime->ms = time->ms;

    while (resultTime->us >= 1000UL)
    {
        resultTime->ms++;
        resultTime->us -= 1000UL;
    }
}

/*
 * @brief This function compares two times time in systick_time_t.
 *
 * @param time1 Structure that holds the time.
 * @param time2 Structure that holds the time.
 * @return Return true if time1 is greater than time2, else false.
 */
bool SYSTICK_Timer_CompareTime(const systick_time_t *time1, const systick_time_t *time2)
{
    return SYSTICK_Timer_Compare(time1->ms, time1->us, time2->ms, time2->us);
}

void SysTick_Handler(void)
{
    s_systickTickCount++;

    if (NULL != s_systickTimerCallback)
    {
        s_systickTimerCallback();
    }
}
