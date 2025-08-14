/*
 * Copyright 2018-2019, 2023, 2025 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_device_registers.h"
#include "fsl_adapter_timer.h"
#include "fsl_ctimer.h"

#define gStackTimerChannel_c (kCTIMER_Match_0)
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

static CTIMER_Type *const s_CtimerBase[] = CTIMER_BASE_PTRS;

static hal_timer_handle_t s_timerHandle[sizeof(s_CtimerBase) / sizeof(CTIMER_Type *)];
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

    if (halTimerState->callback != NULL)
    {
        halTimerState->callback(halTimerState->callbackParam);
    }
}

#if defined(CTIMER0)
void ctimer0_match0_callback(uint32_t flags);
void ctimer0_match0_callback(uint32_t flags)
{
    HAL_TimerInterruptHandle(0);
}
#else /* CTIMER0 */
#define ctimer0_match0_callback (ctimer_callback_t)0u
#endif /* CTIMER0 */

#if defined(CTIMER1)
void ctimer1_match0_callback(uint32_t flags);
void ctimer1_match0_callback(uint32_t flags)
{
    HAL_TimerInterruptHandle(1);
}
#else /* CTIMER1 */
#define ctimer1_match0_callback (ctimer_callback_t)0u
#endif /* CTIMER1 */

#if defined(CTIMER2)
void ctimer2_match0_callback(uint32_t flags);
void ctimer2_match0_callback(uint32_t flags)
{
    HAL_TimerInterruptHandle(2);
}
#else /* CTIMER2 */
#define ctimer2_match0_callback (ctimer_callback_t)0u
#endif /* CTIMER2 */

#if defined(CTIMER3)
void ctimer3_match0_callback(uint32_t flags);
void ctimer3_match0_callback(uint32_t flags)
{
    HAL_TimerInterruptHandle(3);
}
#else /* CTIMER3 */
#define ctimer3_match0_callback (ctimer_callback_t)0u
#endif /* CTIMER3 */

#if defined(CTIMER4)
void ctimer4_match0_callback(uint32_t flags);
void ctimer4_match0_callback(uint32_t flags)
{
    HAL_TimerInterruptHandle(4);
}
#else /* CTIMER4 */
#define ctimer4_match0_callback (ctimer_callback_t)0u
#endif /* CTIMER4 */

#if defined(CTIMER5)
void ctimer5_match0_callback(uint32_t flags);
void ctimer5_match0_callback(uint32_t flags)
{
    HAL_TimerInterruptHandle(5);
}
#else /* CTIMER5 */
#define ctimer5_match0_callback (ctimer_callback_t)0u
#endif /* CTIMER5 */

#if defined(CTIMER6)
void ctimer6_match0_callback(uint32_t flags);
void ctimer6_match0_callback(uint32_t flags)
{
    HAL_TimerInterruptHandle(6);
}
#else /* CTIMER6 */
#define ctimer6_match0_callback (ctimer_callback_t)0u
#endif /* CTIMER6 */

#if defined(CTIMER7)
void ctimer7_match0_callback(uint32_t flags);
void ctimer7_match0_callback(uint32_t flags)
{
    HAL_TimerInterruptHandle(7);
}
#else /* CTIMER7 */
#define ctimer7_match0_callback (ctimer_callback_t)0u
#endif /* CTIMER7 */

static ctimer_callback_t ctimer_callback_table[] = {ctimer0_match0_callback, ctimer1_match0_callback,
    ctimer2_match0_callback, ctimer3_match0_callback, ctimer4_match0_callback, ctimer5_match0_callback,
    ctimer6_match0_callback, ctimer7_match0_callback};

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
hal_timer_status_t HAL_TimerInit(hal_timer_handle_t halTimerHandle, hal_timer_config_t *halTimerConfig)
{
    IRQn_Type instanceIrq[] = CTIMER_IRQS;
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    ctimer_config_t config;
    assert(sizeof(hal_timer_handle_struct_t) == HAL_TIMER_HANDLE_SIZE);
    assert(halTimerConfig);
    assert(halTimerHandle);
    assert(halTimerConfig->instance < (sizeof(s_CtimerBase) / sizeof(CTIMER_Type *)));
    assert(halTimerConfig->instance < (sizeof(ctimer_callback_table) / sizeof(ctimer_callback_t)));

    CTIMER_GetDefaultConfig(&config);
    halTimerState->instance                = halTimerConfig->instance;
    halTimerState->timerClock_Hz           = (uint32_t)halTimerConfig->srcClock_Hz / (uint32_t)(config.prescale + 1U);
    s_timerHandle[halTimerState->instance] = halTimerHandle;
    NVIC_SetPriority(instanceIrq[halTimerState->instance], HAL_TIMER_ISR_PRIORITY);

    CTIMER_Init(s_CtimerBase[halTimerState->instance], &config);
    CTIMER_StopTimer(s_CtimerBase[halTimerState->instance]);
    CTIMER_RegisterCallBack(s_CtimerBase[halTimerState->instance], &ctimer_callback_table[halTimerState->instance],
                            kCTIMER_SingleCallback);

    return HAL_TimerUpdateTimeout(halTimerHandle, halTimerConfig->timeout);
}
/*************************************************************************************/
void HAL_TimerDeinit(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    s_timerHandle[halTimerState->instance]   = NULL;
    CTIMER_Deinit(s_CtimerBase[halTimerState->instance]);
}
/*************************************************************************************/
void HAL_TimerEnable(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    CTIMER_StartTimer(s_CtimerBase[halTimerState->instance]);
}

/*************************************************************************************/
void HAL_TimerDisable(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    CTIMER_StopTimer(s_CtimerBase[halTimerState->instance]);
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
    uint32_t reserveCount;
    uint64_t retValue;
    uint32_t reserveMs = 4U;
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    reserveCount                             = (uint32_t)MSEC_TO_COUNT((reserveMs), (halTimerState->timerClock_Hz));

    retValue = COUNT_TO_USEC(((uint64_t)0xFFFFFFFF - (uint64_t)reserveCount), (uint64_t)halTimerState->timerClock_Hz);
    return (uint32_t)((retValue > 0xFFFFFFFFU) ? (0xFFFFFFFFU - reserveMs * 1000U) : (uint32_t)retValue);
}

/* return micro us */
uint32_t HAL_TimerGetCurrentTimerCount(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    return (uint32_t)COUNT_TO_USEC(s_CtimerBase[halTimerState->instance]->TC, halTimerState->timerClock_Hz);
}

hal_timer_status_t HAL_TimerUpdateTimeout(hal_timer_handle_t halTimerHandle, uint32_t timeout)
{
    ctimer_match_config_t mCtimerMatchConfig;
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    assert(halTimerHandle);

    halTimerState->timeout = timeout;

    /* Configuration 0 */
    mCtimerMatchConfig.enableCounterReset = HAL_CTIMER_COUNTER_RESET;
    mCtimerMatchConfig.enableCounterStop  = false;
    mCtimerMatchConfig.outControl         = kCTIMER_Output_NoAction;
    mCtimerMatchConfig.outPinInitState    = false;
    mCtimerMatchConfig.enableInterrupt    = true;
    mCtimerMatchConfig.matchValue = (uint32_t)USEC_TO_COUNT(halTimerState->timeout, halTimerState->timerClock_Hz);
    if ((mCtimerMatchConfig.matchValue < 1U) || (mCtimerMatchConfig.matchValue > 0xFFFFFFF0U))
    {
        return kStatus_HAL_TimerOutOfRanger;
    }
    CTIMER_Reset(s_CtimerBase[halTimerState->instance]);
    CTIMER_SetupMatch(s_CtimerBase[halTimerState->instance], (ctimer_match_t)gStackTimerChannel_c, &mCtimerMatchConfig);
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

uint32_t HAL_TimerGetCurrentTicks(hal_timer_handle_t halTimerHandle)
{
    assert(halTimerHandle);
    hal_timer_handle_struct_t *halTimerState = halTimerHandle;
    return s_CtimerBase[halTimerState->instance]->TC;
}

void HAL_TimerUpdateMatchValueInTicks(hal_timer_handle_t halTimerHandle, uint32_t matchValue)
{
    hal_timer_handle_struct_t *halTimerState       = halTimerHandle;
    (s_CtimerBase[halTimerState->instance])->MR[0] = matchValue;
}

