/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @file gcov_common.h
 * @brief Common utilities and definitions for GCOV functionality
 *
 * This file provides common definitions and utility functions used by
 * various components of the GCOV coverage collection system. It includes
 * path handling utilities and common constants used throughout the coverage
 * collection implementation.
 */

#ifndef GCOV_COMMON_H
#define GCOV_COMMON_H

#include <stdint.h>
#include <string.h>

/**
 * @brief Maximum length of file paths for GCOV files
 *
 * This constant defines the maximum allowable length for file paths used
 * with GCOV coverage files. Paths longer than this value will be shortened
 * using the gcov_shorten_path() function.
 */
#define GCOV_MAX_PATH (250) /* Maximum file path length */

/**
 * @brief Shortens a file path to fit within a specified maximum length
 *
 * This function shortens a file path by removing parent directories while
 * preserving the basename (filename with extension). It starts removing
 * directories from the end of the path (closest to the basename) and continues
 * until the path fits within the specified maximum length.
 *
 * The function handles both Unix-style paths (with forward slashes) and
 * Windows-style paths (with backslashes).
 *
 * @param[in]  path      The original file path to be shortened
 * @param[out] shortened Buffer to store the shortened path
 * @param[in]  max_len   Maximum allowed length for the shortened path
 *
 * @note If the basename itself exceeds the maximum length, it will be truncated.
 * @note If the original path already fits within max_len, it will be copied
 *       as-is without modification.
 */
void gcov_shorten_path(const char *path, char *shortened, size_t max_len);

#endif /* GCOV_COMMON_H */
