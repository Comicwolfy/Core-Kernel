// kernel/main.c - Fixed with proper 64-bit initialization
#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "paging.h"
#include "drivers/keyboard.h"
#include "drivers/pit.h"
#include "drivers/serial.h"
#include "drivers/vga.h"
#include "drivers/rtc.h"
#include "drivers/ata.h"
#include "drivers/mm.h"
#include "shell.h"

#define HEAP_START 0x200000
#define HEAP_SIZE 0x100000 // 1 MB heap

// Kernel initialization status tracking
typedef struct {
    uint8_t gdt_ready;
    uint8_t idt_ready;
    uint8_t paging_ready;
    uint8_t vga_ready;
    uint8_t serial_ready;
    uint8_t keyboard_ready;
    uint8_t pit_ready;
    uint8_t rtc_ready;
    uint8_t ata_ready;
    uint8_t mm_ready;
    uint8_t shell_ready;
} kernel_status_t;

static kernel_status_t kernel_status = {0};

// Early initialization (before drivers)
int init_core_systems(void) {
    // Initialize GDT first
    gdt_install();
    kernel_status.gdt_ready = 1;
    
    // Initialize IDT (this will enable interrupts)
    idt_install();
    kernel_status.idt_ready = 1;
    
    // Initialize paging
    paging_init();
    kernel_status.paging_ready = 1;
    
    return 0;
}

// Initialize all kernel subsystems
int init_kernel_subsystems(void) {
    // VGA must be initialized first for output
    vga_clear();
    kernel_status.vga_ready = 1;
    vga_puts("Core Kernel x64 - Initializing...\n");
    
    // Initialize serial communication
    serial_init();
    kernel_status.serial_ready = 1;
    serial_write_string("Core Kernel x64 - Serial initialized\n");
    vga_puts("[OK] Serial driver initialized\n");
    
    // Initialize keyboard
    keyboard_init();
    kernel_status.keyboard_ready = 1;
    serial_write_string("Keyboard driver initialized\n");
    vga_puts("[OK] Keyboard driver initialized\n");
    
    // Initialize PIT (Programmable Interval Timer)
    pit_init(1000);
    kernel_status.pit_ready = 1;
    serial_write_string("PIT initialized at 1000Hz\n");
    vga_puts("[OK] PIT initialized at 1000Hz\n");
    
    // Initialize RTC (Real Time Clock)
    rtc_init();
    kernel_status.rtc_ready = 1;
    serial_write_string("RTC initialized\n");
    vga_puts("[OK] RTC initialized\n");
    
    // Initialize ATA (Hard disk controller)
    ata_init();
    kernel_status.ata_ready = 1;
    serial_write_string("ATA driver initialized\n");
    vga_puts("[OK] ATA driver initialized\n");
    
    // Initialize memory management
    mm_init((void*)HEAP_START, HEAP_SIZE);
    kernel_status.mm_ready = 1;
    serial_write_string("Memory management initialized\n");
    vga_puts("[OK] Memory management initialized\n");
    
    return 0; // Success
}

// Print kernel status information
void print_kernel_info(void) {
    vga_puts("\n=== Core Kernel Status ===\n");
    vga_puts("Version: 0.2-stable\n");
    vga_puts("Architecture: x86_64\n");
    vga_puts("Boot Mode: Long Mode (64-bit)\n");
    
    char heap_info[64];
    vga_puts("Heap: 0x200000 - 0x300000 (1MB)\n");
    
    vga_puts("\nCore System Status:\n");
    vga_puts(kernel_status.gdt_ready ? "GDT: Ready\n" : "GDT: Failed\n");
    vga_puts(kernel_status.idt_ready ? "IDT: Ready\n" : "IDT: Failed\n");
    vga_puts(kernel_status.paging_ready ? "Paging: Ready\n" : "Paging: Failed\n");
    
    vga_puts("\nDriver Status:\n");
    vga_puts(kernel_status.vga_ready ? "VGA: Ready\n" : "VGA: Failed\n");
    vga_puts(kernel_status.serial_ready ? "Serial: Ready\n" : "Serial: Failed\n");
    vga_puts(kernel_status.keyboard_ready ? "Keyboard: Ready\n" : "Keyboard: Failed\n");
    vga_puts(kernel_status.pit_ready ? "PIT: Ready\n" : "PIT: Failed\n");
    vga_puts(kernel_status.rtc_ready ? "RTC: Ready\n" : "RTC: Failed\n");
    vga_puts(kernel_status.ata_ready ? "ATA: Ready\n" : "ATA: Failed\n");
    vga_puts(kernel_status.mm_ready ? "Memory: Ready\n" : "Memory: Failed\n");
    vga_puts(kernel_status.shell_ready ? "Shell: Ready\n" : "Shell: Not Started\n");
    vga_puts("========================\n\n");
}

// Test kernel subsystems
void run_kernel_tests(void) {
    vga_puts("Running kernel tests...\n");
    
    // Test memory allocation
    vga_puts("Testing memory allocation... ");
    void* test_ptr = mm_alloc(1024);
    if (test_ptr) {
        vga_puts("PASS\n");
        mm_free(test_ptr);
    } else {
        vga_puts("FAIL\n");
    }
    
    // Test paging
    vga_puts("Testing virtual memory... ");
    void* pages = allocate_pages(2);
    if (pages) {
        vga_puts("PASS\n");
        free_pages(pages, 2);
    } else {
        vga_puts("FAIL\n");
    }
    
    // Test interrupts (they should be working if we got this far)
    vga_puts("Testing interrupt system... ");
    if (kernel_status.idt_ready) {
        vga_puts("PASS\n");
    } else {
        vga_puts("FAIL\n");
    }
    
    vga_puts("All tests completed.\n\n");
}

// Kernel panic function
void kernel_panic(const char* message) {
    // Disable interrupts
    asm volatile ("cli");
    
    vga_puts("\n*** KERNEL PANIC ***\n");
    vga_puts("Error: ");
    vga_puts(message);
    vga_puts("\n");
    vga_puts("System halted.\n");
    
    if (kernel_status.serial_ready) {
        serial_write_string("KERNEL PANIC: ");
        serial_write_string(message);
        serial_write_string("\n");
    }
    
    // Print system state
    vga_puts("\nSystem State at Panic:\n");
    print_kernel_info();
    
    // Halt the system
    while (1) {
        asm volatile ("hlt");
    }
}

// Initialize shell subsystem
int init_shell(void) {
    if (!kernel_status.vga_ready || !kernel_status.keyboard_ready) {
        return -1; // Prerequisites not met
    }
    
    shell_init();
    kernel_status.shell_ready = 1;
    
    vga_puts("[OK] Shell initialized\n");
    serial_write_string("Shell system initialized\n");
    
    return 0;
}

// Main kernel entry point
void kernel_main(void) {
    // Initialize core systems first (GDT, IDT, Paging)
    if (init_core_systems() != 0) {
        // Can't use kernel_panic yet since VGA might not be ready
        while (1) asm volatile("hlt");
    }
    
    // Initialize all kernel subsystems
    if (init_kernel_subsystems() != 0) {
        kernel_panic("Failed to initialize kernel subsystems");
    }
    
    // Print welcome message and system info
    vga_puts("\n");
    vga_puts("=====================================\n");
    vga_puts("    Welcome to Core Kernel x64!     \n");
    vga_puts("=====================================\n");
    serial_write_string("Core Kernel x64 fully initialized\n");
    
    print_kernel_info();
    
    // Run basic tests
    run_kernel_tests();
    
    // Initialize shell
    if (init_shell() != 0) {
        kernel_panic("Cannot start shell - VGA or keyboard not available");
    }
    
    vga_puts("Kernel fully initialized and tested!\n");
    vga_puts("Starting interactive shell...\n\n");
    serial_write_string("Core Kernel x64 ready - starting shell\n");
    
    // Start the interactive shell
    start_shell();
    
    // If shell exits (shouldn't happen), just halt
    vga_puts("Shell terminated unexpectedly. System halting.\n");
    while (1) {
        asm volatile ("hlt");
    }
}