/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "zephyr/debug/coredump.h"
#if defined(CONFIG_DEBUG_COREDUMP_MEMORY_DUMP_MIN) || defined(CONFIG_DEBUG_COREDUMP_MEMORY_DUMP_THREADS)
#include "fsl_osa_thread.h"
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * This internal function does the following:
 *
 * - Retrieves the exception stack frame
 * - Evaluates whether to report being in a nested exception
 *
 * If the ESF is not successfully retrieved, the function signals
 * an error by returning NULL.
 *
 * @return ESF pointer on success, otherwise return NULL
 */
static inline struct arch_esf *get_esf(uint32_t msp, uint32_t psp, uint32_t exc_return, bool *nested_exc)
{
    bool alternative_state_exc = false;
    struct arch_esf *ptr_esf   = NULL;

    *nested_exc = false;

    if ((exc_return & ((0xFFUL << 24UL))) != (0xFFUL << 24UL))
    {
        /* Invalid EXC_RETURN value. This is a fatal error. */
        return NULL;
    }

    if (!alternative_state_exc)
    {
        if (exc_return & (1UL << 2UL))
        {
            /* Returning to thread mode */
            ptr_esf = (struct arch_esf *)psp;
        }
        else
        {
            /* Returning to handler mode */
            ptr_esf = (struct arch_esf *)msp;
        }
    }

    z_arm_coredump_fault_sp = (uint32_t)ptr_esf;
    return ptr_esf;
}

static inline uint32_t mem_manage_fault(struct arch_esf *ptrEsf, bool callFromHardFault)
{
    uint32_t reason = K_ERR_ARM_MEM_GENERIC;
    if ((SCB->CFSR & SCB_CFSR_MSTKERR_Msk) != 0UL)
    {
        /* Stacking error. */
        reason = K_ERR_ARM_MEM_STACKING;
    }
    if ((SCB->CFSR & SCB_CFSR_MUNSTKERR_Msk) != 0UL)
    {
        /* Unstacking error. */
        reason = K_ERR_ARM_MEM_UNSTACKING;
    }
    if ((SCB->CFSR & SCB_CFSR_DACCVIOL_Msk) != 0UL)
    {
        /* Data access violation. */
        reason = K_ERR_ARM_MEM_DATA_ACCESS;
        if ((SCB->CFSR & SCB_CFSR_MMARVALID_Msk) != 0UL)
        {
            if ((SCB->CFSR & SCB_CFSR_MMARVALID_Msk) != 0UL)
            {
                if (callFromHardFault)
                {
                    /* clear SCB_MMAR[VALID] to reset */
                    SCB->CFSR &= ~SCB_CFSR_MMARVALID_Msk;
                }
            }
        }
    }
    if ((SCB->CFSR & SCB_CFSR_IACCVIOL_Msk) != 0UL)
    {
        reason = K_ERR_ARM_MEM_INSTRUCTION_ACCESS;
    }
#if ((defined(__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && (defined(__FPU_USED) && (__FPU_USED == 1U)))
    if ((SCB->CFSR & SCB_CFSR_MLSPERR_Msk) != 0UL)
    {
        reason = K_ERR_ARM_MEM_FP_LAZY_STATE_PRESERVATION;
        SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTPENDED_Msk;
    }
#endif

    /* clear MMFSR sticky bits */
    SCB->CFSR |= SCB_CFSR_MEMFAULTSR_Msk;

    return reason;
}

static inline uint32_t bus_fault(struct arch_esf *ptrEsf, bool callFromHardFault)
{
    uint32_t reason = K_ERR_ARM_BUS_GENERIC;

    if ((SCB->CFSR & SCB_CFSR_STKERR_Msk) != 0UL)
    {
        reason = K_ERR_ARM_BUS_STACKING;
    }
    if ((SCB->CFSR & SCB_CFSR_UNSTKERR_Msk) != 0UL)
    {
        reason = K_ERR_ARM_BUS_UNSTACKING;
    }
    if ((SCB->CFSR & SCB_CFSR_PRECISERR_Msk) != 0UL)
    {
        reason = K_ERR_ARM_BUS_PRECISE_DATA_BUS;
        if ((SCB->CFSR & SCB_CFSR_BFARVALID_Msk) != 0UL)
        {
            if (callFromHardFault)
            {
                /* clear SCB_CFSR_BFAR[VALID] to reset */
                SCB->CFSR &= ~SCB_CFSR_BFARVALID_Msk;
            }
        }
    }
    if ((SCB->CFSR & SCB_CFSR_IMPRECISERR_Msk) != 0UL)
    {
        reason = K_ERR_ARM_BUS_IMPRECISE_DATA_BUS;
    }
    if ((SCB->CFSR & SCB_CFSR_IBUSERR_Msk) != 0UL)
    {
        reason = K_ERR_ARM_BUS_INSTRUCTION_BUS;
    }
#if ((defined(__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && (defined(__FPU_USED) && (__FPU_USED == 1U)))
    if ((SCB->CFSR & SCB_CFSR_LSPERR_Msk) != 0UL)
    {
        reason = K_ERR_ARM_BUS_FP_LAZY_STATE_PRESERVATION;
    }
#endif
    /* clear BFSR sticky bits */
    SCB->CFSR |= SCB_CFSR_BUSFAULTSR_Msk;

    return reason;
}

static inline uint32_t usage_fault()
{
    uint32_t reason = K_ERR_ARM_USAGE_GENERIC;
    if ((SCB->CFSR & SCB_CFSR_DIVBYZERO_Msk) != 0UL)
    {
        /* An integer divsion by zero error occurred. */
        reason = K_ERR_ARM_USAGE_DIV_0;
    }
    if ((SCB->CFSR & SCB_CFSR_UNALIGNED_Msk) != 0UL)
    {
        /* An unaligned access error has occurred. */
        reason = K_ERR_ARM_USAGE_UNALIGNED_ACCESS;
    }
#if defined(SCB_CFSR_STKOF_Msk)
    if ((SCB->CFSR & SCB_CFSR_STKOF_Msk) != 0UL)
    {
        /* A stack overflow error has occurred. */
        reason = K_ERR_ARM_USAGE_STACK_OVERFLOW;
    }
#endif
    if ((SCB->CFSR & SCB_CFSR_NOCP_Msk) != 0UL)
    {
        /* A coprocessor disabled or not present error has occurred. */
        reason = K_ERR_ARM_USAGE_NO_COPROCESSOR;
    }
    if ((SCB->CFSR & SCB_CFSR_INVPC_Msk) != 0UL)
    {
        /* An integrity check error has occurred. */
        reason = K_ERR_ARM_USAGE_ILLEGAL_EXC_RETURN;
    }
    if ((SCB->CFSR & SCB_CFSR_INVSTATE_Msk) != 0UL)
    {
        /* Illegal use of the EPSR. */
        reason = K_ERR_ARM_USAGE_ILLEGAL_EPSR;
    }
    if ((SCB->CFSR & SCB_CFSR_UNDEFINSTR_Msk) != 0UL)
    {
        /* Attempt to execute undefined instruction. */
        reason = K_ERR_ARM_USAGE_UNDEFINED_INSTRUCTION;
    }
    /* clear UFSR sticky bits */
    SCB->CFSR |= SCB_CFSR_USGFAULTSR_Msk;

    return reason;
}

#if (defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U))
static inline uint32_t secure_fault(void)
{
    uint32_t reason = K_ERR_ARM_SECURE_GENERIC;

    if ((SAU->SFSR & SAU_SFSR_INVEP_Msk) != 0UL)
    {
        reason = K_ERR_ARM_SECURE_ENTRY_POINT;
    }
    else if ((SAU->SFSR & SAU_SFSR_INVIS_Msk) != 0UL)
    {
        reason = K_ERR_ARM_SECURE_INTEGRITY_SIGNATURE;
    }
    else if ((SAU->SFSR & SAU_SFSR_INVER_Msk) != 0UL)
    {
        reason = K_ERR_ARM_SECURE_EXCEPTION_RETURN;
    }
    else if ((SAU->SFSR & SAU_SFSR_AUVIOL_Msk) != 0UL)
    {
        reason = K_ERR_ARM_SECURE_ATTRIBUTION_UNIT;
    }
    else if ((SAU->SFSR & SAU_SFSR_INVTRAN_Msk) != 0UL)
    {
        reason = K_ERR_ARM_SECURE_TRANSITION;
    }
    else if ((SAU->SFSR & SAU_SFSR_LSPERR_Msk) != 0UL)
    {
        reason = K_ERR_ARM_SECURE_LAZY_STATE_PRESERVATION;
    }
    else if ((SAU->SFSR & SAU_SFSR_LSERR_Msk) != 0UL)
    {
        reason = K_ERR_ARM_SECURE_LAZY_STATE_ERROR;
    }

    SAU->SFSR |= 0xFF;

    return reason;
}
#endif

static inline uint32_t hard_fault(struct arch_esf *ptrEsf)
{
    uint32_t reason = K_ERR_CPU_EXCEPTION;

    if ((SCB->HFSR & SCB_HFSR_FORCED_Msk) != 0UL)
    {
        /* IF forced hardfault asserted, need to read other fault status registers to find the cause of the fault. */
        if ((SCB->CFSR & SCB_CFSR_MEMFAULTSR_Msk) != 0UL)
        {
            reason = mem_manage_fault(ptrEsf, true);
        }
        else if ((SCB->CFSR & SCB_CFSR_BUSFAULTSR_Msk) != 0UL)
        {
            reason = bus_fault(ptrEsf, true);
        }
        else if ((SCB->CFSR & SCB_CFSR_USGFAULTSR_Msk) != 0UL)
        {
            reason = usage_fault();
        }
#if (defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U))
        else if (SAU->SFSR != 0UL)
        {
            reason = secure_fault();
        }
#endif
    }

    return reason;
}

static inline uint32_t fault_handle(struct arch_esf *ptrEsf, int fault)
{
    uint32_t reason = K_ERR_CPU_EXCEPTION;

    switch (fault)
    {
        case 3: /* HardFault */
        {
            reason = hard_fault(ptrEsf);
            break;
        }
        case 4: /* MemManage */
        {
            reason = mem_manage_fault(ptrEsf, false);
            break;
        }
        case 5: /* BusFault */
        {
            reason = bus_fault(ptrEsf, false);
            break;
        }
        case 6: /* UsageFault */
        {
            reason = usage_fault();
            break;
        }
#if (defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U))
        case 7: /* SecureFault */
        {
            reason = secure_fault();
            break;
        }
#endif
        default:
        {
            /* Reserved exception. */
            break;
        }
    }

    return reason;
}

void fault_capture(uint32_t msp, uint32_t psp, uint32_t exc_return, _callee_saved_t *callee_regs)
{
    uint32_t reason = K_ERR_CPU_EXCEPTION;
    int fault       = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;
    bool nested_exc;
    struct arch_esf *esf;

    struct arch_esf esf_copy;

    esf = get_esf(msp, psp, exc_return, &nested_exc);
    if (esf == NULL)
    {
        assert(false);
    }

#if defined(CONFIG_ARMV7_M_ARMV8_M_MAINLINE) || defined(CONFIG_ARMV6_M_ARMV8_M_BASELINE)
    /* Gdb expects a stack pointer that does not include the exception stack frame in order to
     * unwind. So adjust the stack pointer accordingly.
     */
    //	z_arm_coredump_fault_sp += sizeof(esf->basic);

#if defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
    /* Assess whether thread had been using the FP registers and add size of additional
     * registers if necessary
     */
    if ((exc_return & EXC_RETURN_STACK_FRAME_TYPE_STANDARD) == EXC_RETURN_STACK_FRAME_TYPE_EXTENDED)
    {
        z_arm_coredump_fault_sp += sizeof(esf->fpu);
    }
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */

#ifndef CONFIG_ARMV8_M_MAINLINE
    if ((esf->basic.xpsr & SCB_CCR_STKALIGN_Msk) == SCB_CCR_STKALIGN_Msk)
    {
        /* Adjust stack alignment after PSR bit[9] detected */
        z_arm_coredump_fault_sp |= 0x4;
    }
#endif /* !CONFIG_ARMV8_M_MAINLINE */
#endif /* CONFIG_ARMV7_M_ARMV8_M_MAINLINE || CONFIG_ARMV6_M_ARMV8_M_BASELINE */

#if !defined(CONFIG_EXTRA_EXCEPTION_INFO)
    memcpy(&esf_copy, esf, sizeof(struct arch_esf));
    ARG_UNUSED(callee_regs);
#else
    /* the extra exception info is not present in the original esf
     * so we only copy the fields before those.
     */
    memcpy(&esf_copy, esf, offsetof(struct arch_esf, extra_info));
    esf_copy.extra_info = (struct __extra_esf_info){.callee = callee_regs, .exc_return = exc_return, .msp = msp};
#endif /* CONFIG_EXTRA_EXCEPTION_INFO */

    if (nested_exc)
    {
        if ((esf_copy.basic.xpsr & IPSR_ISR_Msk) == 0)
        {
            esf_copy.basic.xpsr |= IPSR_ISR_Msk;
        }
    }
    else
    {
        esf_copy.basic.xpsr &= ~(IPSR_ISR_Msk);
    }
    reason = fault_handle(&esf_copy, fault);

#if defined(CONFIG_DEBUG_COREDUMP_MEMORY_DUMP_MIN)
    struct k_thread *curThread = NULL;
    curThread                  = OSA_GetCurrentThread();
    coredump(reason, &esf_copy, curThread);
#elif defined(CONFIG_DEBUG_COREDUMP_MEMORY_DUMP_THREADS)
    struct k_thread *curThread     = NULL;
    curThread                      = OSA_GetCurrentThread();
    uint8_t taskCount              = OSA_GetThreadNum();
    struct k_thread *ptrThreadList = malloc(sizeof(struct k_thread) * taskCount);
    OSA_PopulateKernelInstance(ptrThreadList);
    coredump(reason, &esf_copy, curThread);
    free((void *)ptrThreadList);
#else
    coredump(reason, &esf_copy, NULL);
#endif
}
