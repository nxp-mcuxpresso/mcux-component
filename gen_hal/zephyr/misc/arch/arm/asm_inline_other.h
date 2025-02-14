/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ASM_INLINE_OTHER_H
#define _ASM_INLINE_OTHER_H

#include "fsl_common.h"

static inline unsigned int arch_irq_lock(void)
{
    return DisableGlobalIRQ();
}

static inline void arch_irq_unlock(unsigned int key)
{
    EnableGlobalIRQ(key);
}






#endif /* _ASM_INLINE_OTHER_H */