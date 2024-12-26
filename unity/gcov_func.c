/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gcov_support.h"
#include "gcov_func.h"
#include "fsl_debug_console.h"

__attribute__((weak)) void Board_InitNMI(void)
{
}

__attribute__((weak)) void Board_DeInitNMI(void)
{
}

void SystemInitHook(void)
{
    Board_InitNMI();
#if defined(__ICCARM__)
#else
    gcov_init();
#endif
}

void DefaultISR(void)
{
    while (1)
        ;
}

void NMI_Handler(void)
{
    Board_DeInitNMI();

    PRINTF("GCOV START");
#if defined(__ICCARM__)
#else
    gcov_dump_data();
#endif
    PRINTF("GCOV DONE");

    DefaultISR();
}
