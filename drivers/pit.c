#include "pit.h"
#include <stdint.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void pit_init(uint32_t frequency) {
    uint16_t divisor = PIT_FREQUENCY / frequency;

    outb(PIT_COMMAND, 0x36); // channel 0, lobyte/hibyte, mode 3 (square wave)
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}
