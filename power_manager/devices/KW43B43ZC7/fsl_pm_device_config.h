/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_PM_DEVICE_CONFIG_H_
#define _FSL_PM_DEVICE_CONFIG_H_

#define FSL_PM_SUPPORT_NOTIFICATION          (1U)
#define FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER (1U)
#define FSL_PM_SUPPORT_LP_TIMER_CONTROLLER   (1U)

#define PM_CONSTRAINT_COUNT (13U)
#define PM_LP_STATE_COUNT   (4U)

#define PM_RESC_GROUP_ARRAY_SIZE (2U)
#define PM_RESC_MASK_ARRAY_SIZE  (1U)

/*
 * inputType: 4 bit width. 0 for external pins, 1 for internal wakeup modules.
 * inputId:   8 bit width. WUU input index.
 * irqn:      8 bit width. IRQ number.
 * misc:      12 bit width. Edge selection when inputType is 0;
 *            module event type when inputType is 1.
 *            01b -- External input pin enabled with rising edge detection.
 *            10b -- External input pin enabled with falling edge detection.
 *            11b -- External input pin enabled with any change detection.
 */
#define PM_ENCODE_WAKEUP_SOURCE_ID(inputType, inputId, irqn, misc)                        \
    (((inputType)&0xFUL) | (((inputId) << 4UL) & 0xFF0UL) | (((irqn) << 12UL) & 0xFF000UL) | \
     ((misc << 20UL) & 0xFFF00000UL))

#define PM_DECODE_WAKEUP_SOURCE_ID(wsId)  \
    inputType = ((wsId)&0xFUL);           \
    inputId   = ((wsId)&0xFF0UL) >> 4UL;   \
    irqn      = ((wsId)&0xFF000UL) >> 12UL; \
    misc      = ((wsId)&0xFFF00000UL) >> 20UL

#endif /* _FSL_PM_DEVICE_CONFIG_H_ */
