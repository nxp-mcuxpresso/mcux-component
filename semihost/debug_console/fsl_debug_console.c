/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include "fsl_str.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifndef DEBUG_CONSOLE_PRINTF_MAX_LOG_LEN
#define DEBUG_CONSOLE_PRINTF_MAX_LOG_LEN 128U
#endif

#define IO_MAXLINE 20U

/*! @brief semihosting stderr handle used for console output. */
#define DEBUG_CONSOLE_SEMIHOST_HANDLE 2

/* kSCANF_* flags come from fsl_str.h. */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DbgConsole_PrintCallback(char *buf, int32_t *indicator, char val, int len);
static int DbgConsole_ScanfFormattedData(const char *line_ptr, char *format, va_list args_ptr);

/*******************************************************************************
 * Code
 ******************************************************************************/

/* See fsl_debug_console.h for documentation of these functions. */
status_t DbgConsole_Init(uint8_t instance, uint32_t baudRate, serial_port_type_t device, uint32_t clkSrcFreq)
{
    (void)instance;
    (void)baudRate;
    (void)device;
    (void)clkSrcFreq;
    return kStatus_Success;
}

status_t DbgConsole_Deinit(void)
{
    return kStatus_Success;
}

/* Emit full buffer chunks to the semihosting console while formatting. */
static void DbgConsole_PrintCallback(char *buf, int32_t *indicator, char val, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        if (((uint32_t)*indicator + 1UL) >= (uint32_t)DEBUG_CONSOLE_PRINTF_MAX_LOG_LEN)
        {
            (void)SEMIHOST_Write(DEBUG_CONSOLE_SEMIHOST_HANDLE, buf, *indicator);
            *indicator = 0;
        }
        buf[*indicator] = val;
        (*indicator)++;
    }
}

int DbgConsole_Vprintf(const char *fmt_s, va_list formatStringArg)
{
    char printBuf[DEBUG_CONSOLE_PRINTF_MAX_LOG_LEN] = {'\0'};
    int logLength;

    logLength = StrFormatPrintf(fmt_s, formatStringArg, printBuf, DbgConsole_PrintCallback);
    if (logLength > 0)
    {
        (void)SEMIHOST_Write(DEBUG_CONSOLE_SEMIHOST_HANDLE, printBuf, logLength);
    }
    return logLength;
}

int DbgConsole_Printf(const char *fmt_s, ...)
{
    va_list ap;
    int result;

    va_start(ap, fmt_s);
    result = DbgConsole_Vprintf(fmt_s, ap);
    va_end(ap);

    return result;
}

int DbgConsole_Putchar(int ch)
{
    char c = (char)ch;
    return SEMIHOST_Write(DEBUG_CONSOLE_SEMIHOST_HANDLE, &c, 1);
}

int DbgConsole_Getchar(void)
{
    /* SYS_READC returns one character read from the debugger console. */
    return SEMIHOST_HostCall((int)kSemihost_SysReadC, NULL, NULL);
}

int DbgConsole_Scanf(char *fmt_s, ...)
{
    char temp_buf[IO_MAXLINE + 1];
    va_list ap;
    int32_t i;
    char result;
    int nassigned;

    va_start(ap, fmt_s);
    temp_buf[0] = '\0';

    i = 0;
    while (true)
    {
        if (i >= (int32_t)IO_MAXLINE)
        {
            break;
        }

        result      = (char)DbgConsole_Getchar();
        temp_buf[i] = result;

        if ((result == '\r') || (result == '\n'))
        {
            if (i == 0)
            {
                temp_buf[i] = '\0';
                i           = -1;
            }
            else
            {
                break;
            }
        }
        i++;
    }

    if (i == (int32_t)IO_MAXLINE)
    {
        temp_buf[i] = '\0';
    }
    else
    {
        temp_buf[i + 1] = '\0';
    }
    nassigned = DbgConsole_ScanfFormattedData(temp_buf, fmt_s, ap);
    va_end(ap);

    return nassigned;
}

static uint32_t DbgConsole_ScanIgnoreWhiteSpace(const char **s)
{
    uint8_t count = 0;
    char c;

    c = **s;
    while ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\v') || (c == '\f'))
    {
        count++;
        (*s)++;
        c = **s;
    }
    return count;
}

static int DbgConsole_ScanfFormattedData(const char *line_ptr, char *format, va_list args_ptr)
{
    uint8_t base;
    int8_t neg;
    char *c = format;
    char temp;
    char *buf;
    uint32_t flag = 0;
    uint32_t field_width;
    uint32_t nassigned = 0;
    bool match_failure = false;
    int32_t val;
    const char *s;
    const char *p = line_ptr;

    if (*p == '\0')
    {
        return -1;
    }

    while (('\0' != (*c)) && ('\0' != (*p)))
    {
        if (0U != DbgConsole_ScanIgnoreWhiteSpace((const char **)(void *)&c))
        {
            (void)DbgConsole_ScanIgnoreWhiteSpace(&p);
        }
        else if ((*c != '%') || ((*c == '%') && (*(c + 1) == '%')))
        {
            c++;
            if (*p == *c)
            {
                p++;
                c++;
            }
            else
            {
                break;
            }
        }
        else
        {
            c++;
            flag        = 0;
            field_width = 0;
            base        = 0;

            while (('\0' != *c) && (0U == (flag & (uint32_t)kSCANF_DestMask)))
            {
                switch (*c)
                {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        if (0U != field_width)
                        {
                            match_failure = true;
                            break;
                        }
                        do
                        {
                            field_width = field_width * 10U + ((uint32_t)*c - (uint32_t)'0');
                            c++;
                        } while ((*c >= '0') && (*c <= '9'));
                        break;
                    case 'd':
                        base = 10;
                        flag |= (uint32_t)kSCANF_TypeSinged;
                        flag |= (uint32_t)kSCANF_DestInt;
                        c++;
                        break;
                    case 'u':
                        base = 10;
                        flag |= (uint32_t)kSCANF_DestInt;
                        c++;
                        break;
                    case 'o':
                        base = 8;
                        flag |= (uint32_t)kSCANF_DestInt;
                        c++;
                        break;
                    case 'x':
                    case 'X':
                        base = 16;
                        flag |= (uint32_t)kSCANF_DestInt;
                        c++;
                        break;
                    case 'i':
                        base = 0;
                        flag |= (uint32_t)kSCANF_DestInt;
                        c++;
                        break;
                    case 'c':
                        flag |= (uint32_t)kSCANF_DestChar;
                        if (0U == field_width)
                        {
                            field_width = 1;
                        }
                        c++;
                        break;
                    case 's':
                        flag |= (uint32_t)kSCANF_DestString;
                        c++;
                        break;
                    default:
                        match_failure = true;
                        break;
                }

                if (match_failure)
                {
                    return (int)nassigned;
                }
            }

            if (0U == (flag & (uint32_t)kSCANF_DestMask))
            {
                return (int)nassigned;
            }

            if (0U == field_width)
            {
                field_width = 99;
            }

            switch (flag & (uint32_t)kSCANF_DestMask)
            {
                case (uint32_t)kSCANF_DestChar:
                    s   = (const char *)p;
                    buf = va_arg(args_ptr, char *);
                    while (((field_width--) > 0U) && ('\0' != *p))
                    {
                        *buf++ = *p++;
                    }
                    if (s != p)
                    {
                        nassigned++;
                    }
                    break;
                case (uint32_t)kSCANF_DestString:
                    (void)DbgConsole_ScanIgnoreWhiteSpace(&p);
                    s   = p;
                    buf = va_arg(args_ptr, char *);
                    while ((field_width-- > 0U) && (*p != '\0') && (*p != ' ') && (*p != '\t') && (*p != '\n') &&
                           (*p != '\r') && (*p != '\v') && (*p != '\f'))
                    {
                        *buf++ = *p++;
                    }
                    if (s != p)
                    {
                        *buf = '\0';
                        nassigned++;
                    }
                    break;
                case (uint32_t)kSCANF_DestInt:
                    (void)DbgConsole_ScanIgnoreWhiteSpace(&p);
                    s   = p;
                    val = 0;
                    if ((base == 0U) || (base == 16U))
                    {
                        if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X')))
                        {
                            base = 16U;
                            if (field_width >= 1U)
                            {
                                p += 2;
                                field_width -= 2U;
                            }
                        }
                    }
                    if (base == 0U)
                    {
                        base = (s[0] == '0') ? 8U : 10U;
                    }

                    neg = 1;
                    switch (*p)
                    {
                        case '-':
                            neg = -1;
                            p++;
                            field_width--;
                            break;
                        case '+':
                            neg = 1;
                            p++;
                            field_width--;
                            break;
                        default:
                            break;
                    }

                    while ((field_width-- > 0U) && (*p > '\0'))
                    {
                        if ((*p <= '9') && (*p >= '0'))
                        {
                            temp = *p - '0' + (char)0;
                        }
                        else if ((*p <= 'f') && (*p >= 'a'))
                        {
                            temp = *p - 'a' + (char)10;
                        }
                        else if ((*p <= 'F') && (*p >= 'A'))
                        {
                            temp = *p - 'A' + (char)10;
                        }
                        else
                        {
                            temp = (char)base;
                        }

                        if ((uint8_t)temp >= base)
                        {
                            break;
                        }
                        val = (int32_t)base * val + (int32_t)temp;
                        p++;
                    }
                    val *= neg;
                    if (0U != (flag & (uint32_t)kSCANF_TypeSinged))
                    {
                        *va_arg(args_ptr, signed int *) = (signed int)val;
                    }
                    else
                    {
                        *va_arg(args_ptr, unsigned int *) = (unsigned int)val;
                    }
                    nassigned++;
                    break;
                default:
                    match_failure = true;
                    break;
            }

            if (match_failure)
            {
                return (int)nassigned;
            }
        }
    }
    return (int)nassigned;
}

#if defined(__ICCARM__)
/* DLib pulls __write in transitively (e.g. via strtoul/errno) and ships no
 * default, so provide a weak one to link standalone. armgcc/MDK skip this
 * (newlib has a weak _write stub); semihost_console's __write overrides it. */
#include <stddef.h>
#pragma weak __write
size_t __write(int handle, const unsigned char *buffer, size_t size);
size_t __write(int handle, const unsigned char *buffer, size_t size)
{
    (void)handle;
    return (size_t)SEMIHOST_Write(DEBUG_CONSOLE_SEMIHOST_HANDLE, (const char *)buffer, (int)size);
}
#endif
