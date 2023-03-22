#pragma once

#include <stdint.h>

namespace PIC
{

void RemapIRQs();
void AckInterruptMaster();
void AckInterruptSlave();

}