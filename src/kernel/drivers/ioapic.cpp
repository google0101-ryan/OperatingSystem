#include "ioapic.h"
#include <mem/vmm.h>
#include <drivers/vga.hpp>

#define IO_RED_TBL_VECTOR(x) (x & 0xFF)
#define IO_APIC_RED_TABLE_ENT(x) (0x10 + 2 * x)

void IOAPIC::WriteReg(uint32_t reg, uint32_t data)
{
	ioapic->reg = reg;
	ioapic->data = data;
}

void IOAPIC::WriteReg64(uint32_t reg, uint64_t data)
{
	uint32_t low = data & 0xFFFFFFFF;
	uint32_t high = (data >> 32) & 0xFFFFFFFF;

	WriteReg(reg, low);
	WriteReg(reg+1, high);
}

uint32_t IOAPIC::ReadReg(uint32_t reg)
{
	ioapic->reg = reg;
	return ioapic->data;
}

IOAPIC::IOAPIC(uint64_t base)
{
	int id, maxintr;

	ioapic = (_IOAPIC*)base;

	VirtualMemory::MapPage(0xFEC00000, 0xFEC00000, flags::present | flags::writable | flags::cache_disable);

	maxintr = (ReadReg(0x01) >> 16) & 0xFF;
	id = ReadReg(0x00) >> 24;

	printf("Initialized IOAPIC with id %d, maximum interrupts %d\n", id, maxintr);
}

void IOAPIC::Redirect(uint8_t irq, uint8_t vector, uint32_t delivery)
{
	printf("Redirecting irq %d to irq %d\n", irq, vector);
	WriteReg64(IO_APIC_RED_TABLE_ENT(irq), vector | delivery);
}