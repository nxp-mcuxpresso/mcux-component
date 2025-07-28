/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FSL_OSA_THREAD
#define _FSL_OSA_THREAD

#include "fsl_common.h"

void coredump_freertos_trace_task_create(void *tcb);

void coredump_freertos_trace_task_delete(void *tcb);

/*!
 * @brief Map current task/thread into zephyr format.
 *
 * @return Pointer to the k_thread structure.
 */
struct k_thread *OSA_GetCurrentThread(void);

/*!
 * @brief Get counts of threads.
 * 
 * @return Counts of threads.
 */
uint8_t OSA_GetThreadNum(void);

/*!
 * @brief Populate global variable kernel with input thread list.
 * 
 * @param [in] ptrThreads Pointer to the start of thread list.
 */
void OSA_PopulateKernelInstance(void *ptrThreads, uint32_t taskCount);

#endif /* _FSL_OSA_THREAD */
