/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2019, 2023, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SLCD_PANEL_H_
#define _SLCD_PANEL_H_

#ifdef _RTE_
#include "RTE_Components.h"
#endif

#ifdef SLCD_PANEL_GDH_1247WP_H
#include "GDH-1247WP.h"
#elif defined(SLCD_PANEL_LCD_S401M16KR_H)
#include "LCD-S401M16KR.h"
#elif defined(SLCD_PANEL_OD_6010_H)
#include "OD-6010.h"
#elif defined(DSLCD_PANEL_FTP12557_H)
#include "FTP12557.h"
#endif

#endif /* _SLCD_PANEL_H_ */
