#pragma once

#include <cstdint>

// LAPIC is per-core, so make it unique
class LAPIC
{
private:
	uint32_t* lapic_base;

	void WriteReg(uint32_t reg, uint32_t value);
	uint32_t ReadReg(uint32_t reg);
public:
	LAPIC();

	void EOI();
};

extern LAPIC* lapic;