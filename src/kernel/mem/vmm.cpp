#include "vmm.h"
#include "pmm.h"

typedef uint64_t pdpt_entry;
typedef uint64_t pd_entry;
typedef uint64_t pt_entry;

struct PageTable
{
	pt_entry entries[512];
};

struct PageDirTable
{
	pd_entry entries[512];
};

struct PageDirPointerTable
{
	pdpt_entry entries[512];
};

VirtualMemory::PageMapLevel4* top_level = nullptr;

void  VirtualMemory::MapPage(PageMapLevel4* pml4, uint64_t phys, uint64_t virt, int flags)
{
	if (!pml4)
		pml4 = top_level;
	
	uint64_t pml4_index = (virt >> 39) & 0x1FF;
	uint64_t pdpt_index = (virt >> 30) & 0x1FF;
	uint64_t pd_index = (virt >> 21) & 0x1FF;
	uint64_t pt_index = (virt >> 12) & 0x1FF;

	if (!pml4->entries[pml4_index])
		pml4->entries[pml4_index] = ((uint64_t)PhysicalMemory::AllocPages(sizeof(PageDirPointerTable) / 0x1000)) | (flags & 0xFF);
	
	auto ptr = (PageDirPointerTable*)(top_level->entries[pml4_index] & ~0xFF);

	if (!ptr->entries[pdpt_index])
		ptr->entries[pdpt_index] = ((uint64_t)PhysicalMemory::AllocPages(sizeof(PageDirTable) / 0x1000)) | (flags & 0xFF);

	auto pagedir = (PageDirTable*)(ptr->entries[pdpt_index] & ~0xFF);

	if (!pagedir->entries[pd_index])
		pagedir->entries[pd_index] = ((uint64_t)PhysicalMemory::AllocPages(sizeof(PageTable) / 0x1000)) | (flags & 0xFF);
	
	auto pagetable = (PageTable*)(pagedir->entries[pd_index] & ~0xFF);

	pagetable->entries[pt_index] = phys | flags;
}

uint64_t VirtualMemory::CreateNewAddressSpace()
{
	PageMapLevel4* top = (PageMapLevel4*)PhysicalMemory::AllocPages(sizeof(PageMapLevel4) / 0x1000);

	// Map the kernel
	MapPage(top, 0, 0xffffffff80000000, 0x50000, flags::present | flags::writable);
	
	// Map VGA
	MapPage(top, 0xB8000, 0xB8000, flags::present | flags::writable);

	return (uint64_t)top;
}

void VirtualMemory::MapPage(PageMapLevel4* pml4, uint64_t phys, uint64_t virt, uint64_t count, int flags)
{
	for (size_t i = 0; i < count; i++)
		MapPage(pml4, phys+(i*0x1000), virt+(i*0x1000), flags);
}

#define FOUR_MEGS (1024*1024*4)

void VirtualMemory::Initialize()
{
	top_level = (PageMapLevel4*)PhysicalMemory::AllocPages(sizeof(PageMapLevel4) / 0x1000);
	
	// Map the kernel
	MapPage(nullptr, 0, 0xffffffff80000000, 0x50000, flags::present | flags::writable);
	
	// Map VGA
	MapPage(nullptr, 0xB8000, 0xB8000, flags::present | flags::writable);

	// Identity map the bottom 2MiB, and mirror it to 0xffff800000000000
	for (int i = 0; i < (FOUR_MEGS / 0x1000); i++)
	{
		uint64_t addr = i * 0x1000;
		MapPage(nullptr, addr, addr, flags::present | flags::writable);
	}

	// Find and map the heap
	MapPage(nullptr, (uint64_t)PhysicalMemory::AllocPages(HEAP_SIZE / 0x1000), HEAP_BEGIN, ALIGN(HEAP_SIZE / 0x1000, 0x1000), flags::present | flags::writable);

	asm volatile("mov %0, %%cr3\n\t" :: "r"(top_level) : "memory");
}