/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FSL_DATETIME_H_
#define FSL_DATETIME_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * @addtogroup datetime
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Datetime component version 1.0.0. */
#define FSL_DATETIME_VERSION (MAKE_VERSION(1, 0, 0))

/*! @brief Epoch year used by the seconds conversion helpers. */
#define DATETIME_EPOCH_YEAR 1970U

/*! @brief Broken down calendar date and time. */
typedef struct _datetime
{
    uint16_t year;  /*!< Full year, for example 2026 */
    uint8_t month;  /*!< Month of the year, 1 to 12 */
    uint8_t day;    /*!< Day of the month, 1 to 31 */
    uint8_t hour;   /*!< Hour of the day, 0 to 23 */
    uint8_t minute; /*!< Minute of the hour, 0 to 59 */
    uint8_t second; /*!< Second of the minute, 0 to 59 */
} datetime_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Tells whether the given Gregorian year is a leap year.
 *
 * @param year Full year, for example 2026.
 * @return true when the year is a leap year.
 */
bool DATETIME_IsLeapYear(uint16_t year);

/*!
 * @brief Returns the number of days in the given month.
 *
 * @param year  Full year, needed to handle February in leap years.
 * @param month Month of the year, 1 to 12.
 * @return Number of days in the month, 0 when the month is out of range.
 */
uint8_t DATETIME_GetDaysInMonth(uint16_t year, uint8_t month);

/*!
 * @brief Tells whether the datetime fields are within their valid ranges.
 *
 * @param datetime Datetime to check.
 * @return true when every field is valid and the year is at or after the epoch.
 */
bool DATETIME_IsValid(const datetime_t *datetime);

/*!
 * @brief Converts a calendar date and time to seconds since the epoch.
 *
 * The epoch is 1970-01-01 00:00:00 UTC. The datetime is treated as UTC.
 *
 * @param datetime Datetime to convert.
 * @return Seconds since the epoch, 0 when the datetime is not valid.
 */
uint32_t DATETIME_ToSeconds(const datetime_t *datetime);

/*!
 * @brief Converts seconds since the epoch to a calendar date and time.
 *
 * The epoch is 1970-01-01 00:00:00 UTC. The result is in UTC.
 *
 * @param seconds  Seconds since the epoch.
 * @param datetime Datetime filled in by this function.
 */
void DATETIME_FromSeconds(uint32_t seconds, datetime_t *datetime);

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* FSL_DATETIME_H_ */
