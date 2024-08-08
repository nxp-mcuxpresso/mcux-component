/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SM_RESET_H_
#define SM_RESET_H_

#include "fsl_common.h"
#include "scmi_reset.h"
#include "sm_platform.h"

/*******************************************************************************
 * definition
 ******************************************************************************/
typedef struct
{
    uint32_t channel;
    uint32_t domainId;
    uint32_t flags;
    uint32_t resetState;
} sm_rst_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /*__cplusplus */

int32_t SM_Reset(sm_rst_t *sm_rst);

#if defined(__cplusplus)
}
#endif /*__cplusplus */

/*! @} */

#endif /* SM_RESET_H_ */
