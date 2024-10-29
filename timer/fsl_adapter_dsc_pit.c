/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_device_registers.h"
#include "fsl_adapter_timer.h"
#include "fsl_pit.h"

typedef struct _hal_timer_handle_struct_t
{
    uint32_t timeout;
    uint32_t timerClock_Hz;
    hal_timer_callback_t callback;
    void *callbackParam;
    uint8_t instance;
} hal_timer_handle_struct_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

static PIT_Type *const s_PitBase[] = PIT_BASE_PTRS;

static hal_timer_handle_t s_timerHandle[sizeof(s_PitBase) / sizeof(PIT_Type *)];
static uint8_t s_pitPrescaler[sizeof(s_PitBase) / sizeof(PIT_Type *)];

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static void HAL_TimerInterruptHandle(uint8_t instance)
{
    hal_timer_handle_struct_t *halTimerState = (hal_timer_handle_struct_t *)s_timerHandle[instance];

    PIT_ClearStatusFlags(s_PitBase[halTimerState->instance]);
    if (halTimerState->callback != NULL)
    {
        halTimerState->callback(halTimerState->callbackParam);
    }
}

#if defined(PIT0)
#pragma interrupt alignsp saveall
void ivINT_PIT0_ROLLOVR(void);
void ivINT_PIT0_ROLLOVR(void)
{
    HAL_TimerInterruptHandle(0);
}
#pragma interrupt off
#endif
#if defined(PIT1)
#pragma interrupt alignsp saveall
void ivINT_PIT1_ROLLOVR(void);
void ivINT_PIT1_ROLLOVR(void)
{
    HAL_TimerInterruptHandle(1);
}
#pragma interrupt off
#endif

static bool PIT_GetProperPrescaler(
    uint32_t clk, uint32_t us, uint8_t pres_bits, uint8_t *pres, uint8_t cnt_bits, uint32_t *period)
{
    uint32_t cnt_max = ((uint32_t)1 << cnt_bits) - 1U;
    uint32_t dn      = clk / 1000000U;
    uint8_t i        = 0;
    for (i = 0; i < pres_bits; i++)
    {
        *period = us * dn / ((uint32_t)1 << i);
        if (*period <= cnt_max)
        {
            *pres = i;
            return true;
        }
    }

    return false;
}

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
hal_timer_status_t HAL_TimerInit(hal_timer_handle_t halTimerHandle, hal_timer_config_t *halTimerConfig)
{
#ifndef PIT_IRQS
    IRQn_Type instanceIrq[] = {PIT0_ROLLOVR_IRQn, PIT1_ROLLOVR_IRQn};
#else
    IRQn_Type instanceIrq[] = PIT_IRQS;
#endif
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    IRQn_Type irqId;
    /* Structure of initialize PIT */
    pit_config_t pitConfig;
    uint32_t period;
    uint8_t prescaler;
    bool preCheckRes;

    // assert(sizeof(hal_timer_handle_struct_t) == HAL_TIMER_HANDLE_SIZE);
    assert(halTimerConfig);
    assert(halTimerHandle);
    assert(halTimerConfig->instance < (sizeof(s_PitBase) / sizeof(PIT_Type *)));

    halTimerState->timeout  = halTimerConfig->timeout;
    halTimerState->instance = halTimerConfig->instance;
    irqId                   = instanceIrq[halTimerState->instance];
    /*
     * pitConfig.enableRunInDebug = false;
     */
    PIT_GetDefaultConfig(&pitConfig);
    assert(s_PitBase[halTimerState->instance]);
    /* Init pit module */

    /* Calculate Prescaler And Period */
    preCheckRes =
        PIT_GetProperPrescaler(halTimerConfig->srcClock_Hz, halTimerState->timeout, 16, &prescaler, 16, &period);
    if (true != preCheckRes)
    {
        return kStatus_HAL_TimerOutOfRanger;
    }

    halTimerState->timerClock_Hz            = halTimerConfig->srcClock_Hz;
    pitConfig.ePrescaler                    = (pit_prescaler_value_t)prescaler;
    s_pitPrescaler[halTimerState->instance] = prescaler;

    PIT_Init(s_PitBase[halTimerState->instance], &pitConfig);
    PIT_SetTimerPeriod(s_PitBase[halTimerState->instance], (uint16_t)period);
    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupt(s_PitBase[halTimerState->instance]);
    s_timerHandle[halTimerState->instance] = halTimerHandle;

    (void)EnableIRQWithPriority((IRQn_Type)irqId, HAL_TIMER_ISR_PRIORITY);
    return kStatus_HAL_TimerSuccess;
}

/*************************************************************************************/
void HAL_TimerDeinit(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    s_timerHandle[halTimerState->instance]   = NULL;
    PIT_Deinit(s_PitBase[halTimerState->instance]);
}
/*************************************************************************************/
void HAL_TimerEnable(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    PIT_StartTimer(s_PitBase[halTimerState->instance]);
}

/*************************************************************************************/
void HAL_TimerDisable(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    PIT_StopTimer(s_PitBase[halTimerState->instance]);
}

/*************************************************************************************/
void HAL_TimerInstallCallback(hal_timer_handle_t halTimerHandle, hal_timer_callback_t callback, void *callbackParam)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    halTimerState->callback                  = callback;
    halTimerState->callbackParam             = callbackParam;
}

uint32_t HAL_TimerGetMaxTimeout(hal_timer_handle_t halTimerHandle)
{
    hal_timer_handle_struct_t *halTimerState;

    assert(halTimerHandle);

    halTimerState = halTimerHandle;

    uint32_t dn = halTimerState->timerClock_Hz / 1000000UL;

    return 0xFFFFUL * (1UL << (16U - 1U)) / dn;
}

/* return micro us */
uint32_t HAL_TimerGetCurrentTimerCount(hal_timer_handle_t halTimerHandle)
{
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    uint32_t dn                              = halTimerState->timerClock_Hz / 1000000UL;
    uint32_t cnt                             = (uint32_t)PIT_GetCurrentTimerCount(s_PitBase[halTimerState->instance]);

    assert(halTimerHandle);

    return cnt * ((uint32_t)1 << s_pitPrescaler[halTimerState->instance]) / dn;
}

hal_timer_status_t HAL_TimerUpdateTimeout(hal_timer_handle_t halTimerHandle, uint32_t timeout)
{
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    uint32_t period;
    uint8_t prescaler;
    bool preCheckRes;

    assert(halTimerHandle);

    halTimerState->timeout = timeout;

    /* Calculate Prescaler And Period */
    preCheckRes =
        PIT_GetProperPrescaler(halTimerState->timerClock_Hz, halTimerState->timeout, 16, &prescaler, 16, &period);
    if (true != preCheckRes)
    {
        return kStatus_HAL_TimerOutOfRanger;
    }

    HAL_TimerDisable(s_PitBase[halTimerState->instance]);
    PIT_SetTimerPrescaler(s_PitBase[halTimerState->instance], (pit_prescaler_value_t)prescaler);
    PIT_SetTimerPeriod(s_PitBase[halTimerState->instance], (uint16_t)period);
    HAL_TimerEnable(s_PitBase[halTimerState->instance]);

    return kStatus_HAL_TimerSuccess;
}

void HAL_TimerExitLowpower(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
}

void HAL_TimerEnterLowpower(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
}
