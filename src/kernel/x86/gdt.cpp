#include "gdt.h"
#include <stddef.h>

extern "C" void loadGdt(uint64_t);

enum gdt_selector : uint16_t
{
	NULL_SELECTOR = 0,
	KERNEL_CODE = 0x8,
	KERNEL_DATA = 0x10,
	USER_DATA = 0x1b,
	USER_CODE = 0x23,
	TSS_SELECTOR = 0x28
};

enum gdt_flags : uint8_t
{
	WRITABLE = 0b10,
	USER = 0b1100000,
	PRESENT = 0b10000000,
	CONFORMING = 0b100,
	TSS = 0b1001,
	DS = 0b10000,
	CS = 0b11000
};

enum gdt_gran : uint8_t
{
	LONG_MODE_GRANULARITY = 0b00100000
};

struct gdtr
{
	uint16_t len;
	uint64_t addr;
} __attribute__((packed));

struct gdt_descriptor
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t flags;
    uint8_t granularity; /* and high limit */
    uint8_t base_high;

    gdt_descriptor(){};
    gdt_descriptor(uint8_t flag, uint8_t gran)
    {
        base_high = 0;
        base_mid = 0;
        base_low = 0;
        flags = flag | gdt_flags::PRESENT;
        granularity = gran;
        limit_low = 0;
    }
} __attribute__((packed));
struct gdt_xdescriptor
{
    gdt_descriptor low;

    struct
    {
        uint32_t base_xhigh;
        uint32_t reserved;
    } high;

    gdt_xdescriptor(uint8_t flag, uintptr_t base, uintptr_t limit)
    {
        low.flags = flag | gdt_flags::PRESENT;
        low.granularity = (0 << 4) | ((limit >> 16) & 0x0F);
        low.limit_low = limit & 0xFFFF;
        low.base_low = base & 0xFFFF;
        low.base_mid = ((base >> 16) & 0xFF);
        low.base_high = ((base >> 24) & 0xFF);
        high.base_xhigh = ((base >> 32) & 0xFFFFFFFF);
        high.reserved = 0;
    }
} __attribute__((packed));

struct tss
{
    uint32_t reserved0 __attribute__((aligned(16)));

    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;

    uint64_t reserved1;

    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;

    uint64_t reserved2;
    uint16_t reserved3;

    uint16_t iomap_base;
} __attribute__((packed));

#define GDT_ARRAY_SEL(a) (a / 8)

void memset(void* a, int c, size_t size)
{
	char* _a = (char*)a;
	for (int i = 0; i < size; i++)
		_a[i] = c;
}

template <int entry_count>
class gdt_descriptor_array
{
    gdt_descriptor entrys[entry_count];

public:
    constexpr size_t size() const
    {
        return sizeof(gdt_descriptor) * entry_count;
    }
    void zero()
    {
        memset(entrys, 0, sizeof(entrys[0]) * entry_count);
    }
    gdt_descriptor *get_entry(gdt_selector entry)
    {
        return &entrys[GDT_ARRAY_SEL(entry)];
    }
    gdt_xdescriptor *get_entry_x(gdt_selector entry)
    {
        return (gdt_xdescriptor *)&entrys[GDT_ARRAY_SEL(entry)];
    }
    void set(gdt_selector entry, gdt_descriptor desc)
    {
        *get_entry(entry) = desc;
    }
    void xset(gdt_selector entry, gdt_xdescriptor desc)
    {
        *get_entry_x(entry) = desc;
    }
    gdt_descriptor *raw()
    {
        return entrys;
    }
    void fill_gdt_register(gdtr *target)
    {
        target->len = size() - 1;
        target->addr = (uintptr_t)raw();
    }
};

gdt_descriptor_array<7> gdt_descriptors;

#define STACK_SIZE 16384

char tss_ist1[STACK_SIZE] __attribute__((aligned(16)));
char tss_ist2[STACK_SIZE] __attribute__((aligned(16)));
char tss_ist3[STACK_SIZE] __attribute__((aligned(16)));

tss ctss;
gdtr cgdt;

void setup_gdt_descriptors(gdt_descriptor_array<7> *target, uintptr_t tss_base, uintptr_t tss_limit)
{
	target->zero();

	target->set(gdt_selector::KERNEL_CODE, gdt_descriptor(gdt_flags::CS | gdt_flags::WRITABLE, gdt_gran::LONG_MODE_GRANULARITY));
	target->set(gdt_selector::KERNEL_DATA, gdt_descriptor(gdt_flags::DS | gdt_flags::WRITABLE, 0));
	target->set(gdt_selector::USER_CODE, gdt_descriptor(gdt_flags::CS | gdt_flags::USER | gdt_flags::WRITABLE, gdt_gran::LONG_MODE_GRANULARITY));
	target->set(gdt_selector::USER_CODE, gdt_descriptor(gdt_flags::CS | gdt_flags::USER | gdt_flags::WRITABLE, 0));

	target->xset(gdt_selector::TSS_SELECTOR, gdt_xdescriptor(gdt_flags::TSS, tss_base, tss_limit));

	gdt_descriptors.fill_gdt_register(&cgdt);
}

void GDT::Init()
{
	uintptr_t tss_base = (uintptr_t)&ctss;
	uintptr_t tss_limit = tss_base + sizeof(ctss);

	setup_gdt_descriptors(&gdt_descriptors, tss_base, tss_limit);

	loadGdt((uint64_t)&cgdt);
}