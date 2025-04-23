/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gcov_shared_mem_reader.h"
#include "fsl_debug_console.h"
#include "gcov_shared_mem.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Semihosting is already set up in environment for file operations */

/* File currently being processed */
static struct
{
    char filename[GCOV_MAX_PATH];
    int fd;
    bool in_use;
} current_file = {0};

/* Check if shared memory is initialized */
bool gcov_shared_mem_is_initialized(void)
{
    gcov_shared_mem_t *sh_mem = gcov_get_shared_mem();
    return (sh_mem->status == GCOV_STATUS_INIT_DONE || sh_mem->status == GCOV_STATUS_DATA_READY);
}

/* Initialize the reader */
static int gcov_shared_mem_reader_init(void)
{
    /* Initialize current file */
    current_file.in_use = false;
    current_file.fd     = -1;
    memset(current_file.filename, 0, sizeof(current_file.filename));

    /* Initialize shared memory */
    gcov_shared_mem_t *sh_mem = gcov_get_shared_mem();

    bool done = false;

    /* Wait until shared memory is initialized complete */
    while (!done)
    {
        if (gcov_shared_mem_is_initialized())
        {
            done = true;
        }
    }

    /* Check magic number */
    if (sh_mem->magic != GCOV_SHARED_MEM_MAGIC)
    {
        PRINTF("GCOV Shared Memory invalid magic number!\r\n");
        return -1;
    }

    PRINTF("GCOV reader initialized\r\n");

    return 0;
}

/* Check if there's data waiting to be read */
bool gcov_shared_mem_has_data(void)
{
    gcov_shared_mem_t *sh_mem = gcov_get_shared_mem();
    return (sh_mem->status == GCOV_STATUS_DATA_READY);
}

/* Process next available GCOV data (non-blocking) */
static bool gcov_shared_mem_process_next(void)
{
    gcov_shared_mem_t *sh_mem = gcov_get_shared_mem();

    /* Check if data is ready */
    if (sh_mem->status != GCOV_STATUS_DATA_READY)
    {
        return false;
    }

    /* Check for completion signal */
    if (sh_mem->is_last_file)
    {
        PRINTF("GCOV: All files read successfully\r\n");

        /* Close current file if open */
        if (current_file.in_use && current_file.fd >= 0)
        {
            close(current_file.fd);
            current_file.fd     = -1;
            current_file.in_use = false;
        }

        /* Acknowledge completion */
        sh_mem->status = GCOV_STATUS_DATA_PROCESSED;
        return true;
    }

    /* Check if this is a new file or continuation of existing file */
    if (!current_file.in_use || strcmp(current_file.filename, sh_mem->filename) != 0)
    {
        /* Close previous file if any */
        if (current_file.in_use && current_file.fd >= 0)
        {
            close(current_file.fd);
            current_file.fd = -1;
        }

        /* Open the file with semihosting - this will create it on the host PC */
        current_file.fd = open(sh_mem->filename, O_WRONLY | O_CREAT | O_BINARY, 0666);
        strncpy(current_file.filename, sh_mem->filename, GCOV_MAX_PATH - 1);

        if (current_file.fd >= 0)
        {
            current_file.in_use = true;
            //   strncpy(current_file.filename, sh_mem->filename, GCOV_MAX_PATH - 1);
            PRINTF("GCOV: Opened file %s\r\n", current_file.filename);
        }
        else
        {
            PRINTF("GCOV: Failed to open file %s: errno=%d\r\n", current_file.filename, errno);

            /* Signal error but continue */
            sh_mem->status = GCOV_STATUS_DATA_PROCESSED;
            return false;
        }
    }

    /* Seek to the specified offset if needed */
    if (sh_mem->offset > 0)
    {
        if (lseek(current_file.fd, sh_mem->offset, SEEK_SET) < 0)
        {
            PRINTF("GCOV: Seek error in %s: errno=%d\r\n", current_file.filename, errno);
            sh_mem->status = GCOV_STATUS_DATA_PROCESSED;
            return false;
        }
    }

    /* Write data using semihosting */
    int written = write(current_file.fd, sh_mem->data, sh_mem->file_size);

    if (written != sh_mem->file_size)
    {
        PRINTF("GCOV: Error writing to %s: errno=%d\r\n", current_file.filename, errno);
    }
    else
    {
        PRINTF("GCOV: Received %u bytes for %s (offset %u)\r\n", sh_mem->file_size, current_file.filename,
               sh_mem->offset);
    }

    /* Signal data processed */
    sh_mem->status = GCOV_STATUS_DATA_PROCESSED;
    return false; /* Return false to continue processing */
}

/* Read GCOV data from shared memory (blocking) */
int gcov_shared_mem_read(void)
{
    /* Initialize reader */
    if (gcov_shared_mem_reader_init() != 0)
    {
        return -1;
    }

    PRINTF("GCOV: Starting data read.\r\n");

    /* Process data until complete */
    bool done           = false;
    uint32_t idle_count = 0;
    uint32_t max_idle   = 500000; /* Adjust based on your system's speed */

    while (!done)
    {
        /* Process next data if available */
        if (gcov_shared_mem_has_data())
        {
            idle_count = 0;
            done       = gcov_shared_mem_process_next();
        }
        else
        {
            idle_count++;
            /* Exit if no data received for a while */
            if (idle_count > max_idle)
            {
                PRINTF("GCOV: No data received for too long, exiting\r\n");
                done = true;
            }
            /* Brief delay - can be implemented with a simple counter or yield */
            for (volatile int i = 0; i < 100; i++)
            {
                /* Empty loop for delay */
            }
        }
    }

    /* Close any open file */
    if (current_file.in_use && current_file.fd >= 0)
    {
        close(current_file.fd);
        current_file.fd     = -1;
        current_file.in_use = false;
    }

    PRINTF("GCOV: Data read complete\r\n");

    return 0;
}
