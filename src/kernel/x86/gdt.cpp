#include "gdt.h"

static GDT::GDT gdt;
static GDT::GDTDescriptor gdtDescriptor;

extern "C" void loadGdt(uint64_t);

void GDT::Init()
{
	gdt.null = {0, 0, 0, 0, 0, 0};
	gdt.kernelCode = {0, 0, 0, 0x9A, 0x20, 0};
	gdt.kernelData = {0, 0, 0, 0x92, 0, 0};
	gdt.userCode = {0, 0, 0, 0xFA, 0x20, 0};
	gdt.userCode = {0, 0, 0, 0xF2, 0, 0};
	gdt.tss = {104, 0, 0, 0x89, 0, 0, 0, 0};

	gdtDescriptor = {sizeof(gdt) - 1, (uint64_t)&gdt};

	loadGdt((uint64_t)&gdtDescriptor);
}