/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @file gcov_shared_mem_reader.h
 * @brief Reader for GCOV coverage data from shared memory
 *
 * This file provides functionality for the primary core to read GCOV coverage data
 * (.gcda files) that has been written to shared memory by secondary cores. It works
 * in conjunction with the gcov_shared_mem_redirect module used on secondary cores.
 */

#ifndef GCOV_SHARED_MEM_READER_H
#define GCOV_SHARED_MEM_READER_H

#include <stdbool.h>

/**
 * @brief Reads all GCOV data from shared memory and writes to host filesystem
 *
 * This function reads GCOV coverage data from shared memory (written by secondary cores)
 * and writes it to the host filesystem using semihosting. It blocks until all data
 * has been received or a timeout occurs.
 *
 * The function:
 * - Initializes the reader (calls gcov_shared_mem_reader_init())
 * - Processes data in a loop until all files are received or timeout occurs
 * - Creates output files on the host system with the appropriate names
 * - Writes data to these files, handling file fragmentation as needed
 *
 * @return 0 on success, -1 if an error occurred
 *
 * @note This is a blocking function that should be called after the primary core
 *       has completed its own gcov_dump operation. It will wait for data from
 *       secondary cores and process it as it becomes available.
 *
 * @note Files with very long paths may be automatically shortened if the
 *       GCOV_SHORTEN_PATHS option is enabled.
 */
int gcov_shared_mem_read(void);

/**
 * @brief Checks if there is GCOV data waiting to be read from shared memory
 *
 * This function checks the status of the shared memory to determine if
 * there is new GCOV coverage data ready to be processed by the primary core.
 *
 * @return true if data is ready to be read, false otherwise
 *
 * @note This is a non-blocking function that can be used to poll for
 *       data availability before calling processing functions.
 */
bool gcov_shared_mem_has_data(void);

#endif /* GCOV_SHARED_MEM_READER_H */
