#pragma once

#include <stdint.h>
#include <x86/idt.h>

namespace Scheduler
{

void Initialize();

void AddThread(uint64_t entry);

void Tick(IDT::registers_t* regs);

}