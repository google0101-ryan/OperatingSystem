#pragma once

#include <stdint.h>

// Set up a custom GDT that we can modify, including a basic TSS
namespace GDT
{

void Init();

}