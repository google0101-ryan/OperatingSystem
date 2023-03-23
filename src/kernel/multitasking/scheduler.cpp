#include "scheduler.h"
#include "thread.h"
#include <drivers/vga.hpp>

Thread* head;
Thread* tail;
Thread* cur_thread;

bool inited;

void Scheduler::Initialize()
{
	cur_thread = head = nullptr;
	inited = true;
}

void Scheduler::AddThread(uint64_t entry)
{
	Thread* thread = CreateThread(entry);

	if (!head)
		head = thread;
	
	if (tail)
		tail->next = thread;
	tail = thread;
}

void SchedMemcpy(void* dst, void* src, size_t size)
{
	char* dest = (char*)dst;
	char* source = (char*)src;

	for (size_t i = 0; i < size; i++)
		dest[i] = source[i];
}

Thread* GetNext()
{
	if (cur_thread->next)
	{
		cur_thread = cur_thread->next;
		return cur_thread;
	}
	else
	{
		cur_thread = head;
		return cur_thread;
	}
}

IDT::registers_t* Scheduler::Tick(IDT::registers_t* regs)
{
	if (!inited)
		return regs;

	if (!head)
		return regs;
	
	printf("Switching tasks\n");

	// Save the current state
	SchedMemcpy(&cur_thread->saved_regs, regs, sizeof(IDT::registers_t));

	// Get the next ready thread
	auto next_thread = GetNext();

	// Copy its registers onto the stack
	SchedMemcpy(regs, &next_thread->saved_regs, sizeof(IDT::registers_t));
	
	// Switch to the new address space
	asm volatile("mov %0, %%cr3\n\t" :: "r"(next_thread->new_address_space) : "memory");

	// Return the modified registers
	return regs;
}
