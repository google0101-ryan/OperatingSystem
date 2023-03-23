#include "vga.hpp"
#include <util/ports.h>
#include <lib/spinlock.h>

Spinlock vga_spinlock;

uint16_t* fb_ptr;
uint16_t x, y;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void VGA::Init()
{
	fb_ptr = (uint16_t*)0xb8000;
	x = y = 0;
}

void SetCursorPos()
{
	uint16_t pos = y * VGA_WIDTH + x;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void memcpy(void* dst, void* src, uint64_t size)
{
	uint8_t* dest = (uint8_t*)dst;
	uint8_t* source = (uint8_t*)src;

	for (uint64_t i = 0; i < size; i++)
		dest[i] = source[i];
}

void memset(char* dst, char c, int l)
{
	for (int i = 0; i < l; i++)
		dst[i] = c;
}

void memsetw(char* dst, uint16_t c, int l)
{
	l /= 2;

	uint16_t* dest = (uint16_t*)dst;

	for (int i = 0; i < l; i++)
		dest[i] = c;
}

void Scroll()
{
	for (int i = 0; i < 24; i++)
	{
		for (int m = 0; m < 80; m++)
		{
			fb_ptr[i * 80 + m] = fb_ptr[(i + 1) * 80 + m];
		}
	} 

	for (int i = 0; i < 80; i++)
	{
		uint16_t attrib = 0x0F;
		volatile uint16_t* where;
		where = (volatile uint16_t*)fb_ptr + (24 * VGA_WIDTH + i);
		*where = ' ' | (attrib << 8);
	}
	
	x = 0;
	y = 24;
}

void VGA::putc(char c)
{
	if (c == '\n')
	{
		y++;
		x = 0;
		SetCursorPos();
		return;
	}
	else if (c == '\t')
	{
		x++;
		while (x % 4) x++;
		SetCursorPos();
		return;
	}
	fb_ptr[y * VGA_WIDTH + x] = c | 0x0F00;
	x++;
	if (x >= VGA_WIDTH)
	{
		x = 0;
		y++;
	}
	if (y >= VGA_HEIGHT)
	{
		Scroll();
	}
	SetCursorPos();
}

void VGA::puts(const char *c)
{
	while (*c)
		putc(*c++);
}

void VGA::seek_to(int _x, int _y)
{
	x = _x;
	y = _y;
	SetCursorPos();
}

int VGA::get_x()
{
	return x;
}

int VGA::get_y()
{
	return y;
}

void swap(char& c1, char& c2)
{
	char tmp = c1;
	c1 = c2;
	c2 = tmp;
}

void reverse(char* str, int length)
{
	int start = 0;
	int end = length - 1;
	while (start < end)
	{
		swap(*(str+start), *(str+end));
		start++;
		end++;
	}
}

void itoa(char *buf, unsigned long int n, int base)
{
    unsigned long int tmp;
    int i, j;

    tmp = n;
    i = 0;

    do {
        tmp = n % base;
        buf[i++] = (tmp < 10) ? (tmp + '0') : (tmp + 'a' - 10);
    } while (n /= base);
    buf[i--] = 0;

    for (j = 0; j < i; j++, i--) {
        tmp = buf[j];
        buf[j] = buf[i];
        buf[i] = tmp;
    }
}

char* ltoa(long val, int base)
{	
	static char buf[64] = {0};

	if (!val)
	{
		buf[0] = '0';
		buf[1] = '\0';
		return buf;
	}
	
	int i = 60;
	
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
}

void VGA::vprint_format(const char *fmt, va_list args)
{
	while (*fmt)
	{
		if (*fmt == '%')
		{
			fmt++;

			switch (*fmt++)
			{
			case 'd':
			{
				int d = va_arg(args, int);
				char buf[32] = {0};
				itoa(buf, d, 10);
				puts(buf);
				break;
			}
			case 'x':
			{
				int x = va_arg(args, int);
				char buf[32] = {0};
				itoa(buf, x, 16);
				puts(buf);
				break;
			}
			case 'l':
			{
				long x = va_arg(args, long);
				puts(ltoa(x, 16));
				break;
			}
			}
		}
		else
		{
			putc(*fmt++);
		}
	}
}

void VGA::print_format(const char *fmt, ...)
{
	vga_spinlock.Lock();

	va_list args;
	va_start(args, fmt);

	vprint_format(fmt, args);

	va_end(args);

	vga_spinlock.Release();
}

