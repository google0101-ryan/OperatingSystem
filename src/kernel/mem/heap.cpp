#include "heap.h"
#include "pmm.h"

struct MemoryBlock
{
	size_t size;
	bool free;
};

#define BLOCK_SIZE sizeof(MemoryBlock)

MemoryBlock* start;
size_t g_size;

size_t ContinousMemory(MemoryBlock* block)
{
	MemoryBlock* curBlock = block;

	size_t curSize = 0;

	while (curBlock->free && curSize < g_size)
	{
		curSize += curBlock->size + BLOCK_SIZE;
		curBlock = &curBlock[curBlock->size + BLOCK_SIZE];
	}

	if (curSize > 0)
	{
		curSize -= BLOCK_SIZE;
		block->size = curSize;
	}

	return curSize;
}

MemoryBlock* GetNextBlock(MemoryBlock* block)
{
	MemoryBlock* next = &block[block->size + BLOCK_SIZE];
	return next >= &start[g_size] ? nullptr : next;
}

void Heap::Initialize()
{
	start = (MemoryBlock*)HEAP_BEGIN;
	start->free = true;
	start->size = HEAP_SIZE - BLOCK_SIZE;

	g_size = HEAP_SIZE;
}

void *Heap::alloc(size_t size)
{
	MemoryBlock* block = start;

	while (block != nullptr)
	{
		auto availMem = ContinousMemory(block);
		if (availMem > size)
		{
			block->free = false;

			if (block->size > size + BLOCK_SIZE)
			{
				MemoryBlock* new_block = &block[size + BLOCK_SIZE];

				if (new_block < &start[g_size])
				{
					new_block->free = true;
					new_block->size = availMem - size - BLOCK_SIZE;

					block->size = size;
				}
			}

			return &block[BLOCK_SIZE];
		}
		else
		{
			block = GetNextBlock(block);
		}
	}

	return nullptr;
}

void Heap::free(void *ptr)
{
	MemoryBlock* block = &((MemoryBlock*)ptr)[-BLOCK_SIZE];
	block->free = false;
}

void* operator new(size_t size)
{
	return Heap::alloc(size);
}

void* operator new[](size_t size)
{
	return Heap::alloc(size);
}