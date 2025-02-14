/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_osa_thread.h"

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

struct k_thread *OSA_GetCurrentThread(void)
{
    free_rtos_TCB *ptrFreeRtosTCB = (free_rtos_TCB *)(xTaskGetCurrentTaskHandle());

#if defined(CONFIG_THREAD_NAME)
    memcpy(ptrCurrent->name, ptrFreeRtosTCB->pcTaskName, CONFIG_THREAD_MAX_NAME_LEN);
#endif /* CONFIG_THREAD_NAME */

#if defined(CONFIG_THREAD_STACK_INFO)
    ptrCurrent->stack_info.start = (uint32_t)ptrFreeRtosTCB->pxStack;
    ptrCurrent->stack_info.size  = 0x400; /*TODO: Change as   */
    ptrCurrent->stack_info.delta = 0UL;
#endif                                    /* CONFIG_THREAD_STACK_INFO */

    return ptrCurrent;
}

uint8_t OSA_GetThreadNum(void)
{
    UBaseType_t taskNum = uxTaskGetNumberOfTasks();

    return (uint8_t)taskNum;
}

void OSA_PopulateKernelInstance(struct k_thread *ptrThreads)
{
#if defined(CONFIG_THREAD_MONITOR)
    struct k_thread *current                   = ptrThreads;
    struct k_thread *next                      = ptrThreads;
    volatile UBaseType_t taskNum               = uxTaskGetNumberOfTasks();
    TaskStatus_t *taskStatus                   = pvPortMalloc(sizeof(TaskStatus_t) * taskNum);
    taskNum                                    = uxTaskGetSystemState(taskStatus, taskNum, NULL);
    TaskStatus_t *curTaskStatus                = taskStatus;
    free_rtos_TCB *ptrCurFreeRtosTCB           = NULL;
    volatile StackType_t *curTaskReservedStack = NULL;

    _kernel.threads = ptrThreads;

    for (uint8_t i = 0U; i < (uint8_t)taskNum; i++)
    {
        memset((void *)current, 0U, sizeof(struct k_thread));
        if (i == (taskNum - 1U))
        {
            next = NULL;
        }
        else
        {
            next += 1UL;
        }
        switch (curTaskStatus->eCurrentState)
        {
            case eRunning:
            {
                current->base.thread_state = 0U;
                break;
            }
            case eReady:
            {
                current->base.thread_state = _THREAD_QUEUED;
                break;
            }
            case eBlocked:
            {
                current->base.thread_state = _THREAD_PENDING;
                break;
            }
            case eSuspended:
            {
                current->base.thread_state = _THREAD_SUSPENDED;
                break;
            }
            case eDeleted:
            {
                current->base.thread_state = _THREAD_DEAD;
                break;
            }
            case eInvalid:
            {
                current->base.thread_state = _THREAD_DUMMY;
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }
        current->base.prio   = curTaskStatus->uxCurrentPriority;
        ptrCurFreeRtosTCB    = (free_rtos_TCB *)(curTaskStatus->xHandle);
        curTaskReservedStack = ptrCurFreeRtosTCB->pxTopOfStack;
        // current->entry.pEntry = (k_thread_entry_t)curTaskReservedStack[16];
        // current->entry.parameter1 = (void *)curTaskReservedStack[10];
        // current->entry.parameter2 = NULL;
        // current->entry.parameter3 = NULL;
        current->next_thread      = next;
        current->stack_info.start = (uint32_t)curTaskStatus->pxStackBase;
        current->stack_info.size  = ((uint32_t)curTaskReservedStack - (uint32_t)(curTaskStatus->pxStackBase)) + 0x58UL;
        current->stack_info.delta = 0UL;
        memcpy(current->name, curTaskStatus->pcTaskName, CONFIG_THREAD_MAX_NAME_LEN);

        current++;
        curTaskStatus++;
    }

    vPortFree((void *)taskStatus);
#else
    (void)ptrThreads;
#endif
    /* Populate current thread. */
    _kernel.cpus[0].current = OSA_GetCurrentThread();
}

#else
#error "Still not supported"
#endif
