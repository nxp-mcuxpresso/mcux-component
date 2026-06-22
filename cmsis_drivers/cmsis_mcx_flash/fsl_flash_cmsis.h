/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * CMSIS Flash driver declarations for NXP MCX series based on ROM API
 *
 * Driver_Flash0 -> Main Flash
 * Driver_Flash1 -> IFR0
 */

#ifndef _FSL_FLASH_CMSIS_H_
#define _FSL_FLASH_CMSIS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Driver_Flash.h"

/* CMSIS Flash driver instance for Main PFlash */
extern ARM_DRIVER_FLASH Driver_Flash0;

/* CMSIS Flash driver instance for IFR0 */
extern ARM_DRIVER_FLASH Driver_Flash1;

#ifdef __cplusplus
}
#endif

#endif /* _FSL_FLASH_CMSIS_H_ */

