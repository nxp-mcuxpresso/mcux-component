/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __FSL_COMPONENT_LCE_H__
#define __FSL_COMPONENT_LCE_H__

#include "fsl_ce_cmd.h"
#include "fsl_ce_matrix.h"
#include "fsl_ce_transform.h"
#include "fsl_common.h"
#include "fsl_os_abstraction.h"
#if defined(SDK_OS_FREE_RTOS)
#include "FreeRTOS.h"
#endif
/*!
 * @addtogroup Lce
 * @{
 */

/*******************************************************************************
 * Public macro
 ******************************************************************************/
/*! @name Driver version */
/*! @{ */
#define FSL_LCE_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0. */
/*! @} */

#ifndef CE_ISR_PRIORITY
#if defined(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#define CE_ISR_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#else
/* The default value 3 is used to support different ARM Core, such as CM0P, CM4,
 * CM7, and CM33, etc. The minimum number of priority bits implemented in the
 * NVIC is 2 on these SOCs. The value of mininum priority is 3 (2^2 - 1). So,
 * the default value is 3.
 */
#define CE_ISR_PRIORITY (3U)
#endif /* defined(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) */
#endif /* CE_ISR_PRIORITY */

#define gCeMutexWaitTimeout_ms_c 500U
#define gCeTaskWaitTimeout_ms_c 50U
#define gCeEvtCmdDone_c (1U << 0U)

#define NOP1 __asm("NOP");
#define NOP4 NOP1 NOP1 NOP1 NOP1
#define NOP16 NOP4 NOP4 NOP4 NOP4
#define NOP32 NOP16 NOP16

#define LCE_MUTEX_LOCK()                                                       \
  OSA_MutexLock((osa_mutex_handle_t)mCeMutexId, gCeMutexWaitTimeout_ms_c)
#define LCE_MUTEX_UNLOCK() OSA_MutexUnlock((osa_mutex_handle_t)mCeMutexId)

#define CE_API_ARG_DEF(...) __VA_ARGS__
#define CE_API_ARG_VAR(...) __VA_ARGS__

#define LCE_API_DEFINE(name, args_def, args_var)                               \
int L##name(args_def) {                                                        \
    if (KOSA_StatusSuccess != LCE_MUTEX_LOCK()) {                              \
        assert(0);                                                             \
        return 1;                                                              \
    }                                                                          \
    int status = name(args_var);                                               \
    osa_event_flags_t event = 0U;                                              \
    osa_status_t osa_status = KOSA_StatusSuccess;                              \
    do {                                                                       \
        osa_status =                                                           \
        OSA_EventWait((osa_event_handle_t)mCeEvent, osaEventFlagsAll_c,        \
                    (uint8_t) false, gCeTaskWaitTimeout_ms_c, &event);         \
        if ((KOSA_StatusSuccess != osa_status) &&                              \
            (KOSA_StatusIdle != osa_status)) { /* RTOS and BM-OS*/             \
            assert(0);                                                         \
            return 1;                                                          \
        } else if (KOSA_StatusIdle == osa_status) {                            \
            NOP32;                                                             \
        }                                                                      \
        /* Polling event till timeout for BM-OS */                             \
    } while (KOSA_StatusIdle == osa_status);                                   \
    CE_CmdReset();                                                             \
    if (KOSA_StatusSuccess != LCE_MUTEX_UNLOCK()) {                            \
        assert(0);                                                             \
        return 1;                                                              \
    }                                                                          \
    return status;                                                             \
}

#define LCE_API_DECL(name, args_def) int L##name(args_def)

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

void LCE_Init(void);

/* LCE matrix APIs */
LCE_API_DECL(CE_MatrixAdd_Q15, CE_API_ARG_DEF(int16_t *pDst, int16_t *pA,
                                              int16_t *pB, int M, int N));
LCE_API_DECL(CE_MatrixAdd_Q31, CE_API_ARG_DEF(int32_t *pDst, int32_t *pA,
                                              int32_t *pB, int M, int N));
LCE_API_DECL(CE_MatrixAdd_F32,
             CE_API_ARG_DEF(float *pDst, float *pA, float *pB, int M, int N));
LCE_API_DECL(CE_MatrixElemMul_F32,
             CE_API_ARG_DEF(float *pDst, float *pA, float *pB, int M, int N));
LCE_API_DECL(CE_MatrixMul_F32, CE_API_ARG_DEF(float *pDst, float *pA, float *pB,
                                              int M, int N, int P));
LCE_API_DECL(CE_MatrixMul_CF32, CE_API_ARG_DEF(float *pDst, float *pA,
                                               float *pB, int M, int N, int P));
LCE_API_DECL(CE_MatrixInv_F32, CE_API_ARG_DEF(float *pAinv, float *pA, int M));
LCE_API_DECL(CE_MatrixInvSymm_F32,
             CE_API_ARG_DEF(float *pAinv, float *pA, int M));
LCE_API_DECL(CE_MatrixInv_CF32, CE_API_ARG_DEF(float *pAinv, float *pA, int M));
LCE_API_DECL(CE_MatrixInvHerm_CF32,
             CE_API_ARG_DEF(float *pAinv, float *pA, float *pScratch, int M,
                            uint8_t flag_packedInput, uint8_t flag_cholInv));
LCE_API_DECL(CE_MatrixEvdHerm_CF32,
             CE_API_ARG_DEF(float *pLambdaOut, float *pUout, float *pUin,
                            float *pScratch, int M, float tol, int max_iter,
                            uint8_t flag_packedInput));
LCE_API_DECL(CE_MatrixChol_CF32, CE_API_ARG_DEF(float *pL, float *pA, int M));

/* LCE transform APIs */
LCE_API_DECL(CE_TransformCFFT_F16,
             CE_API_ARG_DEF(float *pY, float *pX, float *pScratch, int log2N));
LCE_API_DECL(CE_TransformCFFT_F32,
             CE_API_ARG_DEF(float *pY, float *pX, float *pScratch, int log2N));
LCE_API_DECL(CE_TransformIFFT_F16,
             CE_API_ARG_DEF(float *pY, float *pX, float *pScratch, int log2N));
LCE_API_DECL(CE_TransformIFFT_F32,
             CE_API_ARG_DEF(float *pY, float *pX, float *pScratch, int log2N));

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*! @}*/

#endif /* __FSL_COMPONENT_LCE_H__ */
