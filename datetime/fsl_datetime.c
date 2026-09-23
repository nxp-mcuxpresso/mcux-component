/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_datetime.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Number of seconds in one day. */
#define DATETIME_SECONDS_PER_DAY 86400U
/*! @brief Number of seconds in one hour. */
#define DATETIME_SECONDS_PER_HOUR 3600U
/*! @brief Number of seconds in one minute. */
#define DATETIME_SECONDS_PER_MINUTE 60U

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Days per month in a non leap year, January first. */
static const uint8_t s_daysPerMonth[12] = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};

/*******************************************************************************
 * Code
 ******************************************************************************/

bool DATETIME_IsLeapYear(uint16_t year)
{
    return (((year % 4U) == 0U) && ((year % 100U) != 0U)) || ((year % 400U) == 0U);
}

uint8_t DATETIME_GetDaysInMonth(uint16_t year, uint8_t month)
{
    uint8_t days = 0U;

    if ((month >= 1U) && (month <= 12U))
    {
        days = s_daysPerMonth[month - 1U];

        if ((month == 2U) && DATETIME_IsLeapYear(year))
        {
            days = 29U;
        }
    }

    return days;
}

bool DATETIME_IsValid(const datetime_t *datetime)
{
    bool valid = false;

    if (datetime != NULL)
    {
        valid = (datetime->year >= (uint16_t)DATETIME_EPOCH_YEAR) && (datetime->month >= 1U) &&
                (datetime->month <= 12U) && (datetime->day >= 1U) &&
                (datetime->day <= DATETIME_GetDaysInMonth(datetime->year, datetime->month)) &&
                (datetime->hour <= 23U) && (datetime->minute <= 59U) && (datetime->second <= 59U);
    }

    return valid;
}

uint32_t DATETIME_ToSeconds(const datetime_t *datetime)
{
    uint32_t days = 0U;
    uint16_t year;
    uint8_t month;

    if (!DATETIME_IsValid(datetime))
    {
        return 0U;
    }

    for (year = (uint16_t)DATETIME_EPOCH_YEAR; year < datetime->year; year++)
    {
        days += DATETIME_IsLeapYear(year) ? 366U : 365U;
    }

    for (month = 1U; month < datetime->month; month++)
    {
        days += DATETIME_GetDaysInMonth(datetime->year, month);
    }

    days += (uint32_t)datetime->day - 1U;

    return (days * DATETIME_SECONDS_PER_DAY) + ((uint32_t)datetime->hour * DATETIME_SECONDS_PER_HOUR) +
           ((uint32_t)datetime->minute * DATETIME_SECONDS_PER_MINUTE) + (uint32_t)datetime->second;
}

void DATETIME_FromSeconds(uint32_t seconds, datetime_t *datetime)
{
    uint32_t days;
    uint32_t remainder;
    uint16_t daysInYear;

    if (datetime == NULL)
    {
        return;
    }

    days      = seconds / DATETIME_SECONDS_PER_DAY;
    remainder = seconds % DATETIME_SECONDS_PER_DAY;

    datetime->hour   = (uint8_t)(remainder / DATETIME_SECONDS_PER_HOUR);
    datetime->minute = (uint8_t)((remainder % DATETIME_SECONDS_PER_HOUR) / DATETIME_SECONDS_PER_MINUTE);
    datetime->second = (uint8_t)(remainder % DATETIME_SECONDS_PER_MINUTE);

    datetime->year = (uint16_t)DATETIME_EPOCH_YEAR;
    daysInYear     = DATETIME_IsLeapYear(datetime->year) ? 366U : 365U;
    while (days >= (uint32_t)daysInYear)
    {
        days -= (uint32_t)daysInYear;
        datetime->year++;
        daysInYear = DATETIME_IsLeapYear(datetime->year) ? 366U : 365U;
    }

    datetime->month = 1U;
    while (days >= (uint32_t)DATETIME_GetDaysInMonth(datetime->year, datetime->month))
    {
        days -= (uint32_t)DATETIME_GetDaysInMonth(datetime->year, datetime->month);
        datetime->month++;
    }

    datetime->day = (uint8_t)(days + 1U);
}
