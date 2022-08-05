// main() with tests for micro_rtc
//
// TODO:	RTC_seconds_since_epoch_is_in_dst_eu()
//			RTC_local_time_split()
//			RTC_local_time_seconds_since_epoch
//			RTC_get_time()
//			RTC_get_time_seconds_since_epoch()
//			RTC_get_time_split()
//
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "iso_8601_date.h"
#include "micro_rtc.h"
#include "build.h"

#define	ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

/****************************************************************************/

#ifndef RTC_BITS_8
uint32_t test_leap_years[] = {
	1804, 1808, 1812, 1816, 1820, 1824, 1828, 1832, 1836, 1840, 1844, 1848,
	1852, 1856, 1860, 1864, 1868, 1872, 1876, 1880, 1884, 1888, 1892, 1896,
	1904, 1908, 1912, 1916, 1920, 1924, 1928, 1932, 1936, 1940, 1944, 1948,
	1952, 1956, 1960, 1964, 1968, 1972, 1976, 1980, 1984, 1988, 1992, 1996,
	2000, 2004, 2008, 2012, 2016, 2020, 2024, 2028, 2032, 2036, 2040, 2044,
	2048, 2052, 2056, 2060, 2064, 2068, 2072, 2076, 2080, 2084, 2088, 2092,
	2096, 2104, 2108, 2112, 2116, 2120, 2124, 2128, 2132, 2136, 2140, 2144,
	2148, 2152, 2156, 2160, 2164, 2168, 2172, 2176, 2180, 2184, 2188, 2192,
	2196, 2204, 2208, 2212, 2216, 2220, 2224, 2228, 2232, 2236, 2240, 2244,
	2248, 2252, 2256, 2260, 2264, 2268, 2272, 2276, 2280, 2284, 2288, 2292,
	2296, 2304, 2308, 2312, 2316, 2320, 2324, 2328, 2332, 2336, 2340, 2344,
	2348, 2352, 2356, 2360, 2364, 2368, 2372, 2376, 2380, 2384, 2388, 2392,
	2396, 2400
};
#define	TEST_LEAP_YEARS_START_YEAR	1800
#define	TEST_LEAP_YEARS_END_YEAR	2403
#else
uint32_t test_leap_years[] = {
	0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72,
	76, 80, 84, 88, 92,	96, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140,
	144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196,
	204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252
};
#define	TEST_LEAP_YEARS_START_YEAR	0
#define	TEST_LEAP_YEARS_END_YEAR	255
#endif

struct {
	RTC_UINT dow;
	RTC_TIME_t split;
} test_days_of_week[] = {
	{ RTC_SATURDAY,	{ 0 + RTC_EPOCH_YEAR,  1,  1 } },
	{ RTC_SUNDAY,	{ 0 + RTC_EPOCH_YEAR,  1,  2 } },
	{ RTC_TUESDAY,	{ 0 + RTC_EPOCH_YEAR,  2, 29 } },
	{ RTC_SUNDAY,	{ 0 + RTC_EPOCH_YEAR, 12, 31 } },
	{ RTC_MONDAY,	{ 1 + RTC_EPOCH_YEAR,  1,  1 } },
	{ RTC_THURSDAY,	{ 24 + RTC_EPOCH_YEAR,  2, 29 } },
	{ RTC_THURSDAY,	{ 99 + RTC_EPOCH_YEAR, 12, 31 } },
	{ RTC_MONDAY,	{ 255 + RTC_EPOCH_YEAR, 12, 31 } },
};

struct {
	uint32_t seconds_since_epoch;
	RTC_TIME_t split;
	bool is_leap_year;
} test_split_to_seconds_since_epoch[] = {
	{0, {0 + RTC_EPOCH_YEAR, 1, 1, 0, 0, 0}, true},
	{ 1, { 0 + RTC_EPOCH_YEAR, 1, 1, 0, 0, 1 }, true },
	{ 86399, { 0 + RTC_EPOCH_YEAR, 1, 1, 23, 59, 59 }, true },
	{ 86400, { 0 + RTC_EPOCH_YEAR, 1, 2, 0, 0, 0 }, true },
	{ 5157326, { 0 + RTC_EPOCH_YEAR, 2, 29, 16, 35, 26 }, true },
	{ 31622399, { 0 + RTC_EPOCH_YEAR, 12, 31, 23, 59, 59 }, true },
	{ 31622400, { 1 + RTC_EPOCH_YEAR, 1, 1, 0, 0, 0 }, false },
	{ 3172408526, { 100 + RTC_EPOCH_YEAR, 7, 12, 16, 35, 26 }, false },
	{ 4294967295, { 136 + RTC_EPOCH_YEAR, 2, 7, 6, 28, 15 }, true },
};

struct {
	uint32_t days_since_epoch;
	RTC_TIME_t split;
} test_ymd_to_days_since_epoch[] = {
	{ 0, { 0 + RTC_EPOCH_YEAR, 1, 1 } },			// 0
	{ 1, { 0 + RTC_EPOCH_YEAR, 1, 2 } },			// 1
	{ 59, { 0 + RTC_EPOCH_YEAR, 2, 29 } },			// 2
	{ 60, { 0 + RTC_EPOCH_YEAR, 3, 1 } },			// 3
	{ 365, { 0 + RTC_EPOCH_YEAR, 12, 31 } },		// 4
	{ 366, { 1 + RTC_EPOCH_YEAR, 1, 1 } },			// 5
	{ 8139, { 22 + RTC_EPOCH_YEAR, 4, 14 } },		// 6
	{ 36524, { 99 + RTC_EPOCH_YEAR, 12, 31 } },		// 7
	{ 36525, { 100 + RTC_EPOCH_YEAR, 1, 1 } },		// 8
	{ 40177, { 110 + RTC_EPOCH_YEAR, 1, 1 } },		// 9
	{ 43828, { 119 + RTC_EPOCH_YEAR, 12, 31 } },	// 10
	{ 43829, { 120 + RTC_EPOCH_YEAR, 1, 1 } },		// 11
	{ 49709, { 136 + RTC_EPOCH_YEAR, 2, 6 } },		// 12
	{ 49710, { 136 + RTC_EPOCH_YEAR, 2, 7 } },		// 13
#ifndef RTC_BITS_8
	{ 146097, { 400 + RTC_EPOCH_YEAR, 1, 1 } },		// 14
#endif
};

#include "dst_dates.h"

/****************************************************************************/

int main(void)
{
	int errors = 0;
	
	printf("__DATE__ \"%s\"\n", __DATE__);
	printf("__TIME__ \"%s\"\n", __TIME__);
	printf("ISO 8601 date: \"%s\"\n", ISO_8601_DATE);
	printf("Build timestamp: \"%s\"\n", build_timestamp);
	printf("Build number: \"%s\"\n", build_number);

	// leap years
	printf("RTC_is_leap_year()\n");
	for (RTC_UINT year = TEST_LEAP_YEARS_START_YEAR; year < TEST_LEAP_YEARS_END_YEAR; year++)
	{
		bool found = false;
		for (uint32_t i = 0; i < ARRAY_COUNT(test_leap_years); i++)
		{
			if (test_leap_years[i] == year)
				found = true;
		}
		bool res = RTC_is_leap_year(year);
		if (found != res)
		{
			printf("Year %" PRIrtc " returned %s\n", year, res ? "true" : "false");
			errors++;
		}
	}

	// day of week
	printf("RTC_day_of_week()\n");
	for (uint32_t i = 0; i < ARRAY_COUNT(test_days_of_week); i++)
	{
		RTC_UINT dow = RTC_day_of_week(test_days_of_week[i].split);
		if (dow != test_days_of_week[i].dow)
		{
			printf(
				"Error line %" PRIu32 ", want %" PRIrtc ", "
				"DOW=%" PRIrtc ", %04" PRIrtc "/%02" PRIrtc "/%02" PRIrtc "\n",
				i, test_days_of_week[i].dow, dow,
				(RTC_UINT)(test_days_of_week[i].split.year),
				test_days_of_week[i].split.month, test_days_of_week[i].split.day);
			errors++;
		}
	}

	// split to seconds since epoch
	printf("RTC_split_to_seconds_since_epoch()\n");
	for (uint32_t i = 0; i < ARRAY_COUNT(test_split_to_seconds_since_epoch); i++)
	{
		uint32_t seconds_since_epoch = RTC_split_to_seconds_since_epoch(
			test_split_to_seconds_since_epoch[i].split);
		if (seconds_since_epoch != test_split_to_seconds_since_epoch[i].seconds_since_epoch)
		{
			printf("Error line %" PRIu32 ", want %" PRIu32 ", got %" PRIu32 "\n",
				i,
				test_split_to_seconds_since_epoch[i].seconds_since_epoch,
				seconds_since_epoch);
			errors++;
		}
	}

	// seconds since epoch to split
	printf("RTC_seconds_since_epoch_to_split()\n");
	for (uint32_t i = 0; i < ARRAY_COUNT(test_split_to_seconds_since_epoch); i++)
	{
		RTC_TIME_t split;
		RTC_seconds_since_epoch_to_split(
			test_split_to_seconds_since_epoch[i].seconds_since_epoch, &split);
		if (memcmp(&split, &test_split_to_seconds_since_epoch[i].split, sizeof(RTC_TIME_t)) != 0)
		{
			printf("Error line %" PRIu32 ", want "
				"%04" PRIrtc "/%02" PRIrtc "/%02" PRIrtc " "
				"%02" PRIrtc ":%02" PRIrtc ":%02" PRIrtc ", got "
				"%04" PRIrtc "/%02" PRIrtc "/%02" PRIrtc " "
				"%02" PRIrtc ":%02" PRIrtc ":%02" PRIrtc "\n",
				i,
				test_split_to_seconds_since_epoch[i].split.year,
				test_split_to_seconds_since_epoch[i].split.month,
				test_split_to_seconds_since_epoch[i].split.day,
				test_split_to_seconds_since_epoch[i].split.hour,
				test_split_to_seconds_since_epoch[i].split.minute,
				test_split_to_seconds_since_epoch[i].split.second,
				split.year, split.month, split.day, split.hour, split.minute, split.second);
			errors++;
		}
	}

	// seconds since epoch to split
	printf("RTC_seconds_since_epoch_to_split_ex()\n");
	for (uint32_t i = 0; i < ARRAY_COUNT(test_split_to_seconds_since_epoch); i++)
	{
		RTC_TIME_t split;
		bool leap;
		RTC_seconds_since_epoch_to_split_ex(
			test_split_to_seconds_since_epoch[i].seconds_since_epoch, &split, &leap, RTC_DEPTH_YMDHMS);
		if ((memcmp(&split, &test_split_to_seconds_since_epoch[i].split, sizeof(RTC_TIME_t)) != 0) ||
			(leap != test_split_to_seconds_since_epoch[i].is_leap_year))
		{
			printf("Error line %" PRIu32 ", want "
				"%04" PRIrtc "/%02" PRIrtc "/%02" PRIrtc " "
				"%02" PRIrtc ":%02" PRIrtc ":%02" PRIrtc " %s, got "
				"%04" PRIrtc "/%02" PRIrtc "/%02" PRIrtc " "
				"%02" PRIrtc ":%02" PRIrtc ":%02" PRIrtc " %s\n",
				i,
				test_split_to_seconds_since_epoch[i].split.year,
				test_split_to_seconds_since_epoch[i].split.month,
				test_split_to_seconds_since_epoch[i].split.day,
				test_split_to_seconds_since_epoch[i].split.hour,
				test_split_to_seconds_since_epoch[i].split.minute,
				test_split_to_seconds_since_epoch[i].split.second,
				test_split_to_seconds_since_epoch[i].is_leap_year ? "true" : "false",
				split.year, split.month, split.day, split.hour, split.minute, split.second,
				leap ? "true" : "false");
			errors++;
		}
	}

	// year/month/day to days since epoch
	printf("RTC_ymd_to_days_since_epoch()\n");
	for (uint32_t i = 0; i < ARRAY_COUNT(test_ymd_to_days_since_epoch); i++)
	{
		uint32_t days_since_epoch = RTC_ymd_to_days_since_epoch(
			test_ymd_to_days_since_epoch[i].split);
		if (days_since_epoch != test_ymd_to_days_since_epoch[i].days_since_epoch)
		{
			printf("Error line %" PRIu32 ", want %" PRIu32 ", got %" PRIu32 "\n",
				i,
				test_ymd_to_days_since_epoch[i].days_since_epoch,
				days_since_epoch);
			errors++;
		}
	}

	// DST
	printf("RTC_dst_start_day_eu(), RTC_dst_end_day_eu()\n");
	RTC_UINT year = RTC_EPOCH_YEAR;
	for (uint32_t i = 0; i < ARRAY_COUNT(test_eu_dst_dates);)
	{
		uint8_t startd = test_eu_dst_dates[i++];
		uint8_t endd = test_eu_dst_dates[i++];
		if ((startd != RTC_dst_start_day_eu(year)) ||
			(endd != RTC_dst_end_day_eu(year)))
		{
			printf("Year %" PRIu32 ", Got %" PRIrtc "-%" PRIrtc ", wanted %" PRIrtc "-%" PRIrtc "\n",
				year, startd, endd,
				RTC_dst_start_day_eu(year), RTC_dst_end_day_eu(year));
			errors++;
		}
		year++;
	}

	return errors;
}
