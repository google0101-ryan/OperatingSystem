#include "vmm.h"

#include <drivers/vga.hpp>

struct PageTable
{
	uint64_t entries[512];
};

void SetAddr(PageTable* pt, uint64_t entry, uint64_t addr)
{
	pt->entries[entry] |= (addr >> 12) << 12;
}

void SetFlags(PageTable* pt, uint64_t entry, uint64_t flags)
{
	pt->entries[entry] |= flags;
}

PageTable* top_lvl;

void VirtualMemory::MapPage(uint64_t vaddr, uint64_t paddr, uint64_t flags)
{
	uint64_t pml4_index = (vaddr >> 39) & 0x1FF;
	uint64_t pml3_index = (vaddr >> 30) & 0x1FF;
	uint64_t pml2_index = (vaddr >> 21) & 0x1FF;
	uint64_t pml1_index = (vaddr >> 12) & 0x1FF;

	printf("[x]: Allocating pages\n");

	if (!(top_lvl->entries[pml4_index] & 1)) return;

	PageTable* pml3 = (PageTable*)(top_lvl->entries[pml4_index] & ~12);

	if (!pml3->entries[pml3_index])
	{
		pml3 = (PageTable*)PhysicalMemory::AllocPage(1);
	}
}

void VirtualMemory::Initialize()
{
	top_lvl = (PageTable*)PhysicalMemory::AllocPage(1);

	// Don't mark these as present, so we can catch accesses to unmapped pages
	for (int i = 0; i < 512; i++)
	{
		PageTable* entry = (PageTable*)PhysicalMemory::AllocPage(1);
		SetAddr(top_lvl, i, (uint64_t)entry);
	}

	SetFlags(top_lvl, 1, Present | Writeable | User);

	MapPage(HEAP_ADDR, (uint64_t)PhysicalMemory::AllocPage(1), Present | Writeable);
}
