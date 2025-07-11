/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FTP12557_H_
#define _FTP12557_H_

#include <stdint.h>
#include "fsl_common.h"

/*
 * Change log:
 *
 * 1.0.0:
 *   - Initial version.
 */
#define SLCD_PANEL_SUPPORT_LETTER 1

#define SLCD_ENGINE_PIN_VAL(pin, val) (((pin) << 8U) | ((val) << 0U))

typedef enum
{
    NUM_POS1 = 0,
    NUM_POS2,
    NUM_POS3,
    NUM_POS4,
    NUM_POS5,
    NUM_POS6,
    NUM_POSEND
} tSLCD_Segment_Num;

typedef enum
{
    /* Character part 1 */
    ICON_1A = 0,
    ICON_1B,
    ICON_1C,
    ICON_1D,
    ICON_1E,
    ICON_1F,
    ICON_1G,

    /* Character part 2 */
    ICON_2A,
    ICON_2B,
    ICON_2C,
    ICON_2D,
    ICON_2E,
    ICON_2F,
    ICON_2G,

    /* Character part 3 */
    ICON_3A,
    ICON_3B,
    ICON_3C,
    ICON_3D,
    ICON_3E,
    ICON_3F,
    ICON_3G,

    /* Character part 4 */
    ICON_4A,
    ICON_4B,
    ICON_4C,
    ICON_4D,
    ICON_4E,
    ICON_4F,
    ICON_4G,

    /* Character part 4 */
    ICON_5A,
    ICON_5B,
    ICON_5C,
    ICON_5D,
    ICON_5E,
    ICON_5F,
    ICON_5G,

    /* Character part 4 */
    ICON_6A,
    ICON_6B,
    ICON_6C,
    ICON_6D,
    ICON_6E,
    ICON_6F,
    ICON_6G,

    /* icon part */
    ICON_S1,
    ICON_S2,
    ICON_S3,
    ICON_S4,
    ICON_S5,
    ICON_S6,
    ICON_S7,
    ICON_S8,
    ICON_S9,
    ICON_S10,
    ICON_S11,
    ICON_S12,

    ICON_D1,
    ICON_D2,
    ICON_D3,
    ICON_D4,
    ICON_D5,

    ICON_T1,
    ICON_T2,
    ICON_T3,
    ICON_T4,
    ICON_T5,

    ICON_END
} tSLCD_Segment_Icon;

extern const uint16_t SLCD_Icon[];
extern const uint16_t **SLCD_NumPos[];
extern const uint16_t **SLCD_Pos_Upper_Letters[];
extern const uint16_t **SLCD_Pos_Lower_Letters[];

#endif /* _FTP12557_H_ */
