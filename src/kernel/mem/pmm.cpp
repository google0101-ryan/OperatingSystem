#include <mem/pmm.h>
#include <drivers/vga.hpp>
#include <lib/bitmap.h>
#include <stddef.h>
#include "pmm.h"

Bitmap bitmap;
static uintptr_t highest_addr = 0;
static size_t lastI = 0;
static size_t usedRam = 0;
static size_t freeRam = 0;

static void* InnerAlloc(size_t count, size_t limit)
{
	size_t p = 0;

	while (lastI < limit)
	{
		if (!bitmap[lastI++])
		{
			if (++p == count)
			{
				size_t page = lastI - count;
				for (size_t i = page; i < lastI; i++) bitmap.Set(i, true);
				return reinterpret_cast<void*>(page * 0x1000);
			}
		}
		else p = 0;
	}
	return nullptr;
}

void *PhysicalMemory::AllocPages(size_t count)
{
	size_t i = lastI;
	void* ret = InnerAlloc(count, highest_addr / 0x1000);
	if (ret == nullptr)
	{
		lastI = 0;
		ret = InnerAlloc(count, i);
		if (ret == nullptr)
		{
			printf("Panic! Out of memory!\n");
			for (;;)
				printf("");
		}
	}

	for (uint64_t i = 0; i < count * 0x1000; i++)
		*(reinterpret_cast<uint8_t*>(ret)) = 0;
	
	usedRam += count * 0x1000;
	freeRam -= count * 0x1000;

	return ret;
}

void PhysicalMemory::FreePages(void *ptr, size_t count)
{
	if (ptr == nullptr) return;

	size_t page = reinterpret_cast<size_t>(ptr) / 0x1000;
	for (size_t i = page; i < page + count; i++) bitmap.Set(i, false);
	if (lastI > page) lastI = page;

	usedRam -= count * 0x1000;
	freeRam += count * 0x1000;
}

void PhysicalMemory::Initialize(stivale2_struct_tag_memmap *memmap)
{
	for (size_t i = 0; i < memmap->entries; i++)
	{
		if (memmap->memmap[i].type != STIVALE2_MMAP_USABLE) continue;

		uintptr_t top = memmap->memmap[i].base + memmap->memmap[i].length;
		freeRam += memmap->memmap[i].length;

		if (top > highest_addr) highest_addr = top;
	}

	size_t bitmapSize = ALIGN(((highest_addr / 0x1000) / 8), 0x1000);

	for (size_t i = 0; i < memmap->entries; i++)
	{
		if (memmap->memmap[i].type != STIVALE2_MMAP_USABLE) continue;

		if (memmap->memmap[i].length >= bitmapSize)
		{
			bitmap.buffer = reinterpret_cast<uint8_t*>(memmap->memmap[i].base);
			for (size_t i = 0; i < bitmapSize; i++)
				bitmap.buffer[i] = 0xFF;
			
			memmap->memmap[i].length -= bitmapSize;
			memmap->memmap[i].base += bitmapSize;
			freeRam -= bitmapSize;
			break;
		}
	}

	printf("Bitmap is at 0x%x, %d bytes\n", (uint64_t)bitmap.buffer, bitmapSize);

	for (size_t i = 0; i < memmap->entries; i++)
	{
		if (memmap->memmap[i].type != STIVALE2_MMAP_USABLE) continue;

		for (uintptr_t t = 0; t < memmap->memmap[i].length; t += 0x1000)
		{
			bitmap.Set((memmap->memmap[i].base + t) / 0x1000, false);
		}
	}
}