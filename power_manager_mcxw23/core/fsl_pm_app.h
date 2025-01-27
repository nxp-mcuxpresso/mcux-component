/*
 * Copyright 2025 NXP
 * NXP Confidential and Proprietary.
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms. If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#ifndef _FSL_PM_APP_H_
#define _FSL_PM_APP_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
/**
 * @brief Initialize the power manager app component
 *
 */
void PMAPP_Init(void);

/**
 * @brief Enter low power mode for a specified duration.
 *
 * @param sleepDuration sleep duration in microseconds
 * @return status_t
 */
status_t PMAPP_EnterLowPower(uint32_t sleepDuration);

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_PM_APP_H_ */
