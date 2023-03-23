#pragma once

#include <stdint.h>
#include <stddef.h>
#include <lib/hashmap.h>
#include <stivale2.h>

class Filesystem
{
protected:
	char name[128]; // ISO9660, EXT2, FAT32, etc
public:
	Filesystem();

	virtual int open(const char* name) = 0;
	virtual void close(int fd) = 0;
	virtual int write(int fd, uint8_t* buf, size_t size) = 0;
	virtual int read(int fd, uint8_t* buf, size_t size) = 0;
	virtual int seek(int fd, size_t off) = 0;
};

class VFS
{
private:
	static VFS* instance;

	HashMap<const char*, Filesystem*> mountpoints;
public:
	static VFS* the();

	void Initialize(stivale2_struct* root);
};