#pragma once

#include <lib/spinlock.h>
#include <x86/idt.h>
#include <mem/vmm.h>
#include <mem/pmm.h>

struct Thread
{
	uint64_t new_address_space;
	Thread* next;
	IDT::registers_t saved_regs;
};

Thread* CreateThread(uint64_t entry);