/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_smbus_event.h"

/*******************************************************************************
 * Definitons
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if (FSL_OSA_BM_TIMER_CONFIG == FSL_OSA_BM_TIMER_SYSTICK)
void OSA_TimeInit(void);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * brief OSA Create event.
 * param event handle.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventCreate(void *eventHandle)
{
    assert(eventHandle != NULL);

#if (FSL_OSA_BM_TIMER_CONFIG == FSL_OSA_BM_TIMER_SYSTICK)
    OSA_TimeInit();
#endif

#if defined(SMBUS_POLLING_EVENT_BY_SEMPHORE) && SMBUS_POLLING_EVENT_BY_SEMPHORE
    (void)OSA_SemaphoreCreate(&(((smbus_event_t *)eventHandle)->handle), 0U);
#else
    (void)OSA_EventCreate(&(((smbus_event_t *)eventHandle)->handle), true);
#endif

    return kStatus_Success;
}

/*!
 * brief Wait event.
 *
 * param eventHandle The event type
 * param eventType Timeout time in milliseconds.
 * param timeoutMilliseconds timeout value in ms.
 * param event event flags.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventWait(void *eventHandle, uint32_t eventType, uint32_t timeoutMilliseconds, uint32_t *event)
{
    assert(eventHandle != NULL);

    osa_status_t status = KOSA_StatusError;

#if defined(SMBUS_POLLING_EVENT_BY_SEMPHORE) && SMBUS_POLLING_EVENT_BY_SEMPHORE
    while (true)
    {
        status = OSA_SemaphoreWait(&(((smbus_event_t *)eventHandle)->handle), timeoutMilliseconds);
        if (KOSA_StatusTimeout == status)
        {
            break;
        }

        if (KOSA_StatusSuccess == status)
        {
            (void)SMBUS_EventGet(eventHandle, eventType, event);
            if ((*event & eventType) != 0U)
            {
                return kStatus_Success;
            }
        }
    }

#else
    while (true)
    {
        status = OSA_EventWait(&(((smbus_event_t *)eventHandle)->handle), eventType, 0, timeoutMilliseconds, event);
        if ((KOSA_StatusSuccess == status) || (KOSA_StatusTimeout == status))
        {
            break;
        }
    }

    if (KOSA_StatusSuccess == status)
    {
        return kStatus_Success;
    }
#endif

    return kStatus_Fail;
}

/*!
 * brief set event.
 * param event event handle.
 * param eventType The event type
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventSet(void *eventHandle, uint32_t eventType)
{
    assert(eventHandle != NULL);

#if defined(SMBUS_POLLING_EVENT_BY_SEMPHORE) && SMBUS_POLLING_EVENT_BY_SEMPHORE
    OSA_SR_ALLOC();
    OSA_ENTER_CRITICAL();
    ((smbus_event_t *)eventHandle)->eventFlag |= eventType;
    OSA_EXIT_CRITICAL();

    (void)OSA_SemaphorePost(&(((smbus_event_t *)eventHandle)->handle));
#else
    (void)OSA_EventSet(&(((smbus_event_t *)eventHandle)->handle), eventType);
#endif

    return kStatus_Success;
}

/*!
 * brief Get event flag.
 * param eventHandle event handle.
 * param eventType The event type
 * param flag pointer to store event value.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventGet(void *eventHandle, uint32_t eventType, uint32_t *flag)
{
    assert(eventHandle != NULL);
    assert(flag != NULL);

#if defined(SMBUS_POLLING_EVENT_BY_SEMPHORE) && SMBUS_POLLING_EVENT_BY_SEMPHORE
    *flag = ((smbus_event_t *)eventHandle)->eventFlag;
#else
    (void)OSA_EventGet(&(((smbus_event_t *)eventHandle)->handle), eventType, flag);
#endif

    return kStatus_Success;
}

/*!
 * brief clear event flag.
 * param eventHandle event handle.
 * param eventType The event type
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventClear(void *eventHandle, uint32_t eventType)
{
    assert(eventHandle != NULL);

#if defined(SMBUS_POLLING_EVENT_BY_SEMPHORE) && SMBUS_POLLING_EVENT_BY_SEMPHORE
    OSA_SR_ALLOC();
    OSA_ENTER_CRITICAL();
    ((smbus_event_t *)eventHandle)->eventFlag &= ~eventType;
    OSA_EXIT_CRITICAL();
#else
    (void)OSA_EventClear(&(((smbus_event_t *)eventHandle)->handle), eventType);
#endif

    return kStatus_Success;
}

/*!
 * brief Delete event.
 * param event The event handle.
 */
status_t SMBUS_EventDestroy(void *eventHandle)
{
    assert(eventHandle != NULL);

#if defined(SMBUS_POLLING_EVENT_BY_SEMPHORE) && SMBUS_POLLING_EVENT_BY_SEMPHORE
    (void)OSA_SemaphoreDestroy(&(((smbus_event_t *)eventHandle)->handle));
#else
    (void)OSA_EventDestroy(&(((smbus_event_t *)eventHandle)->handle));
#endif

    return kStatus_Success;
}

#if (FSL_OSA_BM_TIMER_CONFIG == FSL_OSA_BM_TIMER_SYSTICK)
void OSA_TimeInit(void)
{
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
    SysTick->LOAD = (uint32_t)(SystemCoreClock / 1000U - 1U);
    SysTick->VAL  = 0;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk;
}
#endif
