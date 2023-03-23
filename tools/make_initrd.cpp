#include <fstream>
#include <vector>
#include <cstring>

struct initrd_header
{
	int num_files;
};

struct initrd_file_header
{
	char name[128];
	size_t length;
};

struct file_cont
{
	char* contents;
	size_t size;
};

std::vector<file_cont> file_contents;

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("Usage: %s <out> <file list>\n", argv[0]);
		return -1;
	}

	std::ofstream out(argv[1]);
	
	initrd_header hdr;
	hdr.num_files = argc - 2;

	out.write((char*)&hdr, sizeof(hdr));

	for (int i = 0; i < hdr.num_files; i++)
	{
		std::ifstream in(argv[i+2], std::ios::ate | std::ios::binary);
		size_t size = in.tellg();
		in.seekg(0, std::ios::beg);

		char* buf = new char[size];

		in.read(buf, size);

		initrd_file_header file_hdr;
		file_hdr.length = size;
		strncpy(file_hdr.name, argv[i+2], 128);

		file_cont contents;
		contents.contents = buf;
		contents.size = size;

		file_contents.push_back(contents);
		
		out.write((char*)&file_hdr, sizeof(initrd_file_header));
	}

	for (int i = 0; i < hdr.num_files; i++)
	{
		out.write((char*)file_contents[i].contents, file_contents[i].size);
	}

	out.close();

	return 0;
}