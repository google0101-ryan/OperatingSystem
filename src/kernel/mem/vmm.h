#pragma once

#include "pmm.hpp"

#define HEAP_ADDR 0xffffffffA0000000
#define HEAP_SIZE 0x2D7000

enum Flags
{
	Present = (1 << 0),
	Writeable = (1 << 1),
	User = (1 << 2)
};

namespace VirtualMemory
{

void Initialize();
void MapPage(uint64_t vaddr, uint64_t paddr, uint64_t flags);

}