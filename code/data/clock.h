#include <stdint.h>

extern struct clock_t
{
	uint8_t time_second;
	uint8_t time_minute;
	uint8_t time_hour;

	uint8_t date_day;
	uint8_t date_month;
	uint8_t date_weekday;
	
} data_clock;
