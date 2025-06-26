#include <stdint.h>
#include <stddef.h>
#include "base_kernel.h" // Includes declarations for auto-discovery and kernel APIs

// --- Extension ID ---
static int timer_ext_id = -1;

// --- Global Tick Counter ---
static volatile uint64_t ticks = 0;

// --- PIT Interrupt Handler (C part) ---
void timer_handler_c() {
    ticks++;
    // Acknowledge the interrupt to the Master PIC
    outb(0x20, 0x20);
}

// --- Command Handler for 'uptime' ---
void cmd_uptime(const char* args) {
    terminal_writestring("System Uptime: ");
    char num_str[20];
    uint64_t current_ticks = ticks;

    // Simple uint64_t to string conversion
    int i = 0;
    if (current_ticks == 0) { num_str[0] = '0'; i = 1; }
    else { uint64_t temp = current_ticks; while (temp > 0) { num_str[i++] = (temp % 10) + '0'; temp /= 10; } }
    num_str[i] = '\0';
    for (int start = 0, end = i - 1; start < end; start++, end--) { char temp = num_str[start]; num_str[start] = num_str[end]; num_str[end] = temp; }
    terminal_writestring(num_str);
    terminal_writestring(" ticks\n");

    // Convert to seconds for better readability (assuming 100 Hz as configured)
    uint64_t seconds = current_ticks / 100;
    terminal_writestring(" (~");

    i = 0;
    if (seconds == 0) { num_str[0] = '0'; i = 1; }
    else { uint64_t temp = seconds; while (temp > 0) { num_str[i++] = (temp % 10) + '0'; temp /= 10; } }
    num_str[i] = '\0';
    for (int start = 0, end = i - 1; start < end; start++, end--) { char temp = num_str[start]; num_str[start] = num_str[end]; num_str[end] = temp; }
    terminal_writestring(num_str);
    terminal_writestring(" seconds)\n");
}

// --- Extension Initialization and Cleanup ---
int timer_extension_init(void) {
    terminal_writestring("Timer Extension: Initializing...\n");

    // PIT frequency is 1193180 Hz.
    // For 100 Hz, divisor = 11931.8 -> use 11932
    uint16_t divisor = 11932; // For ~100 Hz

    // Command Register (0x43):
    // Channel 0 (00), Access Mode: Low/High byte (11), Operating Mode: Square Wave Generator (011), BCD: Binary (0)
    // 00110110b = 0x36
    outb(0x43, 0x36); // Set our command byte.

    // Data Register (0x40): Send divisor (low byte then high byte)
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));

    // Ensure IRQ0 is unmasked in the PIC (this is done by irq_kb_extension.c,
    // but a check or a re-unmask here might be useful if they were separate)

    terminal_writestring("Timer Extension: PIT configured for ~100 Hz.\n");
    terminal_writestring("Timer Extension: Uptime counter active.\n");

    // Register uptime command
    register_command("uptime", cmd_uptime, "Display system uptime", timer_ext_id);

    return 0; // Success
}

void timer_extension_cleanup(void) {
    terminal_writestring("Timer Extension: Cleaning up...\n");
    // Optionally disable PIT interrupts or set it to a safe default mode
    terminal_writestring("Timer Extension: Cleanup complete.\n");
}

// --- AUTO-REGISTRATION FUNCTION ---
// This function pointer will be placed in the special .ext_register_fns section by the linker
// and automatically called by initialize_all_extensions() in the kernel core.
__attribute__((section(".ext_register_fns")))
void __timer_auto_register(void) {
    timer_ext_id = register_extension("Timer", "1.0",
                                      timer_extension_init,
                                      timer_extension_cleanup);
    if (timer_ext_id >= 0) {
        load_extension(timer_ext_id); // Load it automatically upon discovery
    } else {
        terminal_writestring("Failed to register Timer Extension (auto)!\n");
    }
}
