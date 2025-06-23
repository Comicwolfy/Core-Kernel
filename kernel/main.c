#include <stdint.h>
#include "drivers/keyboard.h"
#include "drivers/pit.h"
#include "drivers/serial.h"
#include "drivers/vga.h"
#include "drivers/rtc.h"
#include "drivers/ata.h"
#include "drivers/mm.h"

#define HEAP_START 0x200000
#define HEAP_SIZE  0x100000  // 1 MB heap

void kernel_main() {
    vga_clear();
    serial_init();
    keyboard_init();
    pit_init(1000);
    rtc_init();
    ata_init();
    mm_init((void*)HEAP_START, HEAP_SIZE);

    vga_puts("Welcome to Core Kernel x64!\n");
    serial_write_string("Welcome to Core Kernel x64!\n");

    while (1) {
        __asm__ volatile ("hlt");
    }
}
