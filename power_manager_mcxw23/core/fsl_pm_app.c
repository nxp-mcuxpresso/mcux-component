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

#include "fsl_pm_app.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "fsl_debug_console.h"



void PMAPP_Init(void)
{
}

status_t PMAPP_EnterLowPower(uint32_t sleepDuration)
{
    status_t status             = kStatus_Success;
    pm_mode_t sleepMode         = kPM_Mode_Sleep;
    uint64_t expectedWakeUpTime = 0;
    /* The sleep timer is used to measure how long the system was asleep */
    PMDEVICE_StartSleepTimer();

    /* Capture the time at which the systick timer is stopped */
    uint64_t systickStopTimeUs = PMDEVICE_GetSleepTimer();

    /* Calculate the expected wake up time */
    expectedWakeUpTime = systickStopTimeUs + sleepDuration;

    /* Now enter low power mode till the first event, being OS or radio. */
    status = PM_EnterLowPower(expectedWakeUpTime, &sleepMode);
    assert_equal(status, kStatus_Success);

    return status;
}
