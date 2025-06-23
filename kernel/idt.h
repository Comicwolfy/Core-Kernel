// kernel/idt.h - Complete 64-bit IDT
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// IDT entry structure for 64-bit
struct idt_entry {
    uint16_t offset_low;    // Lower 16 bits of handler address
    uint16_t selector;      // Kernel segment selector
    uint8_t  ist;          // Interrupt Stack Table offset (0 for now)
    uint8_t  type_attr;    // Type and attributes
    uint16_t offset_mid;    // Middle 16 bits of handler address
    uint32_t offset_high;   // Upper 32 bits of handler address
    uint32_t reserved;      // Reserved, must be 0
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

// Interrupt frame structure
struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void idt_install();
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags);

// Exception handlers
void divide_error_handler();
void debug_handler();
void nmi_handler();
void breakpoint_handler();
void overflow_handler();
void bound_range_exceeded_handler();
void invalid_opcode_handler();
void device_not_available_handler();
void double_fault_handler();
void invalid_tss_handler();
void segment_not_present_handler();
void stack_segment_fault_handler();
void general_protection_fault_handler();
void page_fault_handler();
void fpu_error_handler();
void alignment_check_handler();
void machine_check_handler();
void simd_fp_exception_handler();

// IRQ handlers
void irq0_handler();  // Timer
void irq1_handler();  // Keyboard
void irq2_handler();  // Cascade
void irq3_handler();  // COM2
void irq4_handler();  // COM1
void irq5_handler();  // LPT2
void irq6_handler();  // Floppy
void irq7_handler();  // LPT1
void irq8_handler();  // RTC
void irq9_handler();  // Free
void irq10_handler(); // Free
void irq11_handler(); // Free
void irq12_handler(); // PS2 Mouse
void irq13_handler(); // FPU
void irq14_handler(); // Primary ATA
void irq15_handler(); // Secondary ATA

#endif

// kernel/idt.c - Complete implementation
#include "idt.h"
#include "drivers/vga.h"
#include "drivers/serial.h"

static struct idt_entry idt[256];
static struct idt_ptr idtp;

extern void idt_load(uint64_t);

void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags) {
    idt[num].offset_low = handler & 0xFFFF;
    idt[num].selector = selector;
    idt[num].ist = 0;  // No IST for now
    idt[num].type_attr = flags;
    idt[num].offset_mid = (handler >> 16) & 0xFFFF;
    idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].reserved = 0;
}

// Exception handlers implementation
void handle_exception(int exception_num, const char* name, struct interrupt_frame* frame) {
    vga_puts("\n*** EXCEPTION OCCURRED ***\n");
    vga_puts("Exception: ");
    vga_puts(name);
    vga_puts("\n");
    
    // Print register state
    char buffer[64];
    vga_puts("RIP: 0x");
    // Simple hex print (you'd want a proper implementation)
    vga_puts("\nCS: 0x");
    vga_puts("\nRFLAGS: 0x");
    vga_puts("\nRSP: 0x");
    vga_puts("\nSS: 0x");
    vga_puts("\n");
    
    serial_write_string("Exception occurred: ");
    serial_write_string(name);
    serial_write_string("\n");
    
    // Halt system
    asm volatile("cli; hlt");
}

// Individual exception handlers
void divide_error_handler_c(struct interrupt_frame* frame) {
    handle_exception(0, "Divide Error", frame);
}

void debug_handler_c(struct interrupt_frame* frame) {
    handle_exception(1, "Debug", frame);
}

void nmi_handler_c(struct interrupt_frame* frame) {
    handle_exception(2, "Non-Maskable Interrupt", frame);
}

void breakpoint_handler_c(struct interrupt_frame* frame) {
    handle_exception(3, "Breakpoint", frame);
}

void overflow_handler_c(struct interrupt_frame* frame) {
    handle_exception(4, "Overflow", frame);
}

void bound_range_exceeded_handler_c(struct interrupt_frame* frame) {
    handle_exception(5, "Bound Range Exceeded", frame);
}

void invalid_opcode_handler_c(struct interrupt_frame* frame) {
    handle_exception(6, "Invalid Opcode", frame);
}

void device_not_available_handler_c(struct interrupt_frame* frame) {
    handle_exception(7, "Device Not Available", frame);
}

void double_fault_handler_c(struct interrupt_frame* frame) {
    handle_exception(8, "Double Fault", frame);
}

void invalid_tss_handler_c(struct interrupt_frame* frame) {
    handle_exception(10, "Invalid TSS", frame);
}

void segment_not_present_handler_c(struct interrupt_frame* frame) {
    handle_exception(11, "Segment Not Present", frame);
}

void stack_segment_fault_handler_c(struct interrupt_frame* frame) {
    handle_exception(12, "Stack Segment Fault", frame);
}

void general_protection_fault_handler_c(struct interrupt_frame* frame) {
    handle_exception(13, "General Protection Fault", frame);
}

void page_fault_handler_c(struct interrupt_frame* frame) {
    // Get the faulting address from CR2
    uint64_t fault_addr;
    asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
    
    vga_puts("\n*** PAGE FAULT ***\n");
    vga_puts("Faulting address: 0x");
    vga_puts("\n");
    
    handle_exception(14, "Page Fault", frame);
}

void fpu_error_handler_c(struct interrupt_frame* frame) {
    handle_exception(16, "FPU Error", frame);
}

void alignment_check_handler_c(struct interrupt_frame* frame) {
    handle_exception(17, "Alignment Check", frame);
}

void machine_check_handler_c(struct interrupt_frame* frame) {
    handle_exception(18, "Machine Check", frame);
}

void simd_fp_exception_handler_c(struct interrupt_frame* frame) {
    handle_exception(19, "SIMD Floating Point Exception", frame);
}

// IRQ handlers
void irq_handler(int irq_num) {
    // Send EOI to PIC
    if (irq_num >= 8) {
        // Secondary PIC
        asm volatile("outb %0, %1" : : "a"((uint8_t)0x20), "Nd"((uint16_t)0xA0));
    }
    // Primary PIC
    asm volatile("outb %0, %1" : : "a"((uint8_t)0x20), "Nd"((uint16_t)0x20));
}

void irq0_handler_c() {
    // Timer interrupt
    irq_handler(0);
}

void irq1_handler_c() {
    // Keyboard interrupt
    extern void keyboard_handler();
    keyboard_handler();
    irq_handler(1);
}

void irq2_handler_c() { irq_handler(2); }
void irq3_handler_c() { irq_handler(3); }
void irq4_handler_c() { irq_handler(4); }
void irq5_handler_c() { irq_handler(5); }
void irq6_handler_c() { irq_handler(6); }
void irq7_handler_c() { irq_handler(7); }
void irq8_handler_c() { irq_handler(8); }
void irq9_handler_c() { irq_handler(9); }
void irq10_handler_c() { irq_handler(10); }
void irq11_handler_c() { irq_handler(11); }
void irq12_handler_c() { irq_handler(12); }
void irq13_handler_c() { irq_handler(13); }
void irq14_handler_c() { irq_handler(14); }
void irq15_handler_c() { irq_handler(15); }

void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint64_t)&idt;

    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Set up exception handlers (0-31)
    idt_set_gate(0, (uint64_t)divide_error_handler, 0x08, 0x8E);
    idt_set_gate(1, (uint64_t)debug_handler, 0x08, 0x8E);
    idt_set_gate(2, (uint64_t)nmi_handler, 0x08, 0x8E);
    idt_set_gate(3, (uint64_t)breakpoint_handler, 0x08, 0x8E);
    idt_set_gate(4, (uint64_t)overflow_handler, 0x08, 0x8E);
    idt_set_gate(5, (uint64_t)bound_range_exceeded_handler, 0x08, 0x8E);
    idt_set_gate(6, (uint64_t)invalid_opcode_handler, 0x08, 0x8E);
    idt_set_gate(7, (uint64_t)device_not_available_handler, 0x08, 0x8E);
    idt_set_gate(8, (uint64_t)double_fault_handler, 0x08, 0x8E);
    idt_set_gate(10, (uint64_t)invalid_tss_handler, 0x08, 0x8E);
    idt_set_gate(11, (uint64_t)segment_not_present_handler, 0x08, 0x8E);
    idt_set_gate(12, (uint64_t)stack_segment_fault_handler, 0x08, 0x8E);
    idt_set_gate(13, (uint64_t)general_protection_fault_handler, 0x08, 0x8E);
    idt_set_gate(14, (uint64_t)page_fault_handler, 0x08, 0x8E);
    idt_set_gate(16, (uint64_t)fpu_error_handler, 0x08, 0x8E);
    idt_set_gate(17, (uint64_t)alignment_check_handler, 0x08, 0x8E);
    idt_set_gate(18, (uint64_t)machine_check_handler, 0x08, 0x8E);
    idt_set_gate(19, (uint64_t)simd_fp_exception_handler, 0x08, 0x8E);

    // Set up IRQ handlers (32-47)
    idt_set_gate(32, (uint64_t)irq0_handler, 0x08, 0x8E);
    idt_set_gate(33, (uint64_t)irq1_handler, 0x08, 0x8E);
    idt_set_gate(34, (uint64_t)irq2_handler, 0x08, 0x8E);
    idt_set_gate(35, (uint64_t)irq3_handler, 0x08, 0x8E);
    idt_set_gate(36, (uint64_t)irq4_handler, 0x08, 0x8E);
    idt_set_gate(37, (uint64_t)irq5_handler, 0x08, 0x8E);
    idt_set_gate(38, (uint64_t)irq6_handler, 0x08, 0x8E);
    idt_set_gate(39, (uint64_t)irq7_handler, 0x08, 0x8E);
    idt_set_gate(40, (uint64_t)irq8_handler, 0x08, 0x8E);
    idt_set_gate(41, (uint64_t)irq9_handler, 0x08, 0x8E);
    idt_set_gate(42, (uint64_t)irq10_handler, 0x08, 0x8E);
    idt_set_gate(43, (uint64_t)irq11_handler, 0x08, 0x8E);
    idt_set_gate(44, (uint64_t)irq12_handler, 0x08, 0x8E);
    idt_set_gate(45, (uint64_t)irq13_handler, 0x08, 0x8E);
    idt_set_gate(46, (uint64_t)irq14_handler, 0x08, 0x8E);
    idt_set_gate(47, (uint64_t)irq15_handler, 0x08, 0x8E);

    // Load IDT
    idt_load((uint64_t)&idtp);
    
    // Enable interrupts
    asm volatile("sti");
}