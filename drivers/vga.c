#include "vga.h"
#include <stddef.h>  // for size_t

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile char*)0xB8000)

static size_t cursor_pos = 0;

void vga_clear() {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        VGA_MEMORY[i] = ' ';
        VGA_MEMORY[i + 1] = 0x07;
    }
    cursor_pos = 0;
}

void vga_putc(char c) {
    if (c == '\n') {
        cursor_pos += VGA_WIDTH - (cursor_pos % VGA_WIDTH);
    } else {
        VGA_MEMORY[cursor_pos * 2] = c;
        VGA_MEMORY[cursor_pos * 2 + 1] = 0x07;
        cursor_pos++;
    }
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}
