/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_semihost.h"

#include <stddef.h>
#include <string.h>

/* ARMv7-M (M3/M4/M7), ARMv8-M Mainline (M33/M55), and ARMv8.1-M Mainline (M85)
 * expose DHCSR at 0xE000EDF0; bit 0 (C_DEBUGEN) tells software whether a
 * debugger is attached. ARMv6-M (M0/M0+) and ARMv8-M Baseline (M23) do not
 * give software access. */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_8M_MAIN__) || \
    defined(__ARM_ARCH_8_1M_MAIN__)
#define SEMIHOST_HAS_DHCSR_ACCESS 1
#else
#define SEMIHOST_HAS_DHCSR_ACCESS 0
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

static int semihost_debugger_attached(void)
{
#if SEMIHOST_HAS_DHCSR_ACCESS
    /* DHCSR.C_DEBUGEN - fixed address on all M3+ / M33+ cores. */
    return ((*(volatile uint32_t *)0xE000EDF0u) & 0x1u) != 0;
#else
    return 1;
#endif
}

int SEMIHOST_HostCall(int op, void *p1, void *p2)
{
    if (!semihost_debugger_attached())
    {
        return -1;
    }

    int rv;

    /* armclang predefines __GNUC__, so __ARMCC_VERSION must be tested first. */
#if defined(__ICCARM__)
    /* iccarm rejects hard-pinned register vars (register int __asm("r0")) at
     * -On, so move the operands into r0-r2 inside the asm just before BKPT. */
    __asm volatile(
        "MOV r0, %[op]\n"
        "MOV r1, %[p1]\n"
        "MOV r2, %[p2]\n"
        "BKPT #0xAB\n"
        "MOV %[rv], r0\n"
        : [rv] "=r"(rv)
        : [op] "r"(op), [p1] "r"(p1), [p2] "r"(p2)
        : "r0", "r1", "r2", "memory");
#elif defined(__ARMCC_VERSION)
    /* armclang (MDK 6.x / Keil) - same extended asm syntax as GCC. Must precede
     * the __GNUC__ branch (armclang defines __GNUC__ too). */
    register int r0 __asm("r0") = op;
    register int r1 __asm("r1") = (int)p1;
    register int r2 __asm("r2") = (int)p2;
    __asm volatile(
        "bkpt #0xAB"
        : "+r"(r0)
        : "r"(r1), "r"(r2)
        : "memory");
    rv = r0;
#elif defined(__GNUC__)
    register int r0 __asm("r0") = op;
    register int r1 __asm("r1") = (int)p1;
    register int r2 __asm("r2") = (int)p2;
    __asm volatile(
        "bkpt 0xAB"
        : "+r"(r0)
        : "r"(r1), "r"(r2)
        : "memory");
    rv = r0;
#else
#error "SEMIHOST_HostCall: unsupported toolchain"
#endif

    return rv;
}

int SEMIHOST_Write(int handle, const char *buffer, int size)
{
    if (size < 0)
    {
        return 0;
    }

    uint32_t block[3];
    block[0] = (uint32_t)handle;
    block[1] = (uint32_t)buffer;
    block[2] = (uint32_t)size;

    /* SYS_WRITE returns the number of bytes NOT written (0 on full success,
     * or the original count if the host refused / no debugger is attached). */
    int not_written = SEMIHOST_HostCall((int)kSemihost_SysWrite, block, NULL);

    if (not_written < 0 || not_written > size)
    {
        return 0;
    }

    return size - not_written;
}

int SEMIHOST_Exit(int code)
{
    /* Extended SYS_EXIT form (ARM v2): pass {reason, exit_status} via R1. */
    uint32_t block[2];
    block[0] = SEMIHOST_EXIT_REASON_APPLICATION_EXIT;
    block[1] = (uint32_t)code;

    (void)SEMIHOST_HostCall((int)kSemihost_SysExit, block, NULL);

    return -1;
}

/*******************************************************************************
 * Direct host file I/O - libc-free, toolchain-independent (Zephyr's model).
 * Writes reuse SEMIHOST_Write() above.
 ******************************************************************************/

int SEMIHOST_Open(const char *path, int mode)
{
    uint32_t block[3];
    block[0] = (uint32_t)path;
    block[1] = (uint32_t)mode;
    block[2] = (uint32_t)strlen(path);

    /* SYS_OPEN returns a non-negative handle, or -1 on failure. */
    return SEMIHOST_HostCall((int)kSemihost_SysOpen, block, NULL);
}

int SEMIHOST_Read(int handle, char *buffer, int size)
{
    if (size < 0)
    {
        return -1;
    }

    uint32_t block[3];
    block[0] = (uint32_t)handle;
    block[1] = (uint32_t)buffer;
    block[2] = (uint32_t)size;

    /* SYS_READ returns the number of bytes NOT read (0 = full read, == size =
     * nothing read / EOF); translate to a bytes-read return. */
    int not_read = SEMIHOST_HostCall((int)kSemihost_SysRead, block, NULL);

    if (not_read < 0 || not_read > size)
    {
        return -1;
    }

    return size - not_read;
}

int SEMIHOST_Seek(int handle, int pos)
{
    uint32_t block[2];
    block[0] = (uint32_t)handle;
    block[1] = (uint32_t)pos;

    /* SYS_SEEK is absolute from file start; returns 0 on success, nonzero on
     * failure. */
    int status = SEMIHOST_HostCall((int)kSemihost_SysSeek, block, NULL);

    return (status == 0) ? 0 : -1;
}

int SEMIHOST_Flen(int handle)
{
    uint32_t block[1];
    block[0] = (uint32_t)handle;

    /* SYS_FLEN returns the file length in bytes, or -1 on failure. */
    return SEMIHOST_HostCall((int)kSemihost_SysFLen, block, NULL);
}

int SEMIHOST_Close(int handle)
{
    uint32_t block[1];
    block[0] = (uint32_t)handle;

    /* SYS_CLOSE returns 0 on success, nonzero on failure. */
    int status = SEMIHOST_HostCall((int)kSemihost_SysClose, block, NULL);

    return (status == 0) ? 0 : -1;
}
