#include "idt.h"

#include <drivers/vga.hpp>
#include <drivers/pic.hpp>

static IDT::IDTEntry idt[256] = {0, 0, 0, 0, 0, 0};
static IDT::IDTDescriptor idtDescriptor;

extern "C" uintptr_t isr_table[256];

IDT::IntFunc lookup_table[256];

void RegisterIDTEntry(int index, uint64_t addr, uint8_t gate_type, uint8_t ist)
{
	idt[index].ist = ist;
	idt[index].selector = 0x8;
	idt[index].type = gate_type;
	idt[index].zero = 0;
	idt[index].offset1 = addr & 0xFFFF;
	idt[index].offset2 = (addr >> 16) & 0xFFFF;
	idt[index].offset3 = addr >> 32;
}

static const char *exception_messages[32] = {
    "Division by zero",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Detected overflow",
    "Out-of-bounds",
    "Invalid opcode",
    "No coprocessor",
    "Double fault",
    "Coprocessor segment overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown interrupt",
    "Coprocessor fault",
    "Alignment check",
    "Machine check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

extern "C" void InterruptHandler(IDT::registers_t* r)
{
	uint8_t int_no = r->int_no;

	if (r->int_no < 32)
	{
		printf("Exception!\n");
		printf("Exception: "); printf(exception_messages[r->int_no]);
		printf(" on cpu0\n");
		printf("Address: 0x%x\n", r->rip);
		printf("Error code: 0x%x\n", r->error_code);

		printf("\n\n*******************************\n");
			printf("*            ERROR            *\n");
			printf("*          CPU HALTED         *\n");
			printf("*******************************\n\n");
		asm volatile("cli");
		for (;;)
			asm volatile("hlt");
	}

	if (!lookup_table[int_no])
	{
		printf("[IDT]: Unhandled interrupt %d\n", int_no);
		asm volatile("cli");
		for(;;)
			asm volatile("hlt");
	}

	lookup_table[int_no](r);
}

void IDT::Init()
{
	for (int i = 0; i < 256; i++)
		RegisterIDTEntry(i, isr_table[i], 0x8E, /*i == 32 ? 1 :*/ 0);
	
	idtDescriptor.offset = (uint64_t)idt;
	idtDescriptor.size = sizeof(idt) - 1;
	
	asm volatile("lidt %0" :: "m"(idtDescriptor));
	asm volatile("sti");
}

void IDT::RegisterEntry(int interrupt, IntFunc func)
{
	lookup_table[interrupt] = func;
}
