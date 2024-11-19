/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Keil: suppress ellipsis warning in va_arg usage below. */
#if defined(__CC_ARM)
#pragma diag_suppress 1256
#endif /* __CC_ARM */

/*! @brief This definition is maximum line that debugconsole can scanf each time.*/
#define IO_MAXLINE 20U
/*! @brief The overflow value.*/
#ifndef HUGE_VAL
#define HUGE_VAL (99.e99)
#endif /* HUGE_VAL */
/*! @brief Specification modifier flags for scanf. */
enum _debugconsole_scanf_flag
{
    kSCANF_Suppress   = 0x2U,    /*!< Suppress Flag. */
    kSCANF_DestMask   = 0x7cU,   /*!< Destination Mask. */
    kSCANF_DestChar   = 0x4U,    /*!< Destination Char Flag. */
    kSCANF_DestString = 0x8U,    /*!< Destination String FLag. */
    kSCANF_DestSet    = 0x10U,   /*!< Destination Set Flag. */
    kSCANF_DestInt    = 0x20U,   /*!< Destination Int Flag. */
    kSCANF_DestFloat  = 0x30U,   /*!< Destination Float Flag. */
    kSCANF_LengthMask = 0x1f00U, /*!< Length Mask Flag. */
#if SCANF_ADVANCED_ENABLE
    kSCANF_LengthChar        = 0x100U, /*!< Length Char Flag. */
    kSCANF_LengthShortInt    = 0x200U, /*!< Length ShortInt Flag. */
    kSCANF_LengthLongInt     = 0x400U, /*!< Length LongInt Flag. */
    kSCANF_LengthLongLongInt = 0x800U, /*!< Length LongLongInt Flag. */
#endif                                 /* SCANF_ADVANCED_ENABLE */
#if SCANF_FLOAT_ENABLE
    kSCANF_LengthLongLongDouble = 0x1000U, /*!< Length LongLongDuoble Flag. */
#endif                                     /*SCANF_FLOAT_ENABLE */
    kSCANF_TypeSinged = 0x2000U,           /*!< TypeSinged Flag. */
};
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if SDK_DEBUGCONSOLE
static int DbgConsole_ScanfFormattedData(const char *line_ptr, char *format, va_list args_ptr);
#endif /* SDK_DEBUGCONSOLE */

/*******************************************************************************
 * Code
 ******************************************************************************/

/*************Code for DbgConsole Init, Deinit, Printf, Scanf *******************************/

#if ((SDK_DEBUGCONSOLE == DEBUGCONSOLE_REDIRECT_TO_SDK) || defined(SDK_DEBUGCONSOLE_UART))
/* See fsl_debug_console.h for documentation of this function. */
status_t DbgConsole_Init(void)
{
    SEGGER_RTT_Init();
    return kStatus_Success;
}

/* See fsl_debug_console.h for documentation of this function. */
status_t DbgConsole_Deinit(void)
{
    return kStatus_Success;
}
#endif /* DEBUGCONSOLE_REDIRECT_TO_SDK */
#if SDK_DEBUGCONSOLE
int DbgConsole_Printf(const char *fmt_s, ...)
{
    va_list ap;
    int result = 0;

    va_start(ap, fmt_s);
    result = SEGGER_RTT_printf(0, fmt_s, ap);
    va_end(ap);

    return result;
}

int DbgConsole_Vprintf(const char *fmt_s, va_list formatStringArg)
{
    return SEGGER_RTT_vprintf(0, fmt_s, &formatStringArg);
}

int DbgConsole_Putchar(int ch)
{
    return (int)SEGGER_RTT_PutChar(0, (char)ch);
}

int DbgConsole_Getchar(void)
{
    return (int)SEGGER_RTT_WaitKey();
}

/* See fsl_debug_console.h for documentation of this function. */
int DbgConsole_Scanf(char *fmt_s, ...)
{
    /* Plus one to store end of string char */
    char temp_buf[IO_MAXLINE + 1];
    va_list ap;
    int32_t i;
    char result;

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
            /* End of Line. */
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
    result = (char)DbgConsole_ScanfFormattedData(temp_buf, fmt_s, ap);
    va_end(ap);

    return (int)result;
}

/*************Code for process formatted data*******************************/
/*!
 * @brief Scanline function which ignores white spaces.
 *
 * @param[in]   s The address of the string pointer to update.
 * @return      String without white spaces.
 */
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

/*!
 * @brief Converts an input line of ASCII characters based upon a provided
 * string format.
 *
 * @param[in] line_ptr The input line of ASCII data.
 * @param[in] format   Format first points to the format string.
 * @param[in] args_ptr The list of parameters.
 *
 * @return Number of input items converted and assigned.
 * @retval IO_EOF When line_ptr is empty string "".
 */
static int DbgConsole_ScanfFormattedData(const char *line_ptr, char *format, va_list args_ptr)
{
    uint8_t base;
    int8_t neg;
    /* Identifier for the format string. */
    char *c = format;
    char temp;
    char *buf;
    /* Flag telling the conversion specification. */
    uint32_t flag = 0;
    /* Filed width for the matching input streams. */
    uint32_t field_width;
    /* How many arguments are assigned except the suppress. */
    uint32_t nassigned = 0;
    bool match_failure = false;
    /* How many characters are read from the input streams. */
    uint32_t n_decode = 0;

    int32_t val;

    const char *s;
    /* Identifier for the input string. */
    const char *p = line_ptr;

#if SCANF_FLOAT_ENABLE
    double fnum = 0.0;
#endif /* SCANF_FLOAT_ENABLE */

    /* Return EOF error before any conversion. */
    if (*p == '\0')
    {
        return -1;
    }

    /* Decode directives. */
    while (('\0' != (*c)) && ('\0' != (*p)))
    {
        /* Ignore all white-spaces in the format strings. */
        if (0U != DbgConsole_ScanIgnoreWhiteSpace((const char **)(void *)&c))
        {
            n_decode += DbgConsole_ScanIgnoreWhiteSpace(&p);
        }
        else if ((*c != '%') || ((*c == '%') && (*(c + 1) == '%')))
        {
            /* Ordinary characters. */
            c++;
            if (*p == *c)
            {
                n_decode++;
                p++;
                c++;
            }
            else
            {
                /* Match failure. Misalignment with C99, the unmatched characters need to be pushed back to stream.
                 * However, it is deserted now. */
                break;
            }
        }
        else
        {
            /* convernsion specification */
            c++;
            /* Reset. */
            flag        = 0;
            field_width = 0;
            base        = 0;

            /* Loop to get full conversion specification. */
            while (('\0' != *c) && (0U == (flag & (uint32_t)kSCANF_DestMask)))
            {
                switch (*c)
                {
#if SCANF_ADVANCED_ENABLE
                    case '*':
                        if (0U != (flag & (uint32_t)kSCANF_Suppress))
                        {
                            /* Match failure. */
                            match_failure = true;
                            break;
                        }
                        flag |= (uint32_t)kSCANF_Suppress;
                        c++;
                        break;
                    case 'h':
                        if (0U != (flag & (uint32_t)kSCANF_LengthMask))
                        {
                            /* Match failure. */
                            match_failure = true;
                            break;
                        }

                        if (c[1] == 'h')
                        {
                            flag |= (uint32_t)kSCANF_LengthChar;
                            c++;
                        }
                        else
                        {
                            flag |= (uint32_t)kSCANF_LengthShortInt;
                        }
                        c++;
                        break;
                    case 'l':
                        if (0U != (flag & (uint32_t)kSCANF_LengthMask))
                        {
                            /* Match failure. */
                            match_failure = true;
                            break;
                        }

                        if (c[1] == 'l')
                        {
                            flag |= (uint32_t)kSCANF_LengthLongLongInt;
                            c++;
                        }
                        else
                        {
                            flag |= (uint32_t)kSCANF_LengthLongInt;
                        }
                        c++;
                        break;
#endif /* SCANF_ADVANCED_ENABLE */
#if SCANF_FLOAT_ENABLE
                    case 'L':
                        if (flag & (uint32_t)kSCANF_LengthMask)
                        {
                            /* Match failure. */
                            match_failure = true;
                            break;
                        }
                        flag |= (uint32_t)kSCANF_LengthLongLongDouble;
                        c++;
                        break;
#endif /* SCANF_FLOAT_ENABLE */
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
                            /* Match failure. */
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
#if SCANF_FLOAT_ENABLE
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'f':
                    case 'F':
                    case 'g':
                    case 'G':
                        flag |= kSCANF_DestFloat;
                        c++;
                        break;
#endif /* SCANF_FLOAT_ENABLE */
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
                        /* Match failure. */
                        match_failure = true;
                        break;
                }

                /* Match failure. */
                if (match_failure)
                {
                    return (int)nassigned;
                }
            }

            if (0U == (flag & (uint32_t)kSCANF_DestMask))
            {
                /* Format strings are exhausted. */
                return (int)nassigned;
            }

            if (0U == field_width)
            {
                /* Large than length of a line. */
                field_width = 99;
            }

            /* Matching strings in input streams and assign to argument. */
            switch (flag & (uint32_t)kSCANF_DestMask)
            {
                case (uint32_t)kSCANF_DestChar:
                    s   = (const char *)p;
                    buf = va_arg(args_ptr, char *);
                    while (((field_width--) > 0U) && ('\0' != *p))
                    {
                        if (0U == (flag & (uint32_t)kSCANF_Suppress))
                        {
                            *buf++ = *p++;
                        }
                        else
                        {
                            p++;
                        }
                        n_decode++;
                    }

                    if ((0U == (flag & (uint32_t)kSCANF_Suppress)) && (s != p))
                    {
                        nassigned++;
                    }
                    break;
                case (uint32_t)kSCANF_DestString:
                    n_decode += DbgConsole_ScanIgnoreWhiteSpace(&p);
                    s   = p;
                    buf = va_arg(args_ptr, char *);
                    while ((field_width-- > 0U) && (*p != '\0') && (*p != ' ') && (*p != '\t') && (*p != '\n') &&
                           (*p != '\r') && (*p != '\v') && (*p != '\f'))
                    {
                        if (0U != (flag & (uint32_t)kSCANF_Suppress))
                        {
                            p++;
                        }
                        else
                        {
                            *buf++ = *p++;
                        }
                        n_decode++;
                    }

                    if ((0U == (flag & (uint32_t)kSCANF_Suppress)) && (s != p))
                    {
                        /* Add NULL to end of string. */
                        *buf = '\0';
                        nassigned++;
                    }
                    break;
                case (uint32_t)kSCANF_DestInt:
                    n_decode += DbgConsole_ScanIgnoreWhiteSpace(&p);
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
                                n_decode += 2U;
                                field_width -= 2U;
                            }
                        }
                    }

                    if (base == 0U)
                    {
                        if (s[0] == '0')
                        {
                            base = 8U;
                        }
                        else
                        {
                            base = 10U;
                        }
                    }

                    neg = 1;
                    switch (*p)
                    {
                        case '-':
                            neg = -1;
                            n_decode++;
                            p++;
                            field_width--;
                            break;
                        case '+':
                            neg = 1;
                            n_decode++;
                            p++;
                            field_width--;
                            break;
                        default:
                            /* MISRA C-2012 Rule 16.4 */
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
                        else
                        {
                            val = (int32_t)base * val + (int32_t)temp;
                        }
                        p++;
                        n_decode++;
                    }
                    val *= neg;
                    if (0U == (flag & (uint32_t)kSCANF_Suppress))
                    {
#if SCANF_ADVANCED_ENABLE
                        switch (flag & (uint32_t)kSCANF_LengthMask)
                        {
                            case (uint32_t)kSCANF_LengthChar:
                                if (0U != (flag & (uint32_t)kSCANF_TypeSinged))
                                {
                                    *va_arg(args_ptr, signed char *) = (signed char)val;
                                }
                                else
                                {
                                    *va_arg(args_ptr, unsigned char *) = (unsigned char)val;
                                }
                                break;
                            case (uint32_t)kSCANF_LengthShortInt:
                                if (0U != (flag & (uint32_t)kSCANF_TypeSinged))
                                {
                                    *va_arg(args_ptr, signed short *) = (signed short)val;
                                }
                                else
                                {
                                    *va_arg(args_ptr, unsigned short *) = (unsigned short)val;
                                }
                                break;
                            case (uint32_t)kSCANF_LengthLongInt:
                                if (0U != (flag & (uint32_t)kSCANF_TypeSinged))
                                {
                                    *va_arg(args_ptr, signed long int *) = (signed long int)val;
                                }
                                else
                                {
                                    *va_arg(args_ptr, unsigned long int *) = (unsigned long int)val;
                                }
                                break;
                            case (uint32_t)kSCANF_LengthLongLongInt:
                                if (0U != (flag & (uint32_t)kSCANF_TypeSinged))
                                {
                                    *va_arg(args_ptr, signed long long int *) = (signed long long int)val;
                                }
                                else
                                {
                                    *va_arg(args_ptr, unsigned long long int *) = (unsigned long long int)val;
                                }
                                break;
                            default:
                                /* The default type is the type int. */
                                if (0U != (flag & (uint32_t)kSCANF_TypeSinged))
                                {
                                    *va_arg(args_ptr, signed int *) = (signed int)val;
                                }
                                else
                                {
                                    *va_arg(args_ptr, unsigned int *) = (unsigned int)val;
                                }
                                break;
                        }
#else
                        /* The default type is the type int. */
                        if (0U != (flag & (uint32_t)kSCANF_TypeSinged))
                        {
                            *va_arg(args_ptr, signed int *) = (signed int)val;
                        }
                        else
                        {
                            *va_arg(args_ptr, unsigned int *) = (unsigned int)val;
                        }
#endif /* SCANF_ADVANCED_ENABLE */
                        nassigned++;
                    }
                    break;
#if SCANF_FLOAT_ENABLE
                case (uint32_t)kSCANF_DestFloat:
                    n_decode += DbgConsole_ScanIgnoreWhiteSpace(&p);
                    fnum = strtod(p, (char **)&s);

                    if ((fnum >= HUGE_VAL) || (fnum <= -HUGE_VAL))
                    {
                        break;
                    }

                    n_decode += (int)(s) - (int)(p);
                    p = s;
                    if (0U == (flag & (uint32_t)kSCANF_Suppress))
                    {
                        if (0U != (flag & (uint32_t)kSCANF_LengthLongLongDouble))
                        {
                            *va_arg(args_ptr, double *) = fnum;
                        }
                        else
                        {
                            *va_arg(args_ptr, float *) = (float)fnum;
                        }
                        nassigned++;
                    }
                    break;
#endif /* SCANF_FLOAT_ENABLE */
                default:
                    /* Match failure. */
                    match_failure = true;
                    break;
            }

            /* Match failure. */
            if (match_failure)
            {
                return (int)nassigned;
            }
        }
    }
    return (int)nassigned;
}

#endif /* SDK_DEBUGCONSOLE */
