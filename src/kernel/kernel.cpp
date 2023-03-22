#include <stdint.h>
#include "stivale2.h"

#include <drivers/vga.hpp>
#include <drivers/pic.hpp>

#include <x86/gdt.h>
#include <x86/idt.h>

#include <mem/pmm.h>
#include <mem/vmm.h>
#include <mem/heap.h>

uint8_t stack[16384];

__attribute__((section(".stivale2hdr"), used))
static stivale2_header stivaleHeader = {
	.entry_point = 0,
	.stack = (uintptr_t)stack + sizeof(stack),
	.flags = (1 << 1) | (1 << 2),
	.tags = 0
};

void* get_tag(stivale2_struct* first_tag, uint64_t tag_id) {
    stivale2_tag *current_tag = (stivale2_tag*)first_tag->tags;

    while (true) {
        if (!current_tag) {
            return nullptr;
        }

        if (current_tag->identifier == tag_id) {
            return current_tag;
        }

        current_tag = (stivale2_tag*)current_tag->next;
    }
}

extern "C" void kmain(stivale2_struct* stivale)
{
	VGA::Init();
	
	VGA::puts("[x]: Kernel console initialized, debug messages enabled\n");

	GDT::Init();

	VGA::puts("[x]: Kernel GDT initialized\n");

	IDT::Init();

	VGA::puts("[x]: Kernel IDT initialized\n");

	PIC::RemapIRQs();

	VGA::puts("[x]: Remapped IRQs via PIC\n");

	VGA::puts("[x]: Loading physical memory\n");

	PhysicalMemory::Initialize((stivale2_struct_tag_memmap*)get_tag(stivale, STIVALE2_STRUCT_TAG_MEMMAP_ID));

	VGA::puts("[x]: Loading virtual memory\n");

	VirtualMemory::Initialize();

	VGA::puts("[x]: Initializing kernel heap\n");

	Heap::Initialize();

	VGA::puts("[x]: Kernel core initialized\n");

	for (;;);
}