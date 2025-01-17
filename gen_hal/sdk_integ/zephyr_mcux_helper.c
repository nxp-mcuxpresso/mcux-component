/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "zephyr/toolchain.h"

__weak bool k_is_in_isr(void)
{
  return (__get_IPSR()) ? (true) : (false);
}

unsigned int arch_irq_lock(void)
{
    return DisableGlobalIRQ();
}

void arch_irq_unlock(unsigned int key)
{
    EnableGlobalIRQ(key);
}
