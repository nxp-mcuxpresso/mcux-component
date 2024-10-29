/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SMBUS_EVENT_H_
#define _FSL_SMBUS_EVENT_H_

#include "fsl_common.h"
#include "fsl_os_abstraction.h"

/*!
 * @addtogroup smbus_event SMBUS EVENT
 * @ingroup SMBUS
 * @{
 */
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!@brief transfer event */
#define SMBUS_EVENT_TRANSFER_MASTER_COMPLETE (1UL << 0U)
#define SMBUS_EVENT_TRANSFER_SLAVE_COMPLETE  (1UL << 1U)
#define SMBUS_EVENT_TRANSFER_MASTER_ERROR    (1U << 2U)
/*!@brief enable semphore by default */
#ifndef SMBUS_POLLING_EVENT_BY_SEMPHORE
#define SMBUS_POLLING_EVENT_BY_SEMPHORE 1
#endif

/*!@brief smbus event event */
typedef struct _smbus_event
{
#if defined(SMBUS_POLLING_EVENT_BY_SEMPHORE) && SMBUS_POLLING_EVENT_BY_SEMPHORE
    volatile uint32_t eventFlag;
    OSA_SEMAPHORE_HANDLE_DEFINE(handle);
#else
    OSA_EVENT_HANDLE_DEFINE(handle);
#endif
} smbus_event_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name smbus event Function
 * @{
 */

/*!
 * @brief OSA Create event.
 * @param eventHandle event handle.
 * @retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventCreate(void *eventHandle);

/*!
 * @brief Wait event.
 *
 * @param eventHandle The event type
 * @param eventType Timeout time in milliseconds.
 * @param timeoutMilliseconds timeout value in ms.
 * @param event event flags.
 * @retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventWait(void *eventHandle, uint32_t eventType, uint32_t timeoutMilliseconds, uint32_t *event);

/*!
 * @brief set event.
 * @param eventHandle event handle.
 * @param eventType The event type
 * @retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventSet(void *eventHandle, uint32_t eventType);

/*!
 * @brief Get event flag.
 * @param eventHandle event handle.
 * @param eventType event type.
 * @param flag pointer to store event value.
 * @retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventGet(void *eventHandle, uint32_t eventType, uint32_t *flag);

/*!
 * @brief clear event flag.
 * @param eventHandle event handle.
 * @param eventType The event type
 * @retval kStatus_Fail or kStatus_Success.
 */
status_t SMBUS_EventClear(void *eventHandle, uint32_t eventType);

/*!
 * @brief Delete event.
 * @param eventHandle The event handle.
 */
status_t SMBUS_EventDestroy(void *eventHandle);

/*! @} */

#if defined(__cplusplus)
}
#endif
/*! @} */
#endif /* _FSL_SMBUS_EVENT_H_*/
