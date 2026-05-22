#pragma once

/******************************************************************************
* Configuration
*/
#define RTC_BITS_8	// comment out to use 32 bits

//#define	RTC_MINIMAL_RTC_TIME_T			// use the least possible memory for RTC_TIME_T,
										// at the expense of alignment on 32 bit systems

#if defined(RTC_MINIMAL_RTC_TIME_T) && defined(RTC_BITS_8)
#error RTC_MINIMAL_RTC_TIME_T makes no sense in 8 bit mode (RTC_BITS_8 defined)
#endif

// comment out to use split mode (RTC_time)
//#define RTC_MODE_SECONDS_SINCE_EPOCH	// uses RTC_seconds_since_epoch

// comment out to use iteration for cacluations
#define	RTC_MODE_DIV_MUL				// use division and multiplication
										// instead of iteration

// fill these in with code to disable/enable the RTC tick interrupt

// for bare metal STM32
//#define	RTC_ENABLE_INTERRUPT	do{ __enable_irq(); } while(0)
//#define	RTC_DISABLE_INTERRUPT	do{ __disable_irq(); } while(0)

#ifndef RTC_ENABLE_INTERRUPT
#error "RTC_ENABLE_INTERRUPT not configured for this platform"
#endif
#ifndef RTC_DISABLE_INTERRUPT
#error "RTC_DISABLE_INTERRUPT not configured for this platform"
#endif

/******************************************************************************
* Public definitions, enums and typedefs
*/
#include <stdint.h>
#include <stdbool.h>

#define	SECONDS_PER_MINUTE			(60)
#define	SECONDS_PER_HOUR			(60 * 60)
#define	SECONDS_PER_DAY				(60 * 60 * 24)
#define	SECONDS_PER_NON_LEAP_YEAR	(SECONDS_PER_DAY * 365)
#define	SECONDS_PER_LEAP_YEAR		(SECONDS_PER_DAY * 366)
#define SECONDS_PER_LEAP_CENTURY	(3155760000)

#define UNIX_TIMESTAMP_AT_EPOCH		(946684800)

// convert to seconds
// useful for calculating timezone offsets
#define	HMS_TO_SECONDS(hours, minutes, seconds) \
			((hours*SECONDS_PER_HOUR)+(minutes*SECONDS_PER_MINUTE)+seconds)
#define	DHMS_TO_SECONDS(days, hours, minutes, seconds) \
			((days*SECONDS_PER_DAY)+(hours*SECONDS_PER_HOUR)+(minutes*SECONDS_PER_MINUTE)+seconds)


typedef enum 
#ifdef RTC_BITS_8
__attribute__((__packed__))
#endif
{
	RTC_DEPTH_Y			= 0,
	RTC_DEPTH_YM		= 1,
	RTC_DEPTH_YMD		= 2,
	RTC_DEPTH_YMDH		= 3,
	RTC_DEPTH_YMDHMS	= 4,
} RTC_DEPTH_e;

#ifdef RTC_BITS_8
typedef	uint8_t		RTC_UINT;
#define	PRIrtc		PRIu8
#define	RTC_EPOCH_YEAR	(0)
#else
typedef	uint32_t	RTC_UINT;
#define	PRIrtc		PRIu32
#define	RTC_EPOCH_YEAR	(2000)
#endif

enum {
	RTC_MONDAY = 0,
	RTC_TUESDAY, RTC_WEDNESDAY, RTC_THURSDAY, RTC_FRIDAY, RTC_SATURDAY, RTC_SUNDAY
};

#ifndef RTC_MINIMAL_RTC_TIME_T

typedef struct {
	RTC_UINT	year, month, day;
	RTC_UINT	hour, minute, second;
} RTC_TIME_t;

#else

typedef struct __attribute__((__packed__))  {
	uint16_t year;
	uint8_t month, day;
	uint8_t hour, minute, second;
} RTC_TIME_t;

#endif

/******************************************************************************
* Public variables
*/
extern const RTC_UINT days_in_month[12];

#ifdef RTC_MODE_SECONDS_SINCE_EPOCH
extern volatile uint32_t RTC_seconds_since_epoch;
#else
extern volatile RTC_TIME_t RTC_time;
#endif

/******************************************************************************
* Public functions
*/
extern bool RTC_is_leap_year(RTC_UINT year);
extern bool RTC_is_leap_year_sse(uint32_t seconds_since_epoch);

extern RTC_UINT RTC_days_in_month(RTC_UINT month, RTC_UINT year);

extern RTC_UINT RTC_day_of_week_split(const RTC_TIME_t *split);
extern RTC_UINT RTC_day_of_week_sse(uint32_t seconds_since_epoch);
#define RTC_day_of_week(X) _Generic((X), \
	RTC_TIME_t*: RTC_day_of_week_split, \
    const RTC_TIME_t*: RTC_day_of_week_split, \
    uint32_t: RTC_day_of_week_sse \
)(X)

extern RTC_UINT RTC_day_of_year_split(const RTC_TIME_t* split);
extern RTC_UINT RTC_day_of_year_sse(uint32_t seconds_since_epoch);
#define RTC_day_of_year(X) _Generic((X), \
	RTC_TIME_t*: RTC_day_of_year_split, \
    const RTC_TIME_t*: RTC_day_of_year_split, \
    uint32_t: RTC_day_of_year_sse \
)(X)

extern void RTC_day_of_week_and_year_split(const RTC_TIME_t* split, RTC_UINT* day_of_week, RTC_UINT* day_of_year);
extern void RTC_day_of_week_and_year_sse(uint32_t seconds_since_epoch, RTC_UINT* day_of_week, RTC_UINT* day_of_year);
#define RTC_day_of_week_and_year(X, Y, Z) _Generic((X), \
	RTC_TIME_t*: RTC_day_of_week_and_year_split, \
    const RTC_TIME_t*: RTC_day_of_week_and_year_split, \
    uint32_t: RTC_day_of_week_and_year_sse \
)(X)

extern uint32_t RTC_split_to_seconds_since_epoch(const RTC_TIME_t *split);
extern void RTC_seconds_since_epoch_to_split(uint32_t seconds_since_epoch, RTC_TIME_t *split);
extern void RTC_seconds_since_epoch_to_split_ex(uint32_t seconds_since_epoch, RTC_TIME_t* split, bool* is_leap_year, RTC_DEPTH_e depth);

extern uint32_t RTC_ymd_to_days_since_epoch(const RTC_TIME_t *split);

extern RTC_UINT RTC_dst_start_day_eu(RTC_UINT year);
extern RTC_UINT RTC_dst_end_day_eu(RTC_UINT year);

extern bool RTC_seconds_since_epoch_is_in_dst_eu(uint32_t seconds_since_epoch, bool* leap_year);
extern uint32_t RTC_local_time_split(RTC_UINT year, RTC_UINT month, RTC_UINT day, RTC_UINT hour, RTC_UINT minute, RTC_UINT second, int32_t timezone_offset_seconds, bool eu_dst);
extern uint32_t RTC_local_time_seconds_since_epoch(uint32_t seconds_since_epoch, int32_t timezone_offset_seconds, bool eu_dst);
#ifdef RTC_MODE_SECONDS_SINCE_EPOCH
extern uint32_t RTC_get_time(void);
#else
extern void RTC_get_time(RTC_TIME_t* target);
#endif
extern uint32_t RTC_get_time_seconds_since_epoch(void);
extern void RTC_get_time_split(RTC_TIME_t* split);

/******************************************************************************
* Tick function that advances the RTC by 1 second.
*/
static inline void RTC_tick(void)
{
#ifdef RTC_MODE_SECONDS_SINCE_EPOCH
	RTC_seconds_since_epoch++;
#else
	RTC_time.second++;
	if (RTC_time.second > 59)
	{
		RTC_time.second = 0;
		RTC_time.minute++;
		if (RTC_time.minute > 59)
		{
			RTC_time.minute = 0;
			RTC_time.hour++;
			if (RTC_time.hour > 23)
			{
				RTC_time.hour = 0;
				RTC_time.day++;
				if ((RTC_time.month > 12) || (RTC_time.month < 1))
					RTC_time.month = 1;
				RTC_UINT dim = days_in_month[RTC_time.month - 1];
				if ((RTC_time.month == 2) && (RTC_is_leap_year(RTC_time.year)))
					dim++;
				if (RTC_time.day > dim)
				{
					dim = 1;
					RTC_time.month++;
					if (RTC_time.month > 12)
					{
						RTC_time.month = 1;
						RTC_time.year++;
					}
				}
			}
		}
	}
#endif
}
