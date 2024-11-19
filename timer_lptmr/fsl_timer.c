/*
 * Copyright  2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_timer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LPTMR_MAX_COUNT_USED 0xFFFFU

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile uint64_t lptmrCounter = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* LPTMR interrupt handler */
void LPTMR0_IRQHandler(void)
{
    LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
    lptmrCounter++;
}

/* Initialize the LPTMR for counting data */
void Timer_Init(void)
{
    lptmr_config_t lptmrConfig;
    uint32_t clkFrq = CLOCK_GetFreq(kCLOCK_LpoClk);

    LPTMR_GetDefaultConfig(&lptmrConfig);
    /* Initialize the LPTMR */
    LPTMR_Init(LPTMR0, &lptmrConfig);

    /* Set timer period to 1000000 us = 1s */
    LPTMR_SetTimerPeriod(LPTMR0, USEC_TO_COUNT(1000 * 1000, clkFrq));
    /* Enable timer interrupt */
    LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);
    /* Start counting */
    LPTMR_StartTimer(LPTMR0);

    /* Enable the NVIC */
    EnableIRQ(LPTMR0_IRQn);
}

/* Millisecond delay function */
void Timer_Delay_ms(uint32_t timeValue)
{
    uint32_t ts = Timer_GetTime() + timeValue;

    while (ts > Timer_GetTime())
    {
        __NOP();
    }
}

uint32_t Timer_GetTime(void)
{
    uint64_t count;
    uint32_t freq = CLOCK_GetFreq(kCLOCK_LpoClk);

    count = LPTMR_GetCurrentTimerCount(LPTMR0);
    count += lptmrCounter * USEC_TO_COUNT(1000 * 1000, freq);

    return COUNT_TO_MSEC(count, freq);
}
