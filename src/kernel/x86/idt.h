#pragma once

#include <cstdint>

namespace IDT
{

struct IDTDescriptor
{
	uint16_t size;
	uint64_t offset;
} __attribute__((packed));

struct IDTEntry
{
	uint16_t offset1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t zero;
} __attribute__((packed));

typedef struct __attribute__((packed))
{ 
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rax;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	// Contains error code and interrupt number for exceptions
	// Contains syscall number for syscalls
	// Contains just the interrupt number otherwise
	uint64_t info;
	// Interrupt stack frame
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
} registers_t;

void Init();

using IntFunc = void (*)(registers_t*);

void RegisterEntry(int interrupt, IntFunc func);

}