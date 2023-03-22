#pragma once

#include <stdint.h>
#include <stivale2.h>

namespace PhysicalMemory
{

void Initialize(stivale2_struct_tag_memmap *hdr);

void* AllocPages(size_t count);
void FreePages(void* ptr, size_t count);

}