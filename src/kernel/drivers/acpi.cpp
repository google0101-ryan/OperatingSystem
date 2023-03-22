#include "acpi.h"
#include <mem/vmm.h>
#include <drivers/vga.hpp>
#include <drivers/ioapic.h>
#include <stddef.h>

struct RSDPDescriptor 
{
	char Signature[8];
	uint8_t Checksum;
	char OEMID[6];
	uint8_t Revision;
	uint32_t RsdtAddress;
} __attribute__ ((packed));

struct RSDPDescriptor20 
{
	RSDPDescriptor firstPart;
	
	uint32_t Length;
	uint64_t XsdtAddress;
	uint8_t ExtendedChecksum;
	uint8_t reserved[3];
} __attribute__ ((packed));

struct SDTHeader
{
	char Signature[4];
	uint32_t Length;
	uint8_t Revision;
	uint8_t Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
} __attribute__((packed));

int strncmp(const char *X, const char *Y, int len)
{
    while (*X && len--)
    {
        // if characters differ, or end of the second string is reached
        if (*X != *Y) {
            break;
        }
 
        // move to the next pair of characters
        X++;
        Y++;
    }
 
    // return the ASCII difference after converting `char*` to `unsigned char*`
    return *(const unsigned char*)X - *(const unsigned char*)Y;
}

bool ValidateChecksum(RSDPDescriptor *rsdp)
{
	int checksum = 0;
	uint8_t* rsdp_bytes = (uint8_t*)rsdp;

	for (uint64_t i = 0; i < sizeof(RSDPDescriptor); i++)
		checksum += rsdp_bytes[i];
	
	bool v1_valid = (checksum & 0xff) == 0x00;
	return v1_valid;
}

struct APICHeader
{
	SDTHeader hdr;
	uint32_t lapic_addr;
	uint32_t flags;
} __attribute__((packed));

static uint8_t processor_ids[256] = {0};
int cur_id = 0;

uint64_t apic_base = 0;

struct Redirect
{
	uint8_t irq_source;
	uint32_t gsi;
} ioapic_redirects[32];
int cur_redirect = 0;

void ParseAPIC(uint64_t entry)
{
	APICHeader* hdr = (APICHeader*)entry;
	uint8_t* ptr = (uint8_t*)hdr;
	uint8_t* ptr2 = ((uint8_t*)hdr) + hdr->hdr.Length;

	size_t len = ptr2 - (uint8_t*)hdr;
	size_t pos = 0x2C;
	while (pos < len)
	{
		uint8_t type = ptr[pos++];
		uint8_t length = ptr[pos++];

		switch (type)
		{
		case 0:
		{
			processor_ids[cur_id++] = ptr[pos++];
			uint8_t apic_id = ptr[pos++];
			uint32_t flags = *(uint32_t*)&ptr[pos];
			pos += 4;
			break;
		}
		case 1:
		{
			uint8_t ioapic_id = ptr[pos++];
			pos++;
			uint32_t ioapic_addr = *(uint32_t*)&ptr[pos];
			pos += 4;
			uint8_t interrupt_base = *(uint32_t*)&ptr[pos];
			pos += 4;
			apic_base = ioapic_addr;
			break;
		}
		case 2:
		{
			uint8_t bus_source = ptr[pos++];
			uint8_t irq_source = ptr[pos++];
			uint32_t gsi = *(uint32_t*)&ptr[pos];
			pos += 4;
			uint16_t flags = *(uint16_t*)&ptr[pos];
			pos += 2;
			ioapic_redirects[cur_redirect].irq_source = irq_source;
			ioapic_redirects[cur_redirect++].gsi = gsi;
			break;
		}
		case 4:
		{
			uint8_t acpi_id = ptr[pos++];
			uint16_t flags = *(uint16_t*)&ptr[pos];
			pos += 2;
			uint8_t lint_num = ptr[pos++];
			break;
		}
		default:
			printf("Unknown APIC entry type %d\n", type);
			for (;;) asm volatile("hlt");
		}
	}
}

void ACPI::FindTables(stivale2_struct* hdr)
{
	auto rsdp = (stivale2_struct_tag_rsdp*)get_tag(hdr, STIVALE2_STRUCT_TAG_RSDP_ID);

	if (!rsdp)
	{
		printf("Invalid rsdp tag!\n");
	}

	printf("RSDP is at 0x%x\n", rsdp);

	RSDPDescriptor* rsdpd = (RSDPDescriptor*)rsdp->rsdp;

	if (!ValidateChecksum(rsdpd))
	{
		printf("Invalid rsdp checksum!\n");
	}

	printf("RSDT at 0x%x\n", rsdpd->RsdtAddress);

	SDTHeader* rsdt = (SDTHeader*)(uint64_t)rsdpd->RsdtAddress;

	int NumTables = (rsdt->Length - sizeof(rsdt)) / 4;
	uint32_t* SDTPointers = (uint32_t*)((char*)rsdt + sizeof(rsdt));

	for (int i = 0; i < NumTables; i++)
	{
		SDTHeader* hdr = (SDTHeader*)SDTPointers[i];

		char buf[5];
		buf[0] = hdr->Signature[0];
		buf[1] = hdr->Signature[1];
		buf[2] = hdr->Signature[2];
		buf[3] = hdr->Signature[3];
		buf[4] = '\0';

		if (buf[0] == 'A' && buf[1] == 'P' && buf[2] == 'I' && buf[3] == 'C')
		{
			printf("Found APIC entry at 0x%x\n", SDTPointers[i]);
			ParseAPIC((uint64_t)hdr);
		}
	}
}

void ACPI::SetupAPIC()
{
	ioapic = new IOAPIC(apic_base);

	printf("[IOAPIC]: Setting up %d redirects\n", cur_redirect);

	for (int i = 0; i < cur_redirect; i++)
	{
		ioapic->Redirect(ioapic_redirects[i].gsi, ioapic_redirects[i].irq_source + 0x20, 0);
	}
}
