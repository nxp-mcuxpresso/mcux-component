/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_osa_thread.h"

#include <zephyr_headers/kernel.h>

struct z_kernel _kernel;
struct k_thread _current_Thread;
k_tid_t ptrCurrent = &_current_Thread;
#if defined(SDK_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "task.h"

typedef struct _free_rtos_TCB
{
    volatile StackType_t *pxTopOfStack; /**< Points to the location of the last item placed on the tasks stack.  THIS
                                           MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */

#if (portUSING_MPU_WRAPPERS == 1)
    xMPU_SETTINGS xMPUSettings; /**< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND
                                   MEMBER OF THE TCB STRUCT. */
#endif

#if (configUSE_CORE_AFFINITY == 1) && (configNUMBER_OF_CORES > 1)
    UBaseType_t uxCoreAffinityMask; /**< Used to link the task to certain cores.  UBaseType_t must have greater than or
                                       equal to the number of bits as configNUMBER_OF_CORES. */
#endif

    ListItem_t xStateListItem; /**< The list that the state list item of a task is reference from denotes the state of
                                  that task (Ready, Blocked, Suspended ). */
    ListItem_t xEventListItem; /**< Used to reference a task from an event list. */
    UBaseType_t uxPriority;    /**< The priority of the task.  0 is the lowest priority. */
    StackType_t *pxStack;      /**< Points to the start of the stack. */
#if (configNUMBER_OF_CORES > 1)
    volatile BaseType_t xTaskRunState; /**< Used to identify the core the task is running on, if the task is running.
                                          Otherwise, identifies the task's state - not running or yielding. */
    UBaseType_t uxTaskAttributes;      /**< Task's attributes - currently used to identify the idle tasks. */
#endif
    char pcTaskName[configMAX_TASK_NAME_LEN]; /**< Descriptive name given to the task when created.  Facilitates
                                                 debugging only. */

#if (configUSE_TASK_PREEMPTION_DISABLE == 1)
    BaseType_t xPreemptionDisable; /**< Used to prevent the task from being preempted. */
#endif

#if ((portSTACK_GROWTH > 0) || (configRECORD_STACK_HIGH_ADDRESS == 1))
    StackType_t *pxEndOfStack; /**< Points to the highest valid address for the stack. */
#endif

#if (portCRITICAL_NESTING_IN_TCB == 1)
    UBaseType_t uxCriticalNesting; /**< Holds the critical section nesting depth for ports that do not maintain their
                                      own count in the port layer. */
#endif

#if (configUSE_TRACE_FACILITY == 1)
    UBaseType_t uxTCBNumber;  /**< Stores a number that increments each time a TCB is created.  It allows debuggers to
                                 determine when a task has been deleted and then recreated. */
    UBaseType_t uxTaskNumber; /**< Stores a number specifically for use by third party trace code. */
#endif

#if (configUSE_MUTEXES == 1)
    UBaseType_t
        uxBasePriority; /**< The priority last assigned to the task - used by the priority inheritance mechanism. */
    UBaseType_t uxMutexesHeld;
#endif

#if (configUSE_APPLICATION_TASK_TAG == 1)
    TaskHookFunction_t pxTaskTag;
#endif

#if (configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0)
    void *pvThreadLocalStoragePointers[configNUM_THREAD_LOCAL_STORAGE_POINTERS];
#endif

#if (configGENERATE_RUN_TIME_STATS == 1)
    configRUN_TIME_COUNTER_TYPE
        ulRunTimeCounter; /**< Stores the amount of time the task has spent in the Running state. */
#endif

#if (configUSE_C_RUNTIME_TLS_SUPPORT == 1)
    configTLS_BLOCK_TYPE xTLSBlock; /**< Memory block used as Thread Local Storage (TLS) Block for the task. */
#endif

#if (configUSE_TASK_NOTIFICATIONS == 1)
    volatile uint32_t ulNotifiedValue[configTASK_NOTIFICATION_ARRAY_ENTRIES];
    volatile uint8_t ucNotifyState[configTASK_NOTIFICATION_ARRAY_ENTRIES];
#endif

    /* See the comments in FreeRTOS.h with the definition of
     * tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE. */
#if (tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE != 0)
    uint8_t ucStaticallyAllocated; /**< Set to pdTRUE if the task is a statically allocated to ensure no attempt is made
                                      to free the memory. */
#endif

#if (INCLUDE_xTaskAbortDelay == 1)
    uint8_t ucDelayAborted;
#endif

#if (configUSE_POSIX_ERRNO == 1)
    int iTaskErrno;
#endif
} free_rtos_TCB;

typedef struct _freeRtos_Task_Info
{
    void *taskTcb;      /* Store the address of TCB. */
    uint32_t stackSize; /* The size of stack, in unit of byte. */
} freeRtos_Task_Info_t;

static freeRtos_Task_Info_t g_freeRTOS_tcbs[CONFIG_GEN_HAL_OSA_FREERTOS_MAX_TASK];

static bool find_slot(uint8_t *idx, void *desiredTcb)
{
    for (uint8_t i = 0; i < CONFIG_GEN_HAL_OSA_FREERTOS_MAX_TASK; i++)
    {
        if (g_freeRTOS_tcbs[i].taskTcb == desiredTcb)
        {
            *idx = i;
            return true;
        }
    }

    return false;
}

static void OSA_TranslateTCBToZephyrThread(struct k_thread *zephyrThread, freeRtos_Task_Info_t *freeRTosTaskInfo)
{
    free_rtos_TCB *freeRTosTcb = freeRTosTaskInfo->taskTcb;
    TaskStatus_t curTaskStatus;
#if defined(CONFIG_THREAD_STACK_INFO)
    zephyrThread->stack_info.start = (uint32_t)freeRTosTcb->pxStack;
    zephyrThread->stack_info.size  = freeRTosTaskInfo->stackSize;
    zephyrThread->stack_info.delta = 0U;
#endif
    zephyrThread->base.prio = freeRTosTcb->uxPriority;

    vTaskGetInfo((TaskHandle_t)freeRTosTcb, &curTaskStatus, pdFALSE, eInvalid);

    zephyrThread->base.thread_state = (uint8_t)curTaskStatus.eCurrentState;

#if defined(CONFIG_THREAD_NAME)
    memcpy(zephyrThread->name, freeRTosTcb->pcTaskName, CONFIG_THREAD_MAX_NAME_LEN);
#endif /* defined(CONFIG_THREAD_NAME) */

    zephyrThread->callee_saved.psp = (uint32_t)freeRTosTcb->pxTopOfStack + 40UL;

#if ((configENABLE_FPU == 1) || (configENABLE_MVE == 1)) && (CONFIG_FPU)
    zephyrThread->callee_saved.psp += 64UL;
#endif
}

void coredump_freertos_trace_task_create(void *tcb)
{
    uint8_t idx = 0U;

    bool isFound = find_slot(&idx, 0);

    if (isFound)
    {
        g_freeRTOS_tcbs[idx].taskTcb = tcb;
        g_freeRTOS_tcbs[idx].stackSize =
            ((uint32_t)(((free_rtos_TCB *)tcb)->pxTopOfStack) - (uint32_t)(((free_rtos_TCB *)tcb)->pxStack)) +
            (18UL * 4UL);
    }
    else
    {
        /* Task registry full. */
        assert(false);
    }
}

void coredump_freertos_trace_task_delete(void *tcb)
{
    uint8_t idx = 0U;

    if (find_slot(&idx, tcb) == false)
    {
        return;
    }

    g_freeRTOS_tcbs[idx].taskTcb   = NULL;
    g_freeRTOS_tcbs[idx].stackSize = 0UL;
}

struct k_thread *OSA_GetCurrentThread(void)
{
    free_rtos_TCB *ptrFreeRtosTCB = (free_rtos_TCB *)(xTaskGetCurrentTaskHandle());
    uint32_t curTaskId            = 0UL;
    for (uint8_t i = 0U; i < CONFIG_GEN_HAL_OSA_FREERTOS_MAX_TASK; i++)
    {
        void *tmpTcb = g_freeRTOS_tcbs[i].taskTcb;

        if (tmpTcb == NULL)
        {
            continue;
        }

        if (((void *)ptrFreeRtosTCB) == (tmpTcb))
        {
            curTaskId = i;
            break;
        }
    }

    OSA_TranslateTCBToZephyrThread(ptrCurrent, &g_freeRTOS_tcbs[curTaskId]);

    return ptrCurrent;
}

uint8_t OSA_GetThreadNum(void)
{
    UBaseType_t taskNum = uxTaskGetNumberOfTasks();

    return (uint8_t)taskNum;
}

void OSA_PopulateKernelInstance(void *ptrThreads, uint32_t taskCount)
{
#if defined(CONFIG_THREAD_MONITOR)
    struct k_thread *current         = NULL;
    struct k_thread *next            = NULL;
    uint32_t taskId                  = 0UL;
    free_rtos_TCB *ptrcurFreeRtosTCB = (free_rtos_TCB *)(xTaskGetCurrentTaskHandle());
    uint32_t curTaskId               = 0UL;

    _kernel.threads = (struct k_thread *)ptrThreads;
    current         = (struct k_thread *)ptrThreads;

    for (uint32_t taskId = 0UL; taskId < taskCount; taskId++)
    {
        memset((void *)current, 0UL, sizeof(struct k_thread));
        if (taskId == (taskCount - 1U))
        {
            current->next_thread = NULL;
            break;
        }
        next                 = current + 1;
        current->next_thread = next;

        current = next;
    }

    /* Restore current and next pointer. */
    current = (struct k_thread *)ptrThreads;
    next    = current->next_thread;

    assert(taskCount <= CONFIG_GEN_HAL_OSA_FREERTOS_MAX_TASK);
    taskId = 0U;
    for (uint8_t i = 0U; i < CONFIG_GEN_HAL_OSA_FREERTOS_MAX_TASK; i++)
    {
        void *tmpTcb = g_freeRTOS_tcbs[i].taskTcb;

        if (tmpTcb == NULL)
        {
            continue;
        }

        if (((void *)ptrcurFreeRtosTCB) == (tmpTcb))
        {
            curTaskId = taskId;
        }

        OSA_TranslateTCBToZephyrThread(current, &(g_freeRTOS_tcbs[i]));

        current = next;
        next    = next->next_thread;
        if (++taskId == taskCount)
        {
            break;
        }
    }

#else
    (void)ptrThreads;
#endif
    /* Populate current thread. */
    OSA_TranslateTCBToZephyrThread(_kernel.cpus[0].current, &(g_freeRTOS_tcbs[curTaskId]));
}

#else
#error "Still not supported"
#endif
