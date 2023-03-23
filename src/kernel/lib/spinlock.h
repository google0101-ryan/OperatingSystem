#pragma once

#include <stdint.h>

class Spinlock
{
private:
	bool locked;
public:
	Spinlock();

	void Lock();
	void Release();
};