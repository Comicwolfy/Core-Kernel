#include <stdint.h>
#include "drivers/keyboard.h"
#include "drivers/pit.h"
#include "drivers/serial.h"
#include "drivers/vga.h"
#include "drivers/rtc.h"
#include "drivers/ata.h"
#include "drivers/mm.h"
// #include "shell.h"  // Add this include for the shell (temporarily commented)

#define HEAP_START 0x200000
#define HEAP_SIZE 0x100000 // 1 MB heap

// Kernel initialization status tracking
typedef struct {
    uint8_t vga_ready;
    uint8_t serial_ready;
    uint8_t keyboard_ready;
    uint8_t pit_ready;
    uint8_t rtc_ready;
    uint8_t ata_ready;
    uint8_t mm_ready;
} kernel_status_t;

static kernel_status_t kernel_status = {0};

// Initialize all kernel subsystems
int init_kernel_subsystems(void) {
    // Clear screen first
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
    vga_puts("Version: 0.1-dev\n");
    vga_puts("Architecture: x86_64\n");
    
    char heap_info[64];
    // Assuming you have sprintf or similar function
    vga_puts("Heap: 0x200000 - 0x300000 (1MB)\n");
    
    vga_puts("\nDriver Status:\n");
    vga_puts(kernel_status.vga_ready ? "VGA: Ready\n" : "VGA: Failed\n");
    vga_puts(kernel_status.serial_ready ? "Serial: Ready\n" : "Serial: Failed\n");
    vga_puts(kernel_status.keyboard_ready ? "Keyboard: Ready\n" : "Keyboard: Failed\n");
    vga_puts(kernel_status.pit_ready ? "PIT: Ready\n" : "PIT: Failed\n");
    vga_puts(kernel_status.rtc_ready ? "RTC: Ready\n" : "RTC: Failed\n");
    vga_puts(kernel_status.ata_ready ? "ATA: Ready\n" : "ATA: Failed\n");
    vga_puts(kernel_status.mm_ready ? "Memory: Ready\n" : "Memory: Failed\n");
    vga_puts("========================\n\n");
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
    
    // Halt the system
    while (1) {
        asm volatile ("hlt");
    }
}

// Main kernel entry point
void kernel_main(void) {
    // Initialize all kernel subsystems
    if (init_kernel_subsystems() != 0) {
        kernel_panic("Failed to initialize kernel subsystems");
    }
    
    // Print welcome message and system info
    vga_puts("\nWelcome to Core Kernel x64!\n");
    serial_write_string("Core Kernel x64 fully initialized\n");
    
    print_kernel_info();
    
    // Check if we have minimum requirements for shell
    if (!kernel_status.vga_ready || !kernel_status.keyboard_ready) {
        kernel_panic("Cannot start shell - VGA or keyboard not available");
    }
    
    vga_puts("Kernel fully initialized!\n");
    serial_write_string("Core Kernel x64 ready - shell not yet integrated\n");
    
    // TODO: Start the interactive shell when ready
    // start_shell();
    
    // For now, just halt and wait for interrupts
    vga_puts("System ready. Halting CPU...\n");
    while (1) {
        asm volatile ("hlt");
    }
}