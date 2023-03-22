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
	
void Initialize();
void MapPage(uint64_t phys, uint64_t virt, uint64_t count, int flags);
void MapPage(uint64_t phys, uint64_t virt, int flags);

}