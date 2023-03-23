#include "rtc.h"
#include <util/ports.h>

uint8_t ReadRegister(uint8_t reg)
{
	outb(0x70, reg);
	return inb(0x71);
}

int UpdateInProgress()
{
	outb(0x70, 0x0A);
	return (inb(0x71) & 0x80);
}

RealTimeClock::time_t RealTimeClock::ReadTime()
{
	time_t ret;

	while (UpdateInProgress());

	ret.seconds = ReadRegister(0x00);
	ret.minutes = ReadRegister(0x02);
	ret.hours = ReadRegister(0x04);
	ret.weekday = ReadRegister(0x06);
	ret.day_of_month = ReadRegister(0x07);
	ret.month = ReadRegister(0x08);
	ret.year = ReadRegister(0x09);
	ret.century_maybe = ReadRegister(0x32);

	uint8_t regB = ReadRegister(0x0B);

	if (!(regB & 0x04))
	{
		ret.seconds = (ret.seconds & 0x0F) + ((ret.seconds / 16) * 10);
		ret.minutes = (ret.minutes & 0x0F) + ((ret.minutes / 16) * 10);
		ret.hours = ((ret.hours & 0x0F) + (((ret.hours & 0x70) / 16) * 10)) | (ret.hours & 0x80);
		ret.day_of_month = (ret.day_of_month & 0x0F) + ((ret.day_of_month / 16) * 10);
		ret.month = (ret.month & 0x0F) + ((ret.month / 16) * 10);
		ret.year = (ret.year & 0x0F) + ((ret.year / 16) * 10);
	}

	if (!(regB & 0x02) && (ret.hours & 0x80))
		ret.hours = ((ret.hours & 0x7F) + 12) % 24;

	return ret;
}