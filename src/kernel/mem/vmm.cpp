#include "vmm.h"
#include "pmm.h"

typedef uint64_t pml4_entry;
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

struct PageMapLevel4
{
	pml4_entry entries[512];
};

struct flags
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
};

PageMapLevel4* top_level = nullptr;

void MapPage(uint64_t phys, uint64_t virt, int flags)
{
	uint64_t pml4_index = (virt >> 39) & 0x1FF;
	uint64_t pdpt_index = (virt >> 30) & 0x1FF;
	uint64_t pd_index = (virt >> 21) & 0x1FF;
	uint64_t pt_index = (virt >> 12) & 0x1FF;

	if (!top_level->entries[pml4_index])
		top_level->entries[pml4_index] = ((uint64_t)PhysicalMemory::AllocPages(sizeof(PageDirPointerTable) / 0x1000)) | (flags & 0xFF);
	
	auto ptr = (PageDirPointerTable*)(top_level->entries[pml4_index] & ~0xFF);

	if (!ptr->entries[pdpt_index])
		ptr->entries[pdpt_index] = ((uint64_t)PhysicalMemory::AllocPages(sizeof(PageDirTable) / 0x1000)) | (flags & 0xFF);

	auto pagedir = (PageDirTable*)(ptr->entries[pdpt_index] & ~0xFF);

	if (!pagedir->entries[pd_index])
		pagedir->entries[pd_index] = ((uint64_t)PhysicalMemory::AllocPages(sizeof(PageTable) / 0x1000)) | (flags & 0xFF);
	
	auto pagetable = (PageTable*)(pagedir->entries[pd_index] & ~0xFF);

	pagetable->entries[pt_index] = phys | flags;
}

void MapPage(uint64_t phys, uint64_t virt, uint64_t count, int flags)
{
	for (size_t i = 0; i < count; i++)
		MapPage(phys+(i*0x1000), virt+(i*0x1000), flags);
}

void VirtualMemory::Initialize()
{
	top_level = (PageMapLevel4*)PhysicalMemory::AllocPages(sizeof(PageMapLevel4) / 0x1000);

	// Map the kernel
	MapPage(0, 0xffffffff80000000, 0xC000, flags::present | flags::writable);

	// Map VGA
	MapPage(0xB8000, 0xB8000, flags::present | flags::writable);

	// Find and map the heap
	MapPage((uint64_t)PhysicalMemory::AllocPages(HEAP_SIZE / 0x1000), HEAP_BEGIN, HEAP_SIZE / 0x1000, flags::present | flags::writable);

	asm volatile("mov %0, %%cr3\n\t" :: "r"(top_level) : "memory");
}