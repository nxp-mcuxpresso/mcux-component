/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_semihost.h"

/* SYS_OPEN modes (ARM DUI 0471). Always binary: a target has no host-side
 * newline translation, so the text/binary distinction is moot. */
#define SEMIHOST_FOPEN_MODE_RB  1
#define SEMIHOST_FOPEN_MODE_RPB 3
#define SEMIHOST_FOPEN_MODE_WB  5
#define SEMIHOST_FOPEN_MODE_WPB 7
#define SEMIHOST_FOPEN_MODE_AB  9
#define SEMIHOST_FOPEN_MODE_APB 11

/* armclang predefines both __ARMCC_VERSION and __GNUC__, so its branches must
 * precede __GNUC__. Writes reuse the handle-aware _write/__write from
 * semihost_console (selected by this unit) on GCC/IAR; armclang routes file
 * writes through its own _sys_write. */
#if defined(__ICCARM__)

#if ((defined __ICCARM__) && (__VER__ >= 8000000)) || ((defined __ICCRX__) && (__VER__ >= 400))
#include <LowLevelIOInterface.h>
#else
#include <yfuns.h>
#endif
#include <stdio.h> /* SEEK_SET / SEEK_END: __lseek whence uses the ISO C convention */

static int semihost_file_open_mode(int llio_mode)
{
    /* _LLIO_RDONLY/WRONLY/RDWR is a 2-bit field, not independent flags. */
    int rdwr = (llio_mode & _LLIO_RDWRMASK) == _LLIO_RDWR;

    if ((llio_mode & _LLIO_APPEND) != 0)
    {
        return rdwr ? SEMIHOST_FOPEN_MODE_APB : SEMIHOST_FOPEN_MODE_AB;
    }
    if ((llio_mode & (_LLIO_CREAT | _LLIO_TRUNC)) != 0)
    {
        return rdwr ? SEMIHOST_FOPEN_MODE_WPB : SEMIHOST_FOPEN_MODE_WB;
    }
    return rdwr ? SEMIHOST_FOPEN_MODE_RPB : SEMIHOST_FOPEN_MODE_RB;
}

#pragma weak __open
int __open(const char *filename, int mode);
int __open(const char *filename, int mode)
{
    return SEMIHOST_Open(filename, semihost_file_open_mode(mode));
}

#pragma weak __close
int __close(int handle);
int __close(int handle)
{
    /* Flush any buffered console/file data for this handle before the host
     * closes it - otherwise a later write to a different handle would flush
     * these bytes to a handle number the host may have already reused. */
    SEMIHOST_ConsoleFlush();
    return SEMIHOST_Close(handle);
}

#pragma weak __read
size_t __read(int handle, unsigned char *buffer, size_t size);
size_t __read(int handle, unsigned char *buffer, size_t size)
{
    int read = SEMIHOST_Read(handle, (char *)buffer, (int)size);
    return (read < 0) ? _LLIO_ERROR : (size_t)read;
}

#pragma weak __lseek
long __lseek(int handle, long offset, int whence);
long __lseek(int handle, long offset, int whence)
{
    long target = offset;

    if (whence == SEEK_END)
    {
        int length = SEMIHOST_Flen(handle);
        if (length < 0)
        {
            return -1;
        }
        target = (long)length + offset;
    }
    else if (whence != SEEK_SET)
    {
        return -1; /* SEEK_CUR: no SYS_TELL and no per-handle position state */
    }

    return (SEMIHOST_Seek(handle, (int)target) == 0) ? target : -1;
}

#elif (defined(__CC_ARM) || defined(__ARMCC_VERSION)) && !defined(__MICROLIB)

/* ARM Compiler standard library (non-MicroLib). openmode bits follow the
 * rt_sys.h _sys_open convention. Not verified against an installed armclang. */
#define SEMIHOST_ARMCC_OPEN_PLUS 0x01
#define SEMIHOST_ARMCC_OPEN_W    0x02
#define SEMIHOST_ARMCC_OPEN_A    0x04

static int semihost_file_open_mode(int openmode)
{
    int rdwr = (openmode & SEMIHOST_ARMCC_OPEN_PLUS) != 0;

    if ((openmode & SEMIHOST_ARMCC_OPEN_A) != 0)
    {
        return rdwr ? SEMIHOST_FOPEN_MODE_APB : SEMIHOST_FOPEN_MODE_AB;
    }
    if ((openmode & SEMIHOST_ARMCC_OPEN_W) != 0)
    {
        return rdwr ? SEMIHOST_FOPEN_MODE_WPB : SEMIHOST_FOPEN_MODE_WB;
    }
    return rdwr ? SEMIHOST_FOPEN_MODE_RPB : SEMIHOST_FOPEN_MODE_RB;
}

int _sys_open(const char *name, int openmode)
{
    return SEMIHOST_Open(name, semihost_file_open_mode(openmode));
}

int _sys_close(int fh)
{
    return SEMIHOST_Close(fh);
}

/* _sys_read / _sys_write return the count NOT transferred. */
int _sys_read(int fh, unsigned char *buf, unsigned len, int mode)
{
    (void)mode;
    int read = SEMIHOST_Read(fh, (char *)buf, (int)len);
    return (read < 0) ? (int)len : (int)(len - (unsigned)read);
}

int _sys_write(int fh, const unsigned char *buf, unsigned len, int mode)
{
    (void)mode;
    int written = SEMIHOST_Write(fh, (const char *)buf, (int)len);
    return (written < 0) ? (int)len : (int)(len - (unsigned)written);
}

int _sys_seek(int fh, long pos)
{
    return SEMIHOST_Seek(fh, (int)pos);
}

long _sys_flen(int fh)
{
    return (long)SEMIHOST_Flen(fh);
}

int _sys_istty(int fh)
{
    uint32_t block = (uint32_t)fh;
    /* HostCall returns -1 with no debugger attached; the ARM library expects 0/1. */
    return (SEMIHOST_HostCall((int)kSemihost_SysIsTty, &block, NULL) == 1) ? 1 : 0;
}

#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)

/* MicroLib has no file-system layer; nothing to retarget. See semihost.dox. */

#elif defined(__GNUC__)

#include <fcntl.h>
#include <string.h> /* memset */
#include <sys/stat.h>
#include <unistd.h> /* SEEK_SET / SEEK_END */

static int semihost_file_open_mode(int flags)
{
    int rdwr = (flags & O_ACCMODE) == O_RDWR;

    if ((flags & O_APPEND) != 0)
    {
        return rdwr ? SEMIHOST_FOPEN_MODE_APB : SEMIHOST_FOPEN_MODE_AB;
    }
    if ((flags & (O_CREAT | O_TRUNC)) != 0)
    {
        return rdwr ? SEMIHOST_FOPEN_MODE_WPB : SEMIHOST_FOPEN_MODE_WB;
    }
    return rdwr ? SEMIHOST_FOPEN_MODE_RPB : SEMIHOST_FOPEN_MODE_RB;
}

int _open(const char *name, int flags, int mode)
{
    (void)mode;
    return SEMIHOST_Open(name, semihost_file_open_mode(flags));
}

int _close(int handle)
{
    /* Flush any buffered console/file data for this handle before the host
     * closes it - otherwise a later write to a different handle would flush
     * these bytes to a handle number the host may have already reused. */
    SEMIHOST_ConsoleFlush();
    return SEMIHOST_Close(handle);
}

int _read(int handle, char *buffer, int size)
{
    return SEMIHOST_Read(handle, buffer, size);
}

int _lseek(int handle, int offset, int whence)
{
    int target = offset;

    if (whence == SEEK_END)
    {
        int length = SEMIHOST_Flen(handle);
        if (length < 0)
        {
            return -1;
        }
        target = length + offset;
    }
    else if (whence != SEEK_SET)
    {
        return -1; /* SEEK_CUR: no SYS_TELL and no per-handle position state */
    }

    return (SEMIHOST_Seek(handle, target) == 0) ? target : -1;
}

int _fstat(int handle, struct stat *st)
{
    (void)handle;
    /* No metadata beyond length; report a regular file so newlib block-buffers
     * the stream instead of treating it as an unbuffered tty. */
    memset(st, 0, sizeof(*st));
    st->st_mode = S_IFREG;
    return 0;
}

int _isatty(int handle)
{
    uint32_t block = (uint32_t)handle;
    /* HostCall returns -1 with no debugger attached; newlib expects 0/1. */
    return (SEMIHOST_HostCall((int)kSemihost_SysIsTty, &block, NULL) == 1) ? 1 : 0;
}

#else
#error "semihost_file: unsupported toolchain"
#endif
