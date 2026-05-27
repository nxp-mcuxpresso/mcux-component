/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FSL_DEBUGCONSOLE_H_
#define FSL_DEBUGCONSOLE_H_

#include <stdarg.h>
#include "fsl_common.h"
#include "fsl_semihost.h"

/* serial_port_type_t is only needed to match the UART DbgConsole_Init signature
 * so board files build unchanged; the value is ignored. Define it locally
 * (skipped if serial_manager.h is already present) to avoid pulling the UART
 * serial-manager stack. */
#ifndef __SERIAL_MANAGER_H__
typedef enum _serial_port_type
{
    kSerialPort_None = 0U,
    kSerialPort_Uart = 1U,
    kSerialPort_UsbCdc,
    kSerialPort_Swo,
    kSerialPort_Virtual,
    kSerialPort_Rpmsg,
    kSerialPort_UartDma,
    kSerialPort_SpiMaster,
    kSerialPort_SpiSlave,
    kSerialPort_BleWu,
} serial_port_type_t;
#endif

/*!
 * @addtogroup debugconsole_semihost
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEBUGCONSOLE_REDIRECT_TO_TOOLCHAIN 0U /*!< Select toolchain printf and scanf. */
#define DEBUGCONSOLE_REDIRECT_TO_SDK       1U /*!< Select SDK version printf, scanf. */
#define DEBUGCONSOLE_DISABLE               2U /*!< Disable debugconsole function. */

#ifndef SDK_DEBUGCONSOLE
#define SDK_DEBUGCONSOLE DEBUGCONSOLE_REDIRECT_TO_SDK
#endif

#if SDK_DEBUGCONSOLE == DEBUGCONSOLE_DISABLE
static inline int DbgConsole_Disabled(void)
{
    return -1;
}
#define PRINTF(...)  DbgConsole_Disabled()
#define SCANF(...)   DbgConsole_Disabled()
#define PUTCHAR(...) DbgConsole_Disabled()
#define GETCHAR()    DbgConsole_Disabled()
#else
/* PRINTF is a function (DbgConsole_Printf), not the toolchain printf, so
 * console redirection to semihosting does not enable -Wformat checking. */
#define PRINTF  DbgConsole_Printf
#define SCANF   DbgConsole_Scanf
#define PUTCHAR DbgConsole_Putchar
#define GETCHAR DbgConsole_Getchar
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the semihosting debug console.
 *
 * Signature matches the UART debug console so board files build unchanged; the
 * UART arguments are ignored (semihosting needs no peripheral).
 *
 * @retval kStatus_Success Always.
 */
status_t DbgConsole_Init(uint8_t instance, uint32_t baudRate, serial_port_type_t device, uint32_t clkSrcFreq);

/*! @brief De-initializes the semihosting debug console. */
status_t DbgConsole_Deinit(void);

/*! @brief Writes formatted output to the debugger semihosting console. */
int DbgConsole_Printf(const char *fmt_s, ...);

/*! @brief Writes formatted output using a va_list. */
int DbgConsole_Vprintf(const char *fmt_s, va_list formatStringArg);

/*! @brief Writes a character to the debugger semihosting console. */
int DbgConsole_Putchar(int ch);

/*! @brief Reads a character from the debugger (SYS_READC). */
int DbgConsole_Getchar(void);

/*! @brief Reads formatted data from the debugger. */
int DbgConsole_Scanf(char *fmt_s, ...);

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* FSL_DEBUGCONSOLE_H_ */
