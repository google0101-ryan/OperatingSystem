#pragma once

#include <stdint.h>
#include <stdarg.h>

namespace VGA
{

void Init();
void putc(char c);

void puts(const char* c);

void seek_to(int x, int y);
int get_x();
int get_y();

void vprint_format(const char* fmt, va_list args);
void print_format(const char* fmt, ...);

}

#define printf VGA::print_format