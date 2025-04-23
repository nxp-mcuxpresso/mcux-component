/**
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @file gcov_shared_mem_redirect.h
 * @brief Shared memory redirection for GCOV coverage data
 *
 * This file provides functionality to redirect GCOV coverage data (.gcda files)
 * from secondary cores to shared memory, allowing the primary core to read
 * and process this data.
 */

#ifndef GCOV_SHARED_MEM_REDIRECT_H
#define GCOV_SHARED_MEM_REDIRECT_H

#include <stdbool.h>

/**
 * @brief Initializes the shared memory for GCOV data transfer
 *
 * This function initializes the shared memory structure that will be used for
 * transferring GCOV coverage data between cores. It must be called before any
 * other functions in this module.
 *
 * Specifically, this function:
 * - Initializes the current file tracking structure
 * - Sets up the shared memory with the GCOV magic number
 * - Resets the shared memory status to IDLE
 * - Prepares the shared memory for the first data transfer
 *
 * @note This function should be called once during system initialization on
 *       the secondary core before GCOV coverage collection begins.
 */
void gcov_shared_mem_redirect_init(void);

/**
 * @brief Enables or disables GCOV data redirection to shared memory
 *
 * This function controls whether GCOV data is redirected to shared memory.
 * When enabled, standard file I/O operations for .gcda files are intercepted
 * and redirected to shared memory buffers. When disabled, normal file
 * operations proceed without redirection.
 *
 * Enabling redirection causes these system calls to be intercepted:
 * - _open(): For opening .gcda files
 * - _write(): For writing coverage data
 * - _close(): For closing files
 * - _lseek(): For repositioning within files
 * - _fstat(): For obtaining file information
 *
 * @param[in] enable Set to true to enable redirection, false to disable it
 *
 * @note When redirection is enabled, long file paths may be automatically
 *       shortened to fit within GCOV_MAX_PATH limit using the gcov_shorten_path()
 *       function when macro GCOV_SHORTEN_PATHS is set.
 */
void gcov_shared_mem_redirect_enable(bool enable);

/**
 * @brief Notifies the primary core that all GCOV data has been processed
 *
 * This function sends a completion notification to the primary core
 * indicating that all GCOV data has been transferred. It should be called
 * after all coverage data has been processed and no more data will be sent.
 *
 * The function:
 * - Waits for the shared memory to be in a ready state
 * - Sets the is_last_file flag to 1
 * - Updates the shared memory status to DATA_READY
 * - Waits for acknowledgment (status becomes DATA_PROCESSED)
 *
 * @note This function blocks until the primary core acknowledges receipt
 *       of the completion signal. This ensures synchronization between
 *       cores at the end of coverage data collection.
 */
void gcov_shared_mem_notify_complete(void);

#endif /* GCOV_SHARED_MEM_REDIRECT_H */
