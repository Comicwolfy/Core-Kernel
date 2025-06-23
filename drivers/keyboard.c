#include "keyboard.h"
#include <stdbool.h>

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64

// Simple scancode to ASCII map for common keys
static const char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', // 0x00 - 0x07
    '7', '8', '9', '0', '-', '=', '\b', '\t', // 0x08 - 0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', // 0x10 - 0x17
    'o', 'p', '[', ']', '\n', 0, 'a', 's', // 0x18 - 0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 0x20 - 0x27
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', // 0x28 - 0x2F
    'b', 'n', 'm', ',', '.', '/', 0, '*', // 0x30 - 0x37
    0, ' ', 0, 0, 0, 0, 0, 0, // 0x38 - 0x3F
    // rest zeros
};

static volatile char* video = (volatile char*)0xb8000;
static int cursor_pos = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void put_char(char c) {
    if (c == '\n') {
        cursor_pos += 80 - (cursor_pos % 80);
    } else {
        video[cursor_pos * 2] = c;
        video[cursor_pos * 2 + 1] = 0x07;
        cursor_pos++;
    }
}

void keyboard_handler() {
    uint8_t scancode = inb(KBD_DATA_PORT);
    if (scancode & 0x80) {
        // key released, ignore for now
        return;
    }
    char c = scancode_to_ascii[scancode];
    if (c) {
        put_char(c);
    }
}

void keyboard_init() {
    // enable keyboard IRQ in PIC (optional, if PIC init done)
    // setup IRQ handler for keyboard in IDT (needs your interrupt code)
}
