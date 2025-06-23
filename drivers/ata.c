#include "ata.h"

#define ATA_DATA       0x1F0
#define ATA_STATUS     0x1F7
#define ATA_COMMAND    0x1F7
#define ATA_SECTOR_CNT 0x1F2
#define ATA_LBA_LOW    0x1F3
#define ATA_LBA_MID    0x1F4
#define ATA_LBA_HIGH   0x1F5
#define ATA_DEVICE     0x1F6

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void ata_init() {
}

int ata_read_sector(uint32_t lba, uint8_t* buffer) {
    // wait for not busy
    while (inb(ATA_STATUS) & 0x80);

    outb(ATA_DEVICE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)(lba & 0xFF));
    outb(ATA_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_COMMAND, 0x20); // read sector command

    // wait for data ready
    while (!(inb(ATA_STATUS) & 0x08));

    for (int i = 0; i < 256; i++) {
        uint16_t data = inb(ATA_DATA);
        buffer[i * 2] = data & 0xFF;
        buffer[i * 2 + 1] = (data >> 8) & 0xFF;
    }

    return 0;
}
