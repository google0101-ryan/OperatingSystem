#include "thread.h"
#include <mem/pmm.h>

void memset(void* a, char c, size_t size)
{
	char* dst = (char*)a;

	for (size_t i = 0; i < size; i++)
		dst[i] = c;
}

Thread *CreateThread(uint64_t entry)
{
	Thread* thread = new Thread();
	uint64_t stack = (uint64_t)PhysicalMemory::AllocPages((ALIGN(16384, 0x1000)) / 0x1000);
	
	memset(thread, 0, sizeof(thread));

	thread->saved_regs.rip = entry;
	thread->saved_regs.rsp = 0xffffb00000000000 + ALIGN(16384, 0x1000);
	thread->saved_regs.rflags = 0x202; // Make sure the I flag is set
	thread->saved_regs.cs = 0x08;
	thread->saved_regs.ss = 0x10;

	thread->next = nullptr;
	thread->new_address_space = VirtualMemory::CreateNewAddressSpace(stack, ALIGN(16384, 0x1000));

	uint8_t* kstackBase = new uint8_t[524288];
	thread->kernelStack = (uint64_t)kstackBase + 524288;
	
	return thread;
}