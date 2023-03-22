#include "heap.h"
#include "pmm.h"

struct MemBlock
{
	uint64_t size;
	bool free;
	MemBlock *next, *prev;
};

MemBlock* first;

void Heap::Initialize()
{
	first = (MemBlock*)HEAP_BEGIN;

	first->next = first->prev = nullptr;
	first->size = HEAP_SIZE - sizeof(MemBlock);
}