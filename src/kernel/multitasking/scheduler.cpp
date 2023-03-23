#include "scheduler.h"
#include "thread.h"
#include <drivers/vga.hpp>
#include <x86/gdt.h>

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
	if (!cur_thread)
	{
		cur_thread = head;
		return cur_thread;
	}

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

void Scheduler::Tick(IDT::registers_t* regs)
{
	if (!inited)
		return;

	if (!head)
		return;

	// Save the current state
	cur_thread->saved_regs = *regs;
	
	// Get the next ready thread
	auto next_thread = GetNext();
	
	GDT::SetKernelStack(next_thread->kernelStack);

	asm volatile(
		 R"(mov %0, %%rsp;
		   	mov %1, %%rax;
		   	pop %%r15;
			pop %%r14;
			pop %%r13;
			pop %%r12;
			pop %%r11;
			pop %%r10;
			pop %%r9;
			pop %%r8;
			pop %%rbp;
			pop %%rdi;
			pop %%rsi;
			pop %%rdx;
			pop %%rcx;
			pop %%rbx;
			
			mov %%rax, %%cr3
			pop %%rax
			addq $16, %%rsp
			iretq)" :: "r"(&next_thread->saved_regs),
			"r"(next_thread->new_address_space));
}