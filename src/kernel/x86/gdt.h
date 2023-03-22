#pragma once

#include <stdint.h>

// Set up a custom GDT that we can modify, including a basic TSS
namespace GDT
{

struct GDTDescriptor {
    uint16_t limit;
    uint64_t offset;
} __attribute__((packed));

struct GDTEntry {
    uint16_t limit1;
    uint16_t base1;
    uint8_t base2;
    uint8_t access;
    uint8_t flags;
    uint8_t base3;
} __attribute__((packed));

struct TSSEntry {
    uint16_t limit;
    uint16_t base1;
    uint8_t base2;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t base3;
    uint32_t base4;
    uint32_t reserved;
} __attribute__((packed));

struct GDT {
    GDTEntry null;
    GDTEntry kernelCode;
    GDTEntry kernelData;
    GDTEntry userCode;
    GDTEntry userData;
    TSSEntry tss;
} __attribute__((packed));

void Init();

}