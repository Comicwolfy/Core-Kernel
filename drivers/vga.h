#ifndef VGA_H
#define VGA_H

#include <stddef.h>

void vga_clear();
void vga_putc(char c);
void vga_puts(const char* str);

#endif
