/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"

#ifndef NDEBUG

#if (defined(__DSC__) && defined(__CW__))

/* This function override __msl_assertion_failed of compiler.
 * It minimises the code size when assert function is used(say in driver).
 */
void __msl_assertion_failed(char const *failedExpr, char const *file, char const *func, int line);
void __msl_assertion_failed(char const *failedExpr, char const *file, char const *func, int line)
{
    asm(DEBUGHLT);
}

#endif /* (defined(__DSC__) && defined (__CW__)) */

#endif /* NDEBUG */
