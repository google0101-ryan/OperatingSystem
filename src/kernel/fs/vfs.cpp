#include "vfs.h"
#include <drivers/vga.hpp>

Filesystem::Filesystem()
{
	*name = '\0';
}

VFS* VFS::instance = nullptr;

VFS *VFS::the()
{
	if (!instance)
		instance = new VFS();
	return instance;
}

int strcmp(const char *X, const char *Y)
{
    while (*X)
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

void VFS::Initialize(stivale2_struct* root)
{
	auto mods = (stivale2_struct_tag_modules*)get_tag(root, STIVALE2_STRUCT_TAG_MODULES_ID);
	stivale2_module* initrd = nullptr;

	for (uint64_t i = 0; i < mods->module_count; i++)
	{
		auto* mod = &mods->modules[i];
		if (!strcmp(mod->string, "initrd.img"))
		{
			initrd = mod;
			break;
		}
	}

	if (!initrd)
	{
		printf("ERROR: Failed to find initrd!\n");
		VGA::panic("VFS::Initialize", __LINE__);
	}
}
