/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "hal_config.h"
#include "hal_reset.h"
#if SM_RESET
#include "sm_reset.h"
#elif SRC_RESET
#include "fsl_src.h"
#include "fsl_reset.h"
#endif

#if SM_RESET && SRC_RESET
#error "Pls not define them as 1 at the same time in hal_config.h!!!"
#endif

#if SM_RESET
/*!
 * @brief Perform Reset Action
 *
 * @param hal_rst
 */
int32_t HAL_Reset(hal_rst_t *hal_rst)
{
    sm_rst_t sm_rst         = {0};
    status_t status = kStatus_Success;
    int32_t result = SCMI_ERR_SUCCESS;

    if (hal_rst->id >= HAL_RST_PLATFORM_MAX_ID)
    {
        status = kStatus_OutOfRange;
    }

    sm_rst.domainId   = hal_rst->id;
    sm_rst.channel    = SM_PLATFORM_A2P;
    sm_rst.flags      = hal_rst->flags;
    sm_rst.resetState = hal_rst->resetState;

    result = SM_Reset(&sm_rst);
    if (result != SCMI_ERR_SUCCESS)
    {
        status = kStatus_Fail;
    }

    return (int32_t)status;
}

#elif SRC_RESET
/*!
 * @brief Perform Reset Action
 *
 * @param hal_rst
 */
int32_t HAL_Reset(hal_rst_t *hal_rst)
{
    status_t status = kStatus_Success;

    if (hal_rst->id >= HAL_RST_PLATFORM_MAX_ID)
    {
        status = kStatus_OutOfRange;
    }

    if (status == kStatus_Success)
    {
        uint32_t resetType = RST_LINE_CTRL_DEASSERT;
        uint32_t lineIdx = hal_rst->id;
        bool toggle = (HAL_RESET_GET_FLAGS_AUTO(hal_rst->flags) != 0U);
        bool assertNegate = (HAL_RESET_GET_FLAGS_SIGNAL(hal_rst->flags) != 0U);
        if (toggle)
        {
            resetType = RST_LINE_CTRL_TOGGLE;
        }
        else
        {
            if (assertNegate)
            {
                resetType = RST_LINE_CTRL_ASSERT;
            }
        }
        if (!SRC_MixSetResetLine(lineIdx, resetType))
        {
            status = kStatus_Fail;
        }
    }
    return (int32_t)status;
}


#else
#error "Pls define macro SM_RESET or SRC_RESET in hal_config.h.!!!"
#endif
