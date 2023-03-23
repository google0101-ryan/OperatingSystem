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

struct registers_t
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, error_code, rip, cs, rflags, rsp, ss;
} __attribute__((packed));

void Init();

using IntFunc = void (*)(registers_t*);

void RegisterEntry(int interrupt, IntFunc func);

}