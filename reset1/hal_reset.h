/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HAL_RESET_H_
#define HAL_RESET_H_

#include "hal_reset_platform.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Autonomous Reset action */
#define HAL_RESET_GET_FLAGS_AUTO(x) (((x) & 0x1U) >> 0U)
/* Explicit signal */
#define HAL_RESET_GET_FLAGS_SIGNAL(x) (((x) & 0x2U) >> 1U)
#define HAL_RESET_SET_FLAGS_AUTO(x) (((x) & 0x1U) << 0U)
#define HAL_RESET_SET_FLAGS_SIGNAL(x) (((x) & 0x1U) << 1U)

typedef struct
{
    uint32_t id;
    uint32_t flags;
    uint32_t resetState; /* The reset state being requested */
} hal_rst_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /*__cplusplus */

int32_t HAL_Reset(hal_rst_t *hal_rst);

#if defined(__cplusplus)
}
#endif /*__cplusplus */

/*! @} */

#endif /* HAL_RESET_H_ */
