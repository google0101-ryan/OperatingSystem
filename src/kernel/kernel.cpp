#include <stdint.h>
#include "stivale2.h"

#include <drivers/vga.hpp>
#include <drivers/pic.hpp>
#include <drivers/apic.h>
#include <drivers/ioapic.h>
#include <drivers/pit.h>
#include <drivers/acpi.h>
#include <drivers/rtc.h>

#include <x86/gdt.h>
#include <x86/idt.h>

#include <mem/pmm.h>
#include <mem/vmm.h>
#include <mem/heap.h>

#include <multitasking/scheduler.h>
#include <multitasking/thread.h>

#include <fs/vfs.h>

LAPIC* lapic;
IOAPIC* ioapic;

uint8_t stack[16384];

static struct stivale2_struct_tag_rsdp rsdp_tag = {
	.tag = {
		.identifier = STIVALE2_STRUCT_TAG_RSDP_ID,
		.next = (uintptr_t)0
	},
	.rsdp = 0
};

__attribute__((section(".stivale2hdr"), used))
static stivale2_header stivaleHeader = {
	.entry_point = 0,
	.stack = (uintptr_t)stack + sizeof(stack),
	.flags = (1 << 1) | (1 << 2) | (1 << 4),
	.tags = (uintptr_t)&rsdp_tag
};

void* get_tag(stivale2_struct* first_tag, uint64_t tag_id) 
{
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

stivale2_struct* g_root;

// We go here once our scheduler is initialized
void KernelTask()
{
	printf("[x]: Entered kernel task, setting up initrd\n");

	VFS::the()->Initialize(g_root);

	for (;;)
		asm volatile("hlt");
}

extern "C" void kmain(stivale2_struct* stivale)
{
	VGA::Init();
	
	printf("[x]: Kernel console initialized, debug messages enabled\n");

	GDT::Init();

	printf("[x]: Kernel GDT initialized\n");

	IDT::Init();

	printf("[x]: Kernel IDT initialized\n");

	PIC::RemapIRQs();

	printf("[x]: Remapped IRQs via PIC\n");

	ACPI::FindTables(stivale);

	printf("[x]: Loading physical memory\n");

	PhysicalMemory::Initialize((stivale2_struct_tag_memmap*)get_tag(stivale, STIVALE2_STRUCT_TAG_MEMMAP_ID));

	printf("[x]: Loading virtual memory\n");

	VirtualMemory::Initialize();

	// We have new and delete now
	GDT::InitTSS((uint64_t)stack + sizeof(char) * 16384);

	printf("[x]: Kernel core initialized\n");

	lapic = new LAPIC();
	ACPI::SetupAPIC();

	printf("[x]: Initialized APIC\n");

	PIT::Initialize();

	printf("[x]: Initialized PIT\n");

	auto time = RealTimeClock::ReadTime();

	printf("Booted on %d/%d/%d, %d:%d:%d\n", time.month, time.day_of_month, time.year+2000, time.hours, time.minutes, time.seconds);

	Scheduler::Initialize();

	g_root = stivale;

	asm volatile("cli");

	Scheduler::AddThread((uint64_t)KernelTask);

	asm volatile("sti");

	for (;;)
		asm volatile("hlt");
}