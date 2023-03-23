#pragma once

#include <stdint.h>

// Set up a custom GDT that we can modify, including a basic TSS
namespace GDT
{

void Init();

void InitTSS(uint64_t i);

void SetKernelStack(uint64_t rsp0);

}