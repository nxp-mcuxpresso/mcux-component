/*
 * Copyright 2025-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_mu.h"
#include "fsl_component_lce.h"

static OSA_MUTEX_HANDLE_DEFINE(mCeMutexId);
static OSA_EVENT_HANDLE_DEFINE(mCeEvent);

/*!
 * @brief Initializes the variables used in this component and enable the DSP
 * and MU interrupts.
 *
 * @retval KOSA_StatusError Failed to initialize the mutex handle or event handle used by LCE component.
 * @retval KOSA_StatusSuccess Initialize the mutex handle and event handle used by LCE component successfully.
 */
osa_status_t LCE_Init(void)
{
    /* Create CE Mutex */
    osa_status_t osaStatus;
    osaStatus = OSA_MutexCreate((osa_mutex_handle_t)mCeMutexId);
    if (KOSA_StatusSuccess != osaStatus)
    {
        assert(0);
        return osaStatus;
    }

    /* Create CE event */
    osaStatus = OSA_EventCreate((osa_event_handle_t)mCeEvent, 1U);
    if (KOSA_StatusSuccess != osaStatus)
    {
        assert(0);
        return osaStatus;
    }

#if (defined(KW47_core0_SERIES) || defined(MCXW72_core0_SERIES))
    /* Enable MU interrupt */
    NVIC_SetPriority(DSP_IRQn, CE_ISR_PRIORITY);
    (void)EnableIRQ(DSP_IRQn);
    MU_EnableInterrupts(MUA, (uint32_t)kMU_GenInt0InterruptEnable);
#elif (defined(KW43_core0_SERIES) || defined(MCXW70_core0_SERIES))
    /* Enable MU interrupt */
    NVIC_SetPriority(MU1_IRQn, CE_ISR_PRIORITY);
    (void)EnableIRQ(MU1_IRQn);
    MU_EnableInterrupts(MU1_MUA, (uint32_t)kMU_GenInt0InterruptEnable);
#endif
    return osaStatus;
}

/*!
 * brief DSP IRQ handle function.
 *
 * This function handles the DSP IRQ request.
 *
 */
#if (defined(KW47_core0_SERIES) || defined(MCXW72_core0_SERIES))
void DSP_IRQHandler(void)
{
    (void)CE_CmdReset();
    /* Clear CE interrupt */
    MU_ClearStatusFlags(MUA, (uint32_t)kMU_GenInt0Flag);
    /* Signal LCE API caller */
    (void)OSA_EventSet(mCeEvent, gCeEvtCmdDone_c);
}
#elif (defined(KW43_core0_SERIES) || defined(MCXW70_core0_SERIES))
void MU1_IRQHandler(void)
{
    (void)CE_CmdReset();
    /* Clear CE interrupt */
    MU_ClearStatusFlags(MU1_MUA, (uint32_t)kMU_GenInt0Flag);
    /* Signal LCE API caller */
    (void)OSA_EventSet(mCeEvent, gCeEvtCmdDone_c);
}
#endif

/*
 * Waits for CE completion event, resets the CE command queue, and releases
 * the mutex. Returns 0 on success, 1 on error.
 */
static int32_t lce_api_call_epilogue(void)
{
    osa_event_flags_t event = 0U;
    osa_status_t osa_status = KOSA_StatusSuccess;
    do
    {
        osa_status = OSA_EventWait((osa_event_handle_t)mCeEvent, osaEventFlagsAll_c,
                                   (uint8_t)false, gCeTaskWaitTimeout_ms_c, &event);
        if ((KOSA_StatusSuccess != osa_status) && (KOSA_StatusIdle != osa_status))
        {
            assert(0);
            return 1;
        }
        else if (KOSA_StatusIdle == osa_status)
        {
            NOP32;
        }
        else
        {
            /* osa_status is KOSA_StatusSuccess: event received */
        }
        /* Polling event till timeout for BM-OS */
    } while (KOSA_StatusIdle == osa_status);
    (void)CE_CmdReset();
    if (KOSA_StatusSuccess != LCE_MUTEX_UNLOCK())
    {
        assert(0);
        return 1;
    }
    return 0;
}

/* CE matrix APIs with synchronized access control */
int32_t LCE_MatrixAdd_Q15(int16_t *pDst, int16_t *pA, int16_t *pB, int32_t M, int32_t N)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_MatrixAdd_Q15(pDst, pA, pB, M, N);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_MatrixAdd_Q31(int32_t *pDst, int32_t *pA, int32_t *pB, int32_t M, int32_t N)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_MatrixAdd_Q31(pDst, pA, pB, M, N);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_MatrixAdd_F32(float *pDst, float *pA, float *pB, int32_t M, int32_t N)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_MatrixAdd_F32(pDst, pA, pB, M, N);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_MatrixElemMul_F32(float *pDst, float *pA, float *pB, int32_t M, int32_t N)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_MatrixElemMul_F32(pDst, pA, pB, M, N);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_MatrixMul_F32(float *pDst, float *pA, float *pB, int32_t M, int32_t N, int32_t P)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_MatrixMul_F32(pDst, pA, pB, M, N, P);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_MatrixMul_CF32(float *pDst, float *pA, float *pB, int32_t M, int32_t N, int32_t P)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_MatrixMul_CF32(pDst, pA, pB, M, N, P);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_MatrixInvHerm_CF32(float *pAinv, float *pA, float *pScratch, int32_t M,
                                uint8_t flag_packedInput, uint8_t flag_cholInv)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_MatrixInvHerm_CF32(pAinv, pA, pScratch, M, flag_packedInput, flag_cholInv);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_MatrixEvdHerm_CF32(float *pLambdaOut, float *pUout, float *pUin,
                                float *pScratch, int32_t M, float tol, int32_t max_iter,
                                uint8_t flag_packedInput)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_MatrixEvdHerm_CF32(pLambdaOut, pUout, pUin, pScratch, M, tol,
                                           max_iter, flag_packedInput);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_MatrixChol_CF32(float *pL, float *pA, float *pScratch, int32_t M,
                             uint8_t flag_packedInput)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_MatrixChol_CF32(pL, pA, pScratch, M, flag_packedInput);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

/* CE transform APIs with synchronized access control */
int32_t LCE_TransformCFFT_F16(float *pY, float *pX, float *pScratch, int32_t log2N)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_TransformCFFT_F16(pY, pX, pScratch, log2N);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_TransformCFFT_F32(float *pY, float *pX, float *pScratch, int32_t log2N)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_TransformCFFT_F32(pY, pX, pScratch, log2N);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_TransformIFFT_F16(float *pY, float *pX, float *pScratch, int32_t log2N)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_TransformIFFT_F16(pY, pX, pScratch, log2N);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}

int32_t LCE_TransformIFFT_F32(float *pY, float *pX, float *pScratch, int32_t log2N)
{
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK())
    {
        assert(0);
        return 1;
    }
    int32_t status = CE_TransformIFFT_F32(pY, pX, pScratch, log2N);
    if (lce_api_call_epilogue() != 0)
    {
        return 1;
    }
    return status;
}
