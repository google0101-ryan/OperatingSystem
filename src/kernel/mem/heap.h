#pragma once

#include <stdint.h>
#include <stddef.h>

namespace Heap
{
	
void Initialize();

void* alloc(size_t size);

void free(void* ptr);

}