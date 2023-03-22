#include "pit.h"
#include <util/ports.h>
#include <x86/idt.h>
#include <drivers/vga.hpp>
#include <drivers/apic.h>
#include <drivers/ioapic.h>

static const char ticker[] =
{
	'/',
	'-',
	'\\',
	'|'
};

static int ticker_index;

void HandleTimer()
{
	int x = VGA::get_x();
	int y = VGA::get_y();

	VGA::seek_to(79, 0);

	VGA::putc(ticker[ticker_index++]);

	if (ticker_index > 3)
		ticker_index = 0;
	
	VGA::seek_to(x, y);

	lapic->EOI();
}

int hz = 100; // Hardcoded to 100Hz

void PIT::Initialize()
{
	IDT::RegisterEntry(32, HandleTimer);

	uint32_t divisor = 1193182 / 100;

	outb(0x43, 0x36);

	uint8_t l = divisor & 0xff;
	uint8_t h = divisor >> 8;

	outb(0x40, l);
	outb(0x40, h);
}