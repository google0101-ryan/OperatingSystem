#pragma once

#include <stdint.h>

namespace flags
{

enum : uint64_t
{
    present = 1,
    writable = 2,
    user_mode = 4,
    write_through = 8,
    cache_disable = 16,
	accessed = 32,
	dirty = 64,
	attribute_ext = 128,
	big_page = 128,
	huge_page = 128,
	global = 256,
};

}

namespace VirtualMemory
{

typedef uint64_t pml4_entry;
struct PageMapLevel4
{
	pml4_entry entries[512];
};
	
void Initialize();
void MapPage(PageMapLevel4* pml4, uint64_t phys, uint64_t virt, uint64_t count, int flags);
void MapPage(PageMapLevel4* pml4, uint64_t phys, uint64_t virt, int flags);

uint64_t CreateNewAddressSpace();

}