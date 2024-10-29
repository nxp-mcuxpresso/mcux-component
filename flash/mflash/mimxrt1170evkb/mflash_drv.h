/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MFLASH_DRV_H__
#define __MFLASH_DRV_H__

#include "mflash_common.h"

/* Flash constants */
#ifndef MFLASH_SECTOR_SIZE
#define MFLASH_SECTOR_SIZE (0x1000U)
#endif

#ifndef MFLASH_PAGE_SIZE
#define MFLASH_PAGE_SIZE (256U)
#endif

#ifndef MFLASH_FLEXSPI
#define MFLASH_FLEXSPI (FLEXSPI1)
#endif

#ifndef MFLASH_BASE_ADDRESS
#define MFLASH_BASE_ADDRESS (FlexSPI1_AMBA_BASE)
#endif

/* Flash size expressed in bytes: RT1170 EVKB has 64 MB */
#define MFLASH_BSIZE 0x4000000UL

#define MFLASH_REMAP_OFFSET() (MFLASH_FLEXSPI->HADDROFFSET & FLEXSPI_HADDROFFSET_ADDROFFSET_MASK)
#define MFLASH_REMAP_START()  (MFLASH_FLEXSPI->HADDRSTART & FLEXSPI_HADDRSTART_ADDRSTART_MASK)
#define MFLASH_REMAP_END()    (MFLASH_FLEXSPI->HADDREND & FLEXSPI_HADDREND_ENDSTART_MASK)

#define MFLASH_REMAP_ACTIVE() (MFLASH_REMAP_OFFSET() != 0UL)

#endif
