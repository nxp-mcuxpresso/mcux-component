/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FSL_SEMIHOST_H_
#define FSL_SEMIHOST_H_

#include <stdint.h>

/*!
 * @addtogroup semihost
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief ARM semihosting operation numbers (subset), per ARM DUI 0471. */
typedef enum _semihost_syscall
{
    kSemihost_SysOpen     = 0x01,
    kSemihost_SysClose    = 0x02,
    kSemihost_SysWriteC   = 0x03, /*!< Write a single character. */
    kSemihost_SysWrite0   = 0x04, /*!< Write a NUL-terminated string. */
    kSemihost_SysWrite    = 0x05, /*!< Write block via {handle, ptr, len}. */
    kSemihost_SysRead     = 0x06,
    kSemihost_SysReadC    = 0x07,
    kSemihost_SysIsTty    = 0x09,
    kSemihost_SysSeek     = 0x0A,
    kSemihost_SysFLen     = 0x0C,
    kSemihost_SysRemove   = 0x0E,
    kSemihost_SysRename   = 0x0F,
    kSemihost_SysGetCmdLn = 0x15,
    kSemihost_SysExit     = 0x18,
} semihost_syscall_t;

/*! @brief ARM semihosting extended-exit reason (used with kSemihost_SysExit). */
#define SEMIHOST_EXIT_REASON_APPLICATION_EXIT 0x20026U

/*! @brief ARM semihosting SYS_OPEN file mode (DUI 0471). Mirrors the fopen mode
 * strings ("r", "rb", "r+", ...). Used by SEMIHOST_Open (fsl_semihost.c).
 */
typedef enum _semihost_open_mode
{
    kSemihost_OpenR       = 0,  /*!< "r"   read, must exist. */
    kSemihost_OpenRb      = 1,  /*!< "rb"  read binary, must exist. */
    kSemihost_OpenRPlus   = 2,  /*!< "r+"  read/update, must exist. */
    kSemihost_OpenRbPlus  = 3,  /*!< "rb+" read/update binary, must exist. */
    kSemihost_OpenW       = 4,  /*!< "w"   truncate/create, write. */
    kSemihost_OpenWb      = 5,  /*!< "wb"  truncate/create binary, write. */
    kSemihost_OpenWPlus   = 6,  /*!< "w+"  truncate/create, read/write. */
    kSemihost_OpenWbPlus  = 7,  /*!< "wb+" truncate/create binary, read/write. */
    kSemihost_OpenA       = 8,  /*!< "a"   append/create, write. */
    kSemihost_OpenAb      = 9,  /*!< "ab"  append/create binary, write. */
    kSemihost_OpenAPlus   = 10, /*!< "a+"  append/create, read/write. */
    kSemihost_OpenAbPlus  = 11, /*!< "ab+" append/create binary, read/write. */
} semihost_open_mode_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Issue an ARM semihosting host call (BKPT 0xAB).
 *
 * Not callable from an ISR: BKPT halts the core and a higher-priority IRQ taken
 * mid-call deadlocks. On DHCSR-readable cores (M3/M4/M7, M33/M55/M85) it returns
 * -1 when no debugger is attached instead of faulting; on M0/M0+/M23 (DHCSR not
 * readable) it always traps, so do not ship it enabled there.
 *
 * @param op  Operation number, see semihost_syscall_t.
 * @param p1  Parameter block pointer (per operation). Packed into a 32-bit
 *            register/word by this component; Cortex-M is 32-bit-pointer only,
 *            enforced by this unit's `depends on CPU_CORTEX_M`.
 * @param p2  Unused by most operations.
 * @return Host return value (R0), or -1 if no debugger is attached.
 */
int SEMIHOST_HostCall(int op, void *p1, void *p2);

/*!
 * @brief Write a buffer to the debugger console via SYS_WRITE.
 *
 * @param handle  Target stream handle. Use 2 for stderr or any handle returned
 *                by SYS_OPEN. The console redirect uses 2 unconditionally.
 * @param buffer  Pointer to bytes to write.
 * @param size    Number of bytes.
 * @return Number of bytes actually written; matches @p size on success.
 */
int SEMIHOST_Write(int handle, const char *buffer, int size);

/*!
 * @brief Request the debugger to terminate the application.
 *
 * Issues SYS_EXIT with the extended {reason, exit_status} payload. May return
 * if the debugger ignores the request (some LinkServer configurations do).
 * Callers should fall back to while(1) after this if they require a halt.
 *
 * @param code  Exit status reported to the debugger.
 * @return -1 if the call returns (debugger declined to halt).
 */
int SEMIHOST_Exit(int code);

/*!
 * @brief Flush any bytes buffered by the semihost console redirect.
 *
 * Provided by semihost_console.c; a final line without a trailing '\n' stays
 * buffered, so call this before SEMIHOST_Exit() or a halt. No-op when empty.
 *
 * @note Defined only when MCUX_COMPONENT_utility.semihost_console is enabled.
 */
void SEMIHOST_ConsoleFlush(void);

/*!
 * @name Direct host file I/O
 *
 * Libc-free file access on the debugger host (Zephyr-style), in the base
 * semihost unit. Writes reuse SEMIHOST_Write(); each call inherits the
 * no-debugger DHCSR gate.
 * @{
 */

/*!
 * @brief Open a file on the debugger host machine (SYS_OPEN).
 *
 * @param path  Host file path, relative to the debugger's working directory.
 *              Keep it to 23 characters or fewer: some host debuggers mishandle
 *              longer names over semihosting SYS_OPEN.
 * @param mode  Open mode, a semihost_open_mode_t value.
 * @return Non-negative host file handle on success, or -1 on failure.
 */
int SEMIHOST_Open(const char *path, int mode);

/*!
 * @brief Read from a host file (SYS_READ).
 *
 * @param handle  Handle returned by SEMIHOST_Open.
 * @param buffer  Destination buffer.
 * @param size    Number of bytes to read.
 * @return Number of bytes read (0 at end of file), or -1 on error.
 */
int SEMIHOST_Read(int handle, char *buffer, int size);

/*!
 * @brief Seek to an absolute position in a host file (SYS_SEEK).
 *
 * ARM semihosting seeks are absolute from the start of the file. For an
 * end-relative seek, query the length with SEMIHOST_Flen first.
 *
 * @param handle  Handle returned by SEMIHOST_Open.
 * @param pos     Absolute byte offset from the start of the file.
 * @return 0 on success, or -1 on failure.
 */
int SEMIHOST_Seek(int handle, int pos);

/*!
 * @brief Query the length of a host file in bytes (SYS_FLEN).
 *
 * @param handle  Handle returned by SEMIHOST_Open.
 * @return File length in bytes, or -1 on failure.
 */
int SEMIHOST_Flen(int handle);

/*!
 * @brief Close a host file (SYS_CLOSE).
 *
 * @param handle  Handle returned by SEMIHOST_Open.
 * @return 0 on success, or -1 on failure.
 */
int SEMIHOST_Close(int handle);

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* FSL_SEMIHOST_H_ */
