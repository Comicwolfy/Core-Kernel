#include <stdint.h>
#include <stddef.h>
#include "base_kernel.h"

static int shell_ext_id = -1;

void cmd_shell_handler(const char* args) {
    terminal_writestring("BASE Shell (Type 'exit' or Ctrl+C to leave, 'help' for commands):\n");

    char input_buffer[VGA_WIDTH + 1];
    size_t input_idx = 0;

    while (1) {
        terminal_writestring("kernel> ");

        while (1) {
            char c = wait_for_char_from_kb_buffer();

            if (c == '\n' || c == '\r') {
                input_buffer[input_idx] = '\0';
                terminal_putchar('\n');

                if (input_idx == 4 &&
                    input_buffer[0] == 'e' && input_buffer[1] == 'x' &&
                    input_buffer[2] == 'i' && input_buffer[3] == 't') {
                    terminal_writestring("Exiting shell.\n");
                    return;
                }

                if (input_idx > 0) {
                    process_command(input_buffer);
                }
                input_idx = 0;
                break;
            } else if (c == '\b' || c == 0x7F) {
                if (input_idx > 0) {
                    input_idx--;
                    terminal_putchar('\b');
                    terminal_putchar(' ');
                    terminal_putchar('\b');
                }
            } else if (c == 0x03) {
                terminal_writestring("^C\nExiting shell.\n");
                return;
            } else if (input_idx < VGA_WIDTH) {
                input_buffer[input_idx++] = c;
                terminal_putchar(c);
            }
        }
    }
}

int shell_extension_init(void) {
    terminal_writestring("Shell Extension: Initializing...\n");

    register_command("shell", cmd_shell_handler, "Start an interactive kernel shell", shell_ext_id);

    terminal_writestring("Shell Extension: Command 'shell' registered.\n");
    return 0;
}

void shell_extension_cleanup(void) {
    terminal_writestring("Shell Extension: Cleaning up...\n");
    terminal_writestring("Shell Extension: Cleanup complete.\n");
}

__attribute__((section(".ext_register_fns")))
void __shell_auto_register(void) {
    shell_ext_id = register_extension("Shell", "1.0",
                                      shell_extension_init,
                                      shell_extension_cleanup);
    if (shell_ext_id >= 0) {
        load_extension(shell_ext_id);
    } else {
        terminal_writestring("Failed to register Shell Extension (auto)!\n");
    }
}
