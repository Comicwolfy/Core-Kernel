#include <stdint.h>
#include <stddef.h>
#include "base_kernel.h"

static int timer_ext_id = -1;

static volatile uint64_t ticks = 0;

void timer_handler_c() {
    ticks++;
    outb(0x20, 0x20);
}

void cmd_uptime(const char* args) {
    terminal_writestring("System Uptime: ");
    char num_str[20];
    uint64_t current_ticks = ticks;

    int i = 0;
    if (current_ticks == 0) { num_str[0] = '0'; i = 1; }
    else { uint64_t temp = current_ticks; while (temp > 0) { num_str[i++] = (temp % 10) + '0'; temp /= 10; } }
    num_str[i] = '\0';
    for (int start = 0, end = i - 1; start < end; start++, end--) { char temp = num_str[start]; num_str[start] = num_str[end]; num_str[end] = temp; }
    terminal_writestring(num_str);
    terminal_writestring(" ticks\n");

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

int timer_extension_init(void) {
    terminal_writestring("Timer Extension: Initializing...\n");

    uint16_t divisor = 11932;

    outb(0x43, 0x36);

    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));

    terminal_writestring("Timer Extension: PIT configured for ~100 Hz.\n");
    terminal_writestring("Timer Extension: Uptime counter active.\n");

    register_command("uptime", cmd_uptime, "Display system uptime", timer_ext_id);

    return 0;
}

void timer_extension_cleanup(void) {
    terminal_writestring("Timer Extension: Cleaning up...\n");
    terminal_writestring("Timer Extension: Cleanup complete.\n");
}

__attribute__((section(".ext_register_fns")))
void __timer_auto_register(void) {
    timer_ext_id = register_extension("Timer", "1.0",
                                      timer_extension_init,
                                      timer_extension_cleanup);
    if (timer_ext_id >= 0) {
        load_extension(timer_ext_id);
    } else {
        terminal_writestring("Failed to register Timer Extension (auto)!\n");
    }
}
