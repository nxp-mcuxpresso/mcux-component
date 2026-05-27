/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "fsl_semihost.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifndef SEMIHOST_CONSOLE_BUF_SIZE
#define SEMIHOST_CONSOLE_BUF_SIZE 128U
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

static char s_console_buffer[SEMIHOST_CONSOLE_BUF_SIZE];
static unsigned s_console_idx    = 0U;
static int s_console_last_handle = -1;

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Flush a trailing newline-less line before SEMIHOST_Exit or a halt. */
void SEMIHOST_ConsoleFlush(void)
{
    if (s_console_idx > 0U)
    {
        (void)SEMIHOST_Write(s_console_last_handle, s_console_buffer, (int)s_console_idx);
        s_console_idx = 0U;
    }
}

static int semihost_console_buffered_write(int handle, const char *buffer, int size)
{
    int remaining = size;
    const char *p = buffer;

    if (handle != s_console_last_handle)
    {
        SEMIHOST_ConsoleFlush();
        s_console_last_handle = handle;
    }

    while (remaining > 0)
    {
        unsigned room   = SEMIHOST_CONSOLE_BUF_SIZE - s_console_idx;
        unsigned chunk  = ((unsigned)remaining < room) ? (unsigned)remaining : room;
        int saw_newline = 0;
        for (unsigned i = 0; i < chunk; ++i)
        {
            char c                              = p[i];
            s_console_buffer[s_console_idx + i] = c;
            if (c == '\n')
            {
                saw_newline = 1;
            }
        }
        s_console_idx += chunk;
        p += chunk;
        remaining -= (int)chunk;

        /* Line-buffered: flush on newline so a reset/exit can't strand a line. */
        if (s_console_idx == SEMIHOST_CONSOLE_BUF_SIZE || saw_newline)
        {
            SEMIHOST_ConsoleFlush();
        }
    }

    return size;
}

/* armclang predefines __GNUC__, so its branch must precede __GNUC__. */
#if defined(__ICCARM__)
/* IAR DLib. #pragma weak is the IAR-native weak-symbol idiom. */
#pragma weak __write
size_t __write(int handle, const unsigned char *buffer, size_t size);
size_t __write(int handle, const unsigned char *buffer, size_t size)
{
    return (size_t)semihost_console_buffered_write(handle, (const char *)buffer, (int)size);
}
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
/* armcc/armclang route printf through fputc (both MicroLib and stdlib). */
#pragma weak fputc
int fputc(int ch, FILE *f)
{
    char c = (char)ch;
    (void)f;
    (void)semihost_console_buffered_write(2 /* stderr */, &c, 1);
    return ch;
}
#elif defined(__GNUC__)
/* Strong on purpose: newlib nano.specs ships a non-weak ENOSYS _write stub that
 * a weak override here would lose to. */
int _write(int handle, const char *buffer, int size)
{
    return semihost_console_buffered_write(handle, buffer, size);
}
#else
#error "semihost_console: unsupported toolchain"
#endif
