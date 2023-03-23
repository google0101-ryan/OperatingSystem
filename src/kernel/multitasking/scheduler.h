#pragma once

#include <stdint.h>
#include <x86/idt.h>

namespace Scheduler
{

void Initialize();

void AddThread(uint64_t entry);

IDT::registers_t* Tick(IDT::registers_t* regs);

}