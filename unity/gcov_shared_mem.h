/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @file gcov_shared_mem.h
 * @brief Shared memory definitions for GCOV coverage data transfer
 *
 * This file defines the shared memory structure and related constants used for
 * transferring GCOV coverage data between cores in a multi-core system. It provides
 * the common definitions used by both gcov_shared_mem_redirect (secondary core) and
 * gcov_shared_mem_reader (primary core) modules.
 */

#ifndef GCOV_SHARED_MEM_H
#define GCOV_SHARED_MEM_H

#include <stdint.h>
#include "gcov_common.h"

/**
 * @brief Size of the data buffer in shared memory for transferring GCOV data
 *
 * This defines the maximum size of a single data transfer fragment. Larger files
 * will be split into multiple transfers of this size.
 */
#define GCOV_SH_MEM_SIZE (4096U)

/**
 * @brief Magic number used to identify valid GCOV shared memory
 *
 * The value 0x47434F56 corresponds to the ASCII string "GCOV".
 */
#define GCOV_SHARED_MEM_MAGIC (0x47434F56)

/**
 * @brief Status flags for the shared memory communication
 *
 * These status values coordinate data transfer between cores by indicating
 * the current state of the shared memory region.
 */
typedef enum
{
    GCOV_STATUS_IDLE = 0,       /* No data being transferred */
    GCOV_STATUS_INIT_DONE,      /* Shared memory is initialized */
    GCOV_STATUS_DATA_READY,     /* Secondary core has written data */
    GCOV_STATUS_DATA_PROCESSED, /* Primary core has processed the data */
    GCOV_STATUS_COMPLETE        /* All files have been transferred */
} gcov_status_t;

/**
 * @brief Shared memory structure for GCOV data transfer between cores
 *
 * This structure is placed in a shared memory region accessible by both the
 * primary and secondary cores. It contains all necessary information for
 * transferring GCOV coverage data between cores, including file metadata
 * and the actual file data.
 */
typedef struct
{
    uint32_t magic;                 /* Magic number (0x47434F56 = "GCOV") */
    volatile uint32_t status;       /* Status flags using gcov_status_t */
    uint32_t file_size;             /* Size of current file data */
    uint32_t is_last_file : 1;      /* Flag to indicate the last file */
    uint32_t offset;                /* Current offset in file (for fragmentation) */
    char filename[GCOV_MAX_PATH];   /* Current file path */
    uint8_t data[GCOV_SH_MEM_SIZE]; /* Raw file data buffer */
} gcov_shared_mem_t;

/**
 * @brief Gets a pointer to the shared memory structure
 *
 * This function returns a pointer to the shared memory structure for GCOV data
 * transfer. The shared memory is typically placed in a special memory section
 * that is accessible by both cores.
 *
 * @return Pointer to the shared memory structure
 */
gcov_shared_mem_t *gcov_get_shared_mem(void);

#endif /* GCOV_SHARED_MEM_H */
