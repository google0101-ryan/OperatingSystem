#include "pmm.hpp"

#include <drivers/vga.hpp>

uint64_t free_ram; // How much free space do we have?
uint64_t highest_addr = 0;

#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

struct Bitmap
{
	uint8_t* buffer;
	void Set(uint64_t addr, bool allocated);
} bitmap;

void Bitmap::Set(uint64_t addr, bool allocated)
{
	uint64_t bytei = addr / 8;
	uint8_t biti = addr % 8;
	uint8_t bitindexer = 0b10000000 >> biti;
	buffer[bytei] &= ~bitindexer;
	if (allocated) buffer[bytei] |= bitindexer;
}

void PhysicalMemory::Init(stivale2_struct_tag_memmap *memmap)
{
	printf("[x]:\tStivale-provided memory map contains %d entries\n", memmap->entries);

	uint64_t memmap_count = memmap->entries;

	for (uint64_t i = 0; i < memmap_count; i++)
	{
		if (memmap->memmap[i].type != STIVALE2_MMAP_USABLE) continue;

		uintptr_t top = memmap->memmap[i].base + memmap->memmap[i].length;
		free_ram += memmap->memmap[i].length;

		if (top > highest_addr) highest_addr = top;
	}
	
	uint64_t bitmap_size = ALIGN((highest_addr / 0x1000) / 8, 0x1000);

	for (uint64_t i = 0; i < memmap_count; i++)
	{
		if (memmap->memmap[i].type != STIVALE2_MMAP_USABLE) continue;

		if (memmap->memmap[i].length < bitmap_size) continue;

		bitmap.buffer = (uint8_t*)memmap->memmap[i].base;
		for (int i = 0; i < bitmap_size; i++)
			bitmap.buffer[i] = 0xFF;

		memmap->memmap[i].base += bitmap_size;
		memmap->memmap[i].length -= bitmap_size;
		free_ram -= bitmap_size;
	}

	for (uint64_t i = 0; i < memmap_count; i++)
	{
		if (memmap->memmap[i].type != STIVALE2_MMAP_USABLE) continue;

		for (uint64_t j = 0; j < memmap->memmap[i].length; j += 0x1000)
			bitmap.Set((memmap->memmap[i].base + j) / 0x1000, false);
	}
}

static uint64_t lastI = 0;

// Find the first free page and give it away
void *PhysicalMemory::AllocPage(uint64_t count)
{
	uint64_t limit = highest_addr / 0x1000;
	uint64_t p = 0;

	while (lastI < limit)
	{
		if (!bitmap.buffer[lastI++])
		{
			if (++p == count)
			{
				uint64_t page = lastI - count;
				for (uint64_t i = page; i < lastI; i++) bitmap.Set(i, true);
				return reinterpret_cast<void*>(page * 0x1000);
			}
		}
		else p = 0;
	}

	return nullptr;
}
