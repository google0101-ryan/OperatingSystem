#pragma once

#include <cstdint>
#include <stivale2.h>

namespace ACPI
{

void FindTables(stivale2_struct* hdr);

void SetupAPIC();

}