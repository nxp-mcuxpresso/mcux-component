/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gcov_shared_mem_redirect.h"
#include "gcov_common.h"
#include "gcov_shared_mem.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

static bool redirect_enabled = false;

/* File descriptor tracking */
static struct
{
    bool in_use;
    char filename[GCOV_MAX_PATH];
    uint32_t position;
    uint32_t size;
    int fd; /* Current file descriptor */
} current_file = {0};

void gcov_shared_mem_redirect_init(void)
{
    /* Initialize current file */
    current_file.in_use   = false;
    current_file.position = 0;
    current_file.size     = 0;
    current_file.fd       = -1;

    gcov_shared_mem_t *sh_mem = gcov_get_shared_mem();

    /* Initialize shared memory structure */
    sh_mem->magic        = GCOV_SHARED_MEM_MAGIC;
    sh_mem->is_last_file = 0;
    sh_mem->status       = GCOV_STATUS_INIT_DONE;
}

void gcov_shared_mem_redirect_enable(bool enable)
{
    redirect_enabled = enable;
}

/* Wait for shared memory to be ready for new data */
static void wait_for_shared_mem_ready(void)
{
    gcov_shared_mem_t *sh_mem = gcov_get_shared_mem();

    /* Wait until the primary core signals it has processed previous data */
    while (sh_mem->status != GCOV_STATUS_INIT_DONE && sh_mem->status != GCOV_STATUS_DATA_PROCESSED)
    {
        /* Simple delay or yield */
        for (volatile int i = 0; i < 1000; i++)
        {
        }
    }
}

/* Override open system call */
int _open(const char *name, int flags, int mode)
{
    if (!redirect_enabled)
    {
        /* Let the original implementation handle it */
        return -1;
    }

    /* Only intercept .gcda files */
    if (strstr(name, ".gcda") == NULL)
    {
        return -1;
    }

    /* Wait for any current file processing to complete */
    if (current_file.in_use)
    {
        /* Return error if a file is already being processed */
        errno = EMFILE;
        return -1;
    }

    /* Initialize current file */
    current_file.in_use = true;
#if defined(GCOV_SHORTEN_PATHS) && GCOV_SHORTEN_PATHS == 1
    gcov_shorten_path(name, current_file.filename, GCOV_MAX_PATH - 1);
#else
    strncpy(current_file.filename, name, sizeof(current_file.filename) - 1);
#endif

    current_file.filename[sizeof(current_file.filename) - 1] = '\0'; /* Ensure null termination */
    current_file.position                                    = 0;
    current_file.size                                        = 0;

    gcov_shared_mem_t *sh_mem = gcov_get_shared_mem();

    /* Wait for shared memory to be ready */
    wait_for_shared_mem_ready();

    /* Set file information in shared memory */
    strncpy(sh_mem->filename, current_file.filename, GCOV_MAX_PATH - 1);

    /* Use a static file descriptor (e.g., 100) for all gcov files */
    current_file.fd = 100;

    return current_file.fd;
}

/* Override write system call */
int _write(int file, char *ptr, int len)
{
    if (!redirect_enabled)
    {
        /* Let the original implementation handle it */
        return -1;
    }

    /* Check if this is our managed file */
    if (file != current_file.fd || !current_file.in_use)
    {
        errno = EBADF;
        return -1;
    }

    /* Get shared memory */
    gcov_shared_mem_t *sh_mem = gcov_get_shared_mem();

    /* Wait for shared memory to be ready */
    wait_for_shared_mem_ready();

    /* Process data in fragments if needed */
    uint32_t remaining = len;
    uint32_t offset    = 0;

    while (remaining > 0)
    {
        /* Calculate fragment size */
        uint32_t fragment_size = (remaining > GCOV_SH_MEM_SIZE) ? GCOV_SH_MEM_SIZE : remaining;

        /* Set fragment information */
        sh_mem->file_size    = fragment_size;
        sh_mem->offset       = current_file.position;
        sh_mem->is_last_file = 0; /* Not the last file yet */

        /* Copy data to shared memory */
        memcpy(sh_mem->data, ptr + offset, fragment_size);

        /* Signal data is ready */
        sh_mem->status = GCOV_STATUS_DATA_READY;

        /* Wait for primary core to process */
        while (sh_mem->status != GCOV_STATUS_DATA_PROCESSED)
        {
            /* Simple delay or yield */
            for (volatile int i = 0; i < 1000; i++)
            {
            }
        }

        /* Update position and remaining data */
        current_file.position += fragment_size;
        if (current_file.position > current_file.size)
        {
            current_file.size = current_file.position;
        }

        offset += fragment_size;
        remaining -= fragment_size;
    }

    return len;
}

/* Notify that all GCOV files have been processed */
void gcov_shared_mem_notify_complete(void)
{
    /* Signal completion in shared memory */
    gcov_shared_mem_t *sh_mem = gcov_get_shared_mem();

    /* Wait for shared memory to be ready */
    wait_for_shared_mem_ready();

    /* Send an empty file with is_last_file=1 to signal completion */
    sh_mem->file_size    = 0;
    sh_mem->offset       = 0;
    sh_mem->is_last_file = 1;
    strncpy(sh_mem->filename, "complete", GCOV_MAX_PATH - 1);

    /* Signal data is ready */
    sh_mem->status = GCOV_STATUS_DATA_READY;

    /* Wait for acknowledgment */
    while (sh_mem->status != GCOV_STATUS_DATA_PROCESSED)
    {
        /* Simple delay or yield */
        for (volatile int i = 0; i < 1000; i++)
        {
        }
    }
}

/* Override close system call */
int _close(int file)
{
    if (!redirect_enabled)
    {
        /* Let the original implementation handle it */
        return -1;
    }

    /* Check if this is our managed file */
    if (file != current_file.fd || !current_file.in_use)
    {
        errno = EBADF;
        return -1;
    }

    /* Mark current file as free */
    current_file.in_use = false;

    return 0;
}

/* Override lseek system call */
int _lseek(int file, int offset, int whence)
{
    if (!redirect_enabled)
    {
        /* Let the original implementation handle it */
        return -1;
    }

    /* Check if this is our managed file */
    if (file != current_file.fd || !current_file.in_use)
    {
        errno = EBADF;
        return -1;
    }

    uint32_t new_pos = 0;

    switch (whence)
    {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = current_file.position + offset;
            break;
        case SEEK_END:
            new_pos = current_file.size + offset;
            break;
        default:
            errno = EINVAL;
            return -1;
    }

    current_file.position = new_pos;
    return new_pos;
}

/* Override fstat system call */
int _fstat(int file, struct stat *st)
{
    if (!redirect_enabled)
    {
        /* Let the original implementation handle it */
        return -1;
    }

    /* Check if this is our managed file */
    if (file != current_file.fd || !current_file.in_use)
    {
        errno = EBADF;
        return -1;
    }

    /* Initialize stat structure to indicate regular file */
    memset(st, 0, sizeof(*st));
    st->st_mode = S_IFREG;
    st->st_size = current_file.size;

    return 0;
}
