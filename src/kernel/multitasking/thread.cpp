#include "thread.h"

void memset(void* a, char c, size_t size)
{
	char* dst = (char*)a;

	for (size_t i = 0; i < size; i++)
		dst[i] = c;
}

Thread *CreateThread(uint64_t entry)
{
	Thread* thread = new Thread();
	memset(thread, 0, sizeof(thread));
	thread->saved_regs.rip = entry;
	thread->saved_regs.rsp = (uint64_t)new uint8_t[16384];
	thread->next = nullptr;
	thread->new_address_space = VirtualMemory::CreateNewAddressSpace();

	thread->saved_regs.cs = 0x08;
	thread->saved_regs.ss = 0x10;

	return thread;
}