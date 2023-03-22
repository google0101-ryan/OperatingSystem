#pragma once

#include <stdint.h>

struct Bitmap
{
	uint8_t* buffer;
	bool operator[](uint64_t index);
	bool Set(uint64_t index, bool value);
	bool Get(uint64_t index);	
};