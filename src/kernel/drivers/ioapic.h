#pragma once

#include <cstdint>

class IOAPIC
{
private:
	struct _IOAPIC
	{
		uint32_t reg;
		uint32_t pad[3];
		uint32_t data;
	} *ioapic;

	void WriteReg(uint32_t reg, uint32_t data);
	void WriteReg64(uint32_t reg, uint64_t data);
	uint32_t ReadReg(uint32_t reg);
public:
	IOAPIC(uint64_t base);

	void Redirect(uint8_t irq, uint8_t vector, uint32_t delivery);
};

extern IOAPIC* ioapic;