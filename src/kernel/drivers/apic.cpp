#include "apic.h"
#include <util/ports.h>
#include <mem/vmm.h>
#include <x86/idt.h>
#include <drivers/vga.hpp>

#define LOCAL_APIC_ID 0x20 // APIC ID Register
#define LOCAL_APIC_VERSION 0x30 // APIC Version Register
#define LOCAL_APIC_TPR 0x80 // Task Priority Register 
#define LOCAL_APIC_APR 0x90 // Arbitration Priority Register
#define LOCAL_APIC_PPR 0xA0 // Processor Priority Register
#define LOCAL_APIC_EOI 0xB0 // End of Interrupt Register
#define LOCAL_APIC_RRD 0xC0 // Remote Read Register
#define LOCAL_APIC_LDR 0xD0 // Logical Destination Register
#define LOCAL_APIC_DFR 0xE0 // Destination Format Register
#define LOCAL_APIC_SIVR 0xF0 // Spurious Interrupt Vector Register
#define LOCAL_APIC_ISR 0x100 // In-service Register
#define LOCAL_APIC_TMR 0x180 // Trigger Mode Register
#define LOCAL_APIC_IRR 0x200 // Interrupt Request Register
#define LOCAL_APIC_ERROR_STATUS 0x280 // Error Status Register
#define LOCAL_APIC_ICR_LOW 0x300 // Interrupt Command Register Low
#define LOCAL_APIC_ICR_HIGH 0x310 // Interrupt Command Register High
#define LOCAL_APIC_LVT_TIMER 0x320 // Timer Local Vector Table Entry
#define LOCAL_APIC_LVT_THERMAL 0x330 // Thermal Local Vector Table Entry
#define LOCAL_APIC_LVT_PERF_MONITORING 0x340 // Performance Local Vector Table Entry
#define LOCAL_APIC_LVT_LINT0 0x350 // Local Interrupt 0 Local Vector Table Entry
#define LOCAL_APIC_LVT_LINT1 0x360 // Local Interrupt 1 Local Vector Table Entry
#define LOCAL_APIC_LVT_ERROR 0x370 // Error Local Vector Table Entry
#define LOCAL_APIC_TIMER_INITIAL_COUNT 0x380 // Timer Initial Count Register
#define LOCAL_APIC_TIMER_CURRENT_COUNT 0x390 // Timer Current Count Register
#define LOCAL_APIC_TIMER_DIVIDE 0x3E0 // Timer Divide Configuration Register

#define LOCAL_APIC_BASE 0xFFFFFFFFFF000

#define ICR_VECTOR(x) (x & 0xFF)
#define ICR_MESSAGE_TYPE_FIXED 0
#define ICR_MESSAGE_TYPE_LOW_PRIORITY (1 << 8)
#define ICR_MESSAGE_TYPE_SMI (2 << 8)
#define ICR_MESSAGE_TYPE_REMOTE_READ (3 << 8)
#define ICR_MESSAGE_TYPE_NMI (4 << 8)
#define ICR_MESSAGE_TYPE_INIT (5 << 8)
#define ICR_MESSAGE_TYPE_STARTUP (6 << 8)
#define ICR_MESSAGE_TYPE_EXTERNAL (7 << 8)

#define ICR_DSH_DEST 0          // Use destination field
#define ICR_DSH_SELF (1 << 18)  // Send to self
#define ICR_DSH_ALL (2 << 18)   // Send to ALL APICs
#define ICR_DSH_OTHER (3 << 18) // Send to all OTHER APICs 

#define IO_APIC_REGSEL 0x00 // I/O APIC Register Select Address Offset
#define IO_APIC_WIN 0x10 // I/O APIC I/O Window Address offset

#define IO_APIC_REGISTER_ID 0x0 // ID Register
#define IO_APIC_REGISTER_VER 0x1 // Version Register
#define IO_APIC_REGISTER_ARB 0x2 // I/O APIC Arbitration ID
#define IO_APIC_RED_TABLE_START 0x10 // I/O APIC Redirection Table Start
#define IO_APIC_RED_TABLE_ENT(x) (0x10 + 2 * x)

#define IO_RED_TBL_VECTOR(x) (x & 0xFF)

uint64_t ReadBase()
{
	uint64_t low, high;
	asm("rdmsr" : "=a"(low), "=d"(high) : "c"(0x1b));
	return (high << 32) | low;
}

void WriteBase(uint64_t val)
{
	uint64_t low = val & 0xFFFFFFFF;
    uint64_t high = val >> 32;
    asm("wrmsr" ::"a"(low), "d"(high), "c"(0x1B));
}

void LAPIC::WriteReg(uint32_t reg, uint32_t value)
{
	*(uint32_t*)((uint8_t*)lapic_base + reg) = value;
}

uint32_t LAPIC::ReadReg(uint32_t reg)
{
	return *(uint32_t*)((uint8_t*)lapic_base + reg);
}

void SpuriousInterruptHandler(IDT::registers_t* r)
{
	printf("[APIC]: Spurious interrupt\n");
}

LAPIC::LAPIC()
{
	uint64_t base = ReadBase() & LOCAL_APIC_BASE;
	lapic_base = (uint32_t*)base;

	VirtualMemory::MapPage(nullptr, base, base, flags::present | flags::writable | flags::cache_disable);

	IDT::RegisterEntry(0xFF, SpuriousInterruptHandler);

	WriteBase(ReadBase() | (1UL << 11));

	WriteReg(LOCAL_APIC_SIVR, ReadReg(LOCAL_APIC_SIVR) | 0x1FF);
}

void LAPIC::EOI()
{
	WriteReg(LOCAL_APIC_EOI, 0);
}