/*
 * Copyright 2025 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_adapter_rng.h"
#include "sss_crypto.h"
#include "fsl_sss_sscp.h"

/* Variables used by SSSAPI */
extern sss_sscp_session_t g_sssSession;
static sss_sscp_rng_t context_rng    = {0u};

hal_rng_status_t HAL_RngInit(void)
{
    if ((CRYPTO_InitHardware()) != kStatus_Success)
    {
        return kStatus_HAL_RngInternalError;
    }

    if (kStatus_SSS_Success != sss_sscp_rng_context_init(&g_sssSession, &context_rng, 1u))
    {
        return kStatus_HAL_RngInternalError;
    }     
    return kStatus_HAL_RngSuccess;
}

void HAL_RngDeinit(void)
{
    sss_sscp_rng_free(&context_rng);
    return;
}

hal_rng_status_t HAL_RngHwGetData(void *pRandomNo, uint32_t dataSize)
{
    hal_rng_status_t status = kStatus_HAL_RngSuccess;

    if (NULL == pRandomNo)
    {
        return kStatus_HAL_RngNullPointer;
    }

    if (kStatus_SSS_Success != sss_sscp_rng_get_random(&context_rng, pRandomNo, dataSize))
    {
        status = kStatus_HAL_RngInternalError;
    }
    return status;
}

hal_rng_status_t HAL_RngGetData(void *pRandomNo, uint32_t dataSize)
{
    return HAL_RngHwGetData(pRandomNo, dataSize);
}

hal_rng_status_t HAL_RngSetSeed(uint32_t seed)
{
    return KStatus_HAL_RngNotSupport;
}
