#pragma once

#include <stdint.h>
#include <stivale2.h>

namespace PhysicalMemory
{

void Init(stivale2_struct_tag_memmap* memmap);
void* AllocPage(uint64_t count);

}