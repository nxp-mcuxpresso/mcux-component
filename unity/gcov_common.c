/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "gcov_common.h"

/**
 * Shortens a path by keeping the basename and removing parent directories
 * one by one from the end until the path fits within max_len.
 */
void gcov_shorten_path(const char *path, char *shortened, size_t max_len)
{
    /* If path is already short enough, just copy it */
    if (strlen(path) <= max_len)
    {
        strncpy(shortened, path, max_len);
        shortened[max_len - 1] = '\0';
        return;
    }

    /* Extract the basename (filename with extension) */
    const char *basename_slash     = strrchr(path, '/');
    const char *basename_backslash = strrchr(path, '\\');

    /* Determine which separator is the last one (or if none exist) */
    const char *basename;
    if (basename_slash && basename_backslash)
    {
        basename = (basename_slash > basename_backslash) ? basename_slash + 1 : basename_backslash + 1;
    }
    else if (basename_slash)
    {
        basename = basename_slash + 1;
    }
    else if (basename_backslash)
    {
        basename = basename_backslash + 1;
    }
    else
    {
        basename = path;
    }

    /* If even basename is too long, truncate it (rare case) */
    if (strlen(basename) >= max_len)
    {
        strncpy(shortened, basename, max_len - 1);
        shortened[max_len - 1] = '\0';
        return;
    }

    /* Make a copy of the path to work with */
    char path_copy[GCOV_MAX_PATH * 2]; /* Increased copy buffer size as path is longer */
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    /* Start removing directories from the end */
    char *last_slash     = strrchr(path_copy, '/');
    char *last_backslash = strrchr(path_copy, '\\');
    char *last_separator;

    /* Determine which separator is the last one (or if none exist) */
    if (last_slash && last_backslash)
    {
        last_separator = (last_slash > last_backslash) ? last_slash : last_backslash;
    }
    else if (last_slash)
    {
        last_separator = last_slash;
    }
    else if (last_backslash)
    {
        last_separator = last_backslash;
    }
    else
    {
        /* No separators, just use the basename */
        strcpy(shortened, basename);
        return;
    }

    /* Keep the basename */
    char saved_basename[256];
    strcpy(saved_basename, last_separator + 1);

    /* Remove directories until path fits */
    while (strlen(path_copy) > max_len - strlen(saved_basename) - 1)
    {
        /* Remove last directory (the one immediately before basename) */
        *last_separator = '\0';

        /* Find the previous separator (could be either slash or backslash) */
        last_slash     = strrchr(path_copy, '/');
        last_backslash = strrchr(path_copy, '\\');

        if (last_slash && last_backslash)
        {
            last_separator = (last_slash > last_backslash) ? last_slash : last_backslash;
        }
        else if (last_slash)
        {
            last_separator = last_slash;
        }
        else if (last_backslash)
        {
            last_separator = last_backslash;
        }
        else
        {
            /* No more directories, just use the basename */
            strcpy(shortened, basename);
            return;
        }
    }

    /* Reconstruct the path */
    strcpy(last_separator + 1, basename);
    strcpy(shortened, path_copy);
}
