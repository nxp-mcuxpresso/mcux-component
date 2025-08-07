/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef traceTASK_CREATE
#if defined(__GNUC__)
    extern void coredump_freertos_trace_task_create(void *tcb);
#endif
    #define traceTASK_CREATE coredump_freertos_trace_task_create
#else
#warning "To enable dumping of all thread information, ensure that coredump_freertos_trace_task_create is included in the implementation of traceTASK_CREATE."
#endif
    
#ifndef traceTASK_DELETE
#if defined(__GNUC__)
    extern void coredump_freertos_trace_task_delete(void *tcb);
#endif
    #define traceTASK_DELETE coredump_freertos_trace_task_delete
#else
#warning "To enable dumping of all thread information, ensure that coredump_freertos_trace_task_delete is included in the implementation of traceTASK_DELETE."
#endif
