#pragma once

#include <stdint.h>

namespace RealTimeClock
{

struct time_t
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t weekday;
	uint8_t day_of_month;
	uint8_t month;
	uint32_t year;
	uint8_t century_maybe;
};

time_t ReadTime();

}