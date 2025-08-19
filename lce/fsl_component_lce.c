/*
 * Copyright 2025 NXP
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

    /* Enable MU interrupt */
    NVIC_SetPriority(DSP_IRQn, CE_ISR_PRIORITY);
    (void)EnableIRQ(DSP_IRQn);
    MU_EnableInterrupts(MUA, kMU_GenInt0InterruptEnable);

    return osaStatus;
}

/*!
 * brief DSP IRQ handle function.
 *
 * This function handles the DSP IRQ request.
 *
 */
void DSP_IRQHandler(void)
{
    CE_CmdReset();
    /* Clear CE interrupt */
    MU_ClearStatusFlags(MUA, kMU_GenInt0Flag);
    /* Signal LCE API caller */
    (void)OSA_EventSet(mCeEvent, gCeEvtCmdDone_c);
}

/* CE matrix APIs with synchronized access control */
LCE_API_DEFINE(CE_MatrixAdd_Q15,
               CE_API_ARG_DEF(int16_t *pDst, int16_t *pA, int16_t *pB, int32_t M,
                              int32_t N),
               CE_API_ARG_VAR(pDst, pA, pB, M, N));
LCE_API_DEFINE(CE_MatrixAdd_Q31,
               CE_API_ARG_DEF(int32_t *pDst, int32_t *pA, int32_t *pB, int32_t M,
                              int32_t N),
               CE_API_ARG_VAR(pDst, pA, pB, M, N));
LCE_API_DEFINE(CE_MatrixAdd_F32,
               CE_API_ARG_DEF(float *pDst, float *pA, float *pB, int32_t M, int32_t N),
               CE_API_ARG_VAR(pDst, pA, pB, M, N));
LCE_API_DEFINE(CE_MatrixElemMul_F32,
               CE_API_ARG_DEF(float *pDst, float *pA, float *pB, int32_t M, int32_t N),
               CE_API_ARG_VAR(pDst, pA, pB, M, N));
LCE_API_DEFINE(CE_MatrixMul_F32,
               CE_API_ARG_DEF(float *pDst, float *pA, float *pB, int32_t M, int32_t N,
                              int32_t P),
               CE_API_ARG_VAR(pDst, pA, pB, M, N, P));
LCE_API_DEFINE(CE_MatrixMul_CF32,
               CE_API_ARG_DEF(float *pDst, float *pA, float *pB, int32_t M, int32_t N,
                              int32_t P),
               CE_API_ARG_VAR(pDst, pA, pB, M, N, P));
LCE_API_DEFINE(CE_MatrixInvHerm_CF32,
               CE_API_ARG_DEF(float *pAinv, float *pA, float *pScratch, int32_t M,
                              uint8_t flag_packedInput, uint8_t flag_cholInv),
               CE_API_ARG_VAR(pAinv, pA, pScratch, M, flag_packedInput,
                              flag_cholInv));
LCE_API_DEFINE(CE_MatrixEvdHerm_CF32,
               CE_API_ARG_DEF(float *pLambdaOut, float *pUout, float *pUin,
                              float *pScratch, int32_t M, float tol, int32_t max_iter,
                              uint8_t flag_packedInput),
               CE_API_ARG_VAR(pLambdaOut, pUout, pUin, pScratch, M, tol,
                              max_iter, flag_packedInput));
LCE_API_DEFINE(CE_MatrixChol_CF32, CE_API_ARG_DEF(float *pL, float *pA, float *pScratch, int32_t M, uint8_t flag_packedInput),
               CE_API_ARG_VAR(pL, pA, pScratch, M, flag_packedInput));

/* CE transform APIs with synchronized access control */
LCE_API_DEFINE(CE_TransformCFFT_F16,
               CE_API_ARG_DEF(float *pY, float *pX, float *pScratch, int32_t log2N),
               CE_API_ARG_VAR(pY, pX, pScratch, log2N));
LCE_API_DEFINE(CE_TransformCFFT_F32,
               CE_API_ARG_DEF(float *pY, float *pX, float *pScratch, int32_t log2N),
               CE_API_ARG_VAR(pY, pX, pScratch, log2N));
LCE_API_DEFINE(CE_TransformIFFT_F16,
               CE_API_ARG_DEF(float *pY, float *pX, float *pScratch, int32_t log2N),
               CE_API_ARG_VAR(pY, pX, pScratch, log2N));
LCE_API_DEFINE(CE_TransformIFFT_F32,
               CE_API_ARG_DEF(float *pY, float *pX, float *pScratch, int32_t log2N),
               CE_API_ARG_VAR(pY, pX, pScratch, log2N));