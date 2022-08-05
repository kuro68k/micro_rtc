// micro_rtc.c
//
// TODO: minimized mode (e.g. RTC_ymd_to_days_since_epoch())
// TODO: test RTC_local_time_split()

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "iso_8601_date.h"
#include "micro_rtc.h"

const RTC_UINT days_in_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#ifdef RTC_MODE_SECONDS_SINCE_EPOCH
volatile uint32_t RTC_seconds_since_epoch = 0;
#else
volatile RTC_TIME_t RTC_time;
#endif

/*****************************************************************************
* Check if year is a leap year
*/
bool RTC_is_leap_year(RTC_UINT year)
{
#ifndef RTC_BITS_8
	if ((year & 0b11) != 0)
		return false;
	if (year % 400 == 0)
		return  true;
	if (year % 100 == 0)
		return false;
	return true;
#else
	if ((year & 0b11) != 0)
		return false;
	if ((year == 100) || (year == 200))	// divisible by 100, not divisible by 400
		return false;
	return true;
#endif
}

/*****************************************************************************
* Get the day of the week. 0 = Monday.
* Month = 1-12, day = 1-31.
*/
RTC_UINT RTC_day_of_week(RTC_TIME_t split)
{
	uint32_t	dow = split.day;

	RTC_UINT m = 1;
	while (m < split.month)
	{
		dow += days_in_month[m - 1];
		if ((m == 2) && RTC_is_leap_year(split.year))
			dow++;
		m++;
	}

	RTC_UINT y = RTC_EPOCH_YEAR;
	while (y < split.year)
	{
		dow += 365;
		if (RTC_is_leap_year(y))
			dow++;
		y++;
	}
	dow--;		// 2000/01/01 is day zero
	dow += 5;	// 2000/01/01 is a Saturday
	dow %= 7;
	return dow;
}

/*****************************************************************************
* Convert RTC time to seconds since epoch.
* Note that month is 1-12, day is 1-31.
*/
uint32_t RTC_split_to_seconds_since_epoch(RTC_TIME_t split)
{
	uint32_t seconds_since_epoch = split.second;
	seconds_since_epoch += split.minute * 60;
	seconds_since_epoch += split.hour * SECONDS_PER_HOUR;

	return seconds_since_epoch + RTC_ymd_to_days_since_epoch(split) * SECONDS_PER_DAY;
}

/*****************************************************************************
* Seconds since epoch to Y/D/M h/m/s
*/
void RTC_seconds_since_epoch_to_split(uint32_t seconds_since_epoch, RTC_TIME_t *split)
{
	RTC_seconds_since_epoch_to_split_ex(seconds_since_epoch, split, NULL, RTC_DEPTH_YMDHMS);
}

/*****************************************************************************
* Seconds since epoch to Y/D/M h/m/s, with extended functionality.
* If *is_leap_year is not NULL, it is set accordingly.
* Depth selects the data to calculate:
*	0 - year
*	1 - year, month
*	2 - year, month, day
*	3 - year, month, day, hour
*	>3 - all
* NB. uncalculated values are not changed or cleared.
*/
void RTC_seconds_since_epoch_to_split_ex(
	uint32_t seconds_since_epoch, RTC_TIME_t* split,
	bool *is_leap_year, RTC_DEPTH_e depth)
{
	bool leap_year;

	// year, RTC_EPOCH_YEAR-
	split->year = RTC_EPOCH_YEAR;

	if (seconds_since_epoch >= SECONDS_PER_LEAP_CENTURY)
	{
		seconds_since_epoch -= SECONDS_PER_LEAP_CENTURY;
		split->year += 100;
	}

	for (;;)
	{
		uint32_t seconds_this_decade;
		if (RTC_is_leap_year(split->year))
			seconds_this_decade = (SECONDS_PER_NON_LEAP_YEAR * 7) + (SECONDS_PER_LEAP_YEAR * 3);
		else
			seconds_this_decade = (SECONDS_PER_NON_LEAP_YEAR * 8) + (SECONDS_PER_LEAP_YEAR * 2);

		if (seconds_since_epoch < seconds_this_decade)
			break;
		seconds_since_epoch -= seconds_this_decade;
		split->year += 10;
	}

	for (;;)
	{
		uint32_t seconds_this_year = SECONDS_PER_DAY * 365;
		leap_year = RTC_is_leap_year(split->year);
		if (leap_year)
			seconds_this_year += SECONDS_PER_DAY;
		if (seconds_since_epoch >= seconds_this_year)
		{
			split->year++;
			seconds_since_epoch -= seconds_this_year;
		}
		else
			break;
	}

	if (is_leap_year != NULL)
		*is_leap_year = leap_year;

	if (depth == RTC_DEPTH_Y)
		return;

	// month, 1-12
	split->month = 1;
	for (;;)
	{
		uint32_t seconds_this_month = SECONDS_PER_DAY * days_in_month[split->month - 1];
		if ((split->month == 2) && leap_year)
			seconds_this_month += SECONDS_PER_DAY;
		if (seconds_since_epoch >= seconds_this_month)
		{
			split->month++;
			seconds_since_epoch -= seconds_this_month;
		}
		else
			break;
	}

	if (depth == RTC_DEPTH_YM)
		return;

#ifdef RTC_MODE_DIV_MUL

	// day of month, 1-31
	split->day = seconds_since_epoch / SECONDS_PER_DAY;
	seconds_since_epoch -= split->day * SECONDS_PER_DAY;
	split->day++;

	if (depth == RTC_DEPTH_YMD)
		return;

	// hours, 0-23
	split->hour = seconds_since_epoch / SECONDS_PER_HOUR;
	seconds_since_epoch -= split->hour * SECONDS_PER_HOUR;

	if (depth == RTC_DEPTH_YMDH)
		return;

	// minutes, 0-59
	split->minute = seconds_since_epoch / SECONDS_PER_MINUTE;
	seconds_since_epoch -= split->minute * SECONDS_PER_MINUTE;

#else

	// day of month, 1-31
	split->day = 1;
	while (seconds_since_epoch >= SECONDS_PER_DAY)
	{
		split->day++;
		seconds_since_epoch -= SECONDS_PER_DAY;
	}

	if (depth == RTC_DEPTH_YMD)
		return;

	// hours, 0-23
	split->hour = 0;
	while (seconds_since_epoch >= SECONDS_PER_HOUR)
	{
		split->hour++;
		seconds_since_epoch -= SECONDS_PER_HOUR;
	}

	if (depth == RTC_DEPTH_YMDH)
		return;

	// minutes, 0-59
	split->minute = 0;
	while (seconds_since_epoch >= 60)
	{
		split->minute++;
		seconds_since_epoch -= 60;
	}

#endif

	split->second = seconds_since_epoch;
}

/*****************************************************************************
* Convert D/M/Y to days since epoch.
* Note that month is 1-12, day is 1-31.
*/
uint32_t RTC_ymd_to_days_since_epoch(RTC_TIME_t split)
{
	uint32_t days = split.day - 1;

	RTC_UINT m = 1;
	while (m < split.month)
	{
		days += days_in_month[m-1];
		if ((m == 2) && RTC_is_leap_year(split.year))
			days++;
		m++;
	}

#ifdef RTC_MODE_DIV_MUL
	RTC_UINT y = split.year - RTC_EPOCH_YEAR;
	if (y > 0)
	{
		days += y * 365;
		// calculate leap years
		y--;
		days += y / 4;
		days -= y / 100;
		days += y / 400;
		days++;		// epoch year is a leap year
	}
#else
	RTC_UINT y = RTC_EPOCH_YEAR;
	while (y < split.year)
	{
		if (RTC_is_leap_year(y))
			days += 366;
		else
			days += 365;
		y++;
	}
#endif
	return days;
}

/*****************************************************************************
* Calculate DST start and end dates using the EU scheme.
* Start month is March, end month is October.
* Start and end times are 01:00.
* Only valid from year 2000 onwards.
*/
RTC_UINT RTC_dst_start_day_eu(RTC_UINT year)
{
	uint32_t y32 = year + (2000 - RTC_EPOCH_YEAR);
	return 31 - ((((5 * y32) / 4) + 4) % 7);
}
RTC_UINT RTC_dst_end_day_eu(RTC_UINT year)
{
	uint32_t y32 = year + (2000 - RTC_EPOCH_YEAR);
	return 31 - ((((5 * y32) / 4) + 1) % 7);
}

/*****************************************************************************
* Calculate DST start and end times using the EU scheme.
*/
uint32_t RTC_dst_start_seconds_since_epoch_eu(RTC_UINT year)
{
	RTC_TIME_t split = { .year = year, .month = 3 };
	split.day = RTC_dst_start_day_eu(year);
	return RTC_split_to_seconds_since_epoch(split);
}
uint32_t RTC_dst_end_seconds_since_epoch_eu(RTC_UINT year)
{
	RTC_TIME_t split = { .year = year, .month = 10 };
	split.day = RTC_dst_end_day_eu(year);
	return RTC_split_to_seconds_since_epoch(split);
}

/*****************************************************************************
* Check if an SSE time is inside EU DST.
* leap_year can be NULL if not used.
*/
bool RTC_seconds_since_epoch_is_in_dst_eu(uint32_t seconds_since_epoch, bool *leap_year)
{
	// get year
	RTC_TIME_t time;
	RTC_seconds_since_epoch_to_split_ex(seconds_since_epoch, &time, leap_year, RTC_DEPTH_Y);
	uint32_t start_time = RTC_dst_start_seconds_since_epoch_eu(time.year);
	uint32_t end_time = RTC_dst_end_seconds_since_epoch_eu(time.year);
	return ((seconds_since_epoch >= start_time) && (seconds_since_epoch < end_time));
}

/*****************************************************************************
* Get local time from UTC. Returns 0 on underflow.
* eu_dst enables DST adjustment of the returned value.
*/
uint32_t RTC_local_time_split(
		RTC_UINT year, RTC_UINT month, RTC_UINT day,
		RTC_UINT hour, RTC_UINT minute, RTC_UINT second,
		int32_t timezone_offset_seconds, bool eu_dst)
{
	uint32_t seconds_since_epoch = 0;
	int32_t seconds_offset = timezone_offset_seconds;
	if ((seconds_offset < 0) && (seconds_since_epoch < abs(seconds_offset)))
		seconds_since_epoch = 0;
	else
		seconds_since_epoch += seconds_offset;

	if (eu_dst && RTC_seconds_since_epoch_is_in_dst_eu(seconds_since_epoch, NULL))
		seconds_since_epoch += SECONDS_PER_HOUR;

	return seconds_since_epoch;
}

/*****************************************************************************
* Get local time from UTC. Returns 0 on underflow.
* eu_dst enables DST adjustment of the returned value.
*/
uint32_t RTC_local_time_seconds_since_epoch(
	uint32_t seconds_since_epoch, int32_t timezone_offset_seconds, bool eu_dst)
{
	int32_t seconds_offset = timezone_offset_seconds;
	if ((seconds_offset < 0) && (seconds_since_epoch < abs(seconds_offset)))
		seconds_since_epoch = 0;
	else
		seconds_since_epoch += seconds_offset;

	if (eu_dst && RTC_seconds_since_epoch_is_in_dst_eu(seconds_since_epoch, NULL))
		seconds_since_epoch += SECONDS_PER_HOUR;

	return seconds_since_epoch;
}

/*****************************************************************************
* Get the current RTC time.
*/
#ifdef RTC_MODE_SECONDS_SINCE_EPOCH
uint32_t RTC_get_time(void)
{
	return (uint32_t)RTC_seconds_since_epoch;
}
#else
void RTC_get_time(RTC_TIME_t *target)
{
	RTC_DISABLE_INTERRUPT;
	memcpy(target, (void*)&RTC_time, sizeof(RTC_TIME_t));
	RTC_ENABLE_INTERRUPT;
}
#endif

uint32_t RTC_get_time_seconds_since_epoch(void)
{
#ifdef RTC_MODE_SECONDS_SINCE_EPOCH
	return RTC_get_time();
#else
	RTC_TIME_t temp;
	RTC_get_time(&temp);
	return RTC_split_to_seconds_since_epoch(temp);
#endif
}

void RTC_get_time_split(RTC_TIME_t *split)
{
#ifdef RTC_MODE_SECONDS_SINCE_EPOCH
	RTC_seconds_since_epoch_to_split(RTC_get_time(), split);
#else
	RTC_get_time(split);
#endif
}
