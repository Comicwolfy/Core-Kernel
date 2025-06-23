// kernel/gdt.h - Fixed for 64-bit
#ifndef GDT_H
#define GDT_H

#include <stdint.h>

void gdt_install();

#endif

// kernel/gdt.c - Fixed for 64-bit
#include "gdt.c"

// 64-bit GDT entry structure
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

// GDT pointer structure
struct gdt_ptr {
    uint16_t limit;
    uint64_t base;  // 64-bit base address
} __attribute__((packed));

// 64-bit mode needs fewer segments
struct gdt_entry gdt[5];  // null, kernel code, kernel data, user code, user data
struct gdt_ptr gp;

extern void gdt_flush(uint64_t);

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

void gdt_install() {
    gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gp.base = (uint64_t)&gdt;

    // Null segment
    gdt_set_gate(0, 0, 0, 0, 0);
    
    // Kernel code segment (64-bit)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xAF);  // 0xAF for 64-bit code
    
    // Kernel data segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    
    // User code segment (64-bit)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xAF);  // Ring 3, 64-bit code
    
    // User data segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);  // Ring 3, data

    gdt_flush((uint64_t)&gp);
}