#pragma once

#include <stdint.h>
#include <stivale2.h>
#include <stddef.h>

#define HEAP_SIZE 32*1024
#define HEAP_BEGIN 0xffffa00000000000

#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN(x,a)              __ALIGN_MASK(x,(__typeof__(x))(a)-1)

namespace PhysicalMemory
{

void Initialize(stivale2_struct_tag_memmap *hdr);

void* AllocPages(size_t count);
void FreePages(void* ptr, size_t count);

}