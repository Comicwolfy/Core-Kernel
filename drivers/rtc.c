#include "rtc.h"
#include <stdint.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void rtc_init() {
    outb(CMOS_ADDRESS, 0x8A); // disable NMI, select register A
    uint8_t prev = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x8A);
    outb(CMOS_DATA, prev | 0x40); // set bit 6 to enable periodic interrupt
}

uint8_t rtc_read(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}
