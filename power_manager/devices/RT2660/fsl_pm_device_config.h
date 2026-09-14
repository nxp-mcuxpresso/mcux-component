/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FSL_PM_DEVICE_CONFIG_H_
#define _FSL_PM_DEVICE_CONFIG_H_

#define FSL_PM_SUPPORT_NOTIFICATION          (1U)
#define FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER (1U)
#define FSL_PM_SUPPORT_LP_TIMER_CONTROLLER   (1U) /*!< VBAT LPTMR is wired as the framework's low-power
                                                        timer -- see PM_RegisterTimerController(). */

#define PM_CONSTRAINT_COUNT (43)
#define PM_LP_STATE_COUNT   (5)

#endif /* _FSL_PM_DEVICE_CONFIG_H_ */
