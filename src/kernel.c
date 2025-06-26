#include <stdint.h>
#include <stddef.h>
#include "base_kernel.h" // Includes our core declarations for auto-discovery and APIs

// --- Core Kernel Static Variables ---
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// Memory Management Globals
#define MEMORY_BLOCK_SIZE 4096
#define MAX_MEMORY_BLOCKS 1024

typedef struct memory_block {
    void* address;
    size_t size;
    int is_free;
    struct memory_block* next;
} memory_block_t;

static memory_block_t memory_blocks[MAX_MEMORY_BLOCKS];
static memory_block_t* free_list = NULL;
static int memory_initialized = 0;

// Extension system globals
#define MAX_EXTENSIONS 32
#define MAX_COMMANDS 64
#define MAX_COMMAND_NAME 16

// `extension_t` and `command_t` are defined in `base_kernel.h` now.
// Remove duplicate definitions here if they were present.
static extension_t extensions[MAX_EXTENSIONS];
static command_t commands[MAX_COMMANDS];
static int extension_count = 0;
static int command_count = 0;

// --- Utility functions ---
// `vga_entry_color` and `vga_entry` are now in `base_kernel.h` as `static inline`.

// NOTE: This strlen is a simple implementation. A more robust C standard library would provide this.
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// --- Terminal functions ---
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t src_index = (y + 1) * VGA_WIDTH + x;
            const size_t dst_index = y * VGA_WIDTH + x;
            terminal_buffer[dst_index] = terminal_buffer[src_index];
        }
    }

    // Clear the last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// --- Memory Management functions ---
void memory_initialize(void) {
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        memory_blocks[i].address = NULL;
        memory_blocks[i].size = 0;
        memory_blocks[i].is_free = 1;
        memory_blocks[i].next = NULL;
    }

    // Set up initial free block
    memory_blocks[0].address = (void*)0x100000; // 1MB
    memory_blocks[0].size = 0x100000; // 1MB
    memory_blocks[0].is_free = 1;
    free_list = &memory_blocks[0];

    memory_initialized = 1;
}

void* kmalloc(size_t size) {
    if (!memory_initialized) {
        memory_initialize();
    }

    size = (size + MEMORY_BLOCK_SIZE - 1) & ~(MEMORY_BLOCK_SIZE - 1);

    memory_block_t* current = free_list;
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            current->is_free = 0;

            if (current->size > size) {
                for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
                    if (memory_blocks[i].address == NULL) {
                        memory_blocks[i].address = (char*)current->address + size;
                        memory_blocks[i].size = current->size - size;
                        memory_blocks[i].is_free = 1;
                        memory_blocks[i].next = current->next;
                        current->next = &memory_blocks[i];
                        current->size = size;
                        break;
                    }
                }
            }

            return current->address;
        }
        current = current->next;
    }

    return NULL; // Out of memory
}

void kfree(void* ptr) {
    if (ptr == NULL) return;

    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        if (memory_blocks[i].address == ptr) {
            memory_blocks[i].is_free = 1;
            // TODO: Implement block coalescing
            break;
        }
    }
}

// --- Extension Management API ---
int register_extension(const char* name, const char* version,
                       int (*init_func)(void), void (*cleanup_func)(void)) {
    if (extension_count >= MAX_EXTENSIONS) {
        return -1; // Extension table full
    }

    extension_t* ext = &extensions[extension_count];

    int i;
    for (i = 0; i < 31 && name[i] != '\0'; i++) {
        ext->name[i] = name[i];
    }
    ext->name[i] = '\0';

    for (i = 0; i < 15 && version[i] != '\0'; i++) {
        ext->version[i] = version[i];
    }
    ext->version[i] = '\0';

    ext->init = init_func;
    ext->cleanup = cleanup_func;
    ext->active = 0;

    extension_count++;
    return extension_count - 1;
}

int load_extension(int ext_id) {
    if (ext_id < 0 || ext_id >= extension_count) {
        return -1;
    }

    extension_t* ext = &extensions[ext_id];
    if (ext->active) {
        return 0; // Already active
    }

    if (ext->init && ext->init() != 0) {
        return -1; // Initialization failed
    }

    ext->active = 1;
    return 0;
}

int unload_extension(int ext_id) {
    if (ext_id < 0 || ext_id >= extension_count) {
        return -1;
    }

    extension_t* ext = &extensions[ext_id];
    if (!ext->active) {
        return 0; // Already inactive
    }

    if (ext->cleanup) {
        ext->cleanup();
    }

    ext->active = 0;
    return 0;
}

// --- Command Registration API ---
int register_command(const char* name, void (*handler)(const char*),
                     const char* description, int ext_id) {
    if (command_count >= MAX_COMMANDS) {
        return -1;
    }

    if (ext_id >= 0 && ext_id < extension_count) {
        commands[command_count].owner = &extensions[ext_id];
    } else {
        commands[command_count].owner = NULL; // Core command
    }

    int i;
    for (i = 0; i < MAX_COMMAND_NAME - 1 && name[i] != '\0'; i++) {
        commands[command_count].name[i] = name[i];
    }
    commands[command_count].name[i] = '\0';

    for (i = 0; i < 63 && description[i] != '\0'; i++) {
        commands[command_count].description[i] = description[i];
    }
    commands[command_count].description[i] = '\0';

    commands[command_count].handler = handler;
    command_count++;
    return 0;
}

command_t* find_command(const char* name) {
    for (int i = 0; i < command_count; i++) {
        // Simple string comparison
        int match = 1;
        size_t name_len = strlen(name);
        if (name_len > MAX_COMMAND_NAME -1) name_len = MAX_COMMAND_NAME -1; // Cap length

        for (size_t j = 0; j < name_len; j++) {
            if (commands[i].name[j] != name[j]) {
                match = 0;
                break;
            }
        }
        if (match && commands[i].name[name_len] == '\0') { // Ensure exact match
            return &commands[i];
        }
    }
    return NULL;
}

// --- Core Command Handlers ---
void cmd_help(const char* args) {
    terminal_writestring("BASE Kernel Commands:\n");
    terminal_writestring("====================\n");

    for (int i = 0; i < command_count; i++) {
        terminal_writestring("  ");
        terminal_writestring(commands[i].name);
        terminal_writestring(" - ");
        terminal_writestring(commands[i].description);
        if (commands[i].owner) {
            terminal_writestring(" [");
            terminal_writestring(commands[i].owner->name);
            terminal_writestring("]");
        }
        terminal_writestring("\n");
    }
}

void cmd_info(const char* args) {
    terminal_writestring("=== BASE KERNEL v1.0 ===\n");
    terminal_writestring("Minimal extensible kernel core\n\n");
    terminal_writestring("System Information:\n");
    terminal_writestring("- Architecture: x86\n");
    terminal_writestring("- Memory Management: Basic allocator\n");
    terminal_writestring("- Terminal: VGA text mode\n");
    terminal_writestring("- Extensions: Supported (Auto-discovery)\n");
    terminal_writestring("- Status: Running\n\n");
}

void cmd_extensions(const char* args) {
    terminal_writestring("Loaded Extensions:\n");
    terminal_writestring("==================\n");

    int active_count = 0;
    for (int i = 0; i < extension_count; i++) {
        if (extensions[i].active) {
            terminal_writestring("  ");
            terminal_writestring(extensions[i].name);
            terminal_writestring(" v");
            terminal_writestring(extensions[i].version);
            terminal_writestring(" [ACTIVE]\n");
            active_count++;
        }
    }

    if (active_count == 0) {
        terminal_writestring("  No extensions loaded\n");
    }

    terminal_writestring("\nAvailable Extensions (Not Loaded):\n");
    int available_count = 0;
    for (int i = 0; i < extension_count; i++) {
        if (!extensions[i].active) {
            terminal_writestring("  ");
            terminal_writestring(extensions[i].name);
            terminal_writestring(" v");
            terminal_writestring(extensions[i].version);
            terminal_writestring(" [AVAILABLE]\n");
            available_count++;
        }
    }
    if (available_count == 0 && active_count > 0) {
        terminal_writestring("  All available extensions are loaded.\n");
    } else if (available_count == 0 && active_count == 0) {
         terminal_writestring("  No extensions registered.\n");
    }
}

void cmd_mem(const char* args) {
    terminal_writestring("Memory Status:\n");
    terminal_writestring("- Memory manager: Active\n");
    terminal_writestring("- Basic allocation in 0x100000 - 0x1FFFFF region\n");
    terminal_writestring("- Free blocks available (coalescing TODO)\n");
}

void cmd_clear(const char* args) {
    terminal_initialize();
    terminal_writestring("BASE Kernel v1.0 - Extension Ready\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
}

void init_core_commands(void) {
    register_command("help", cmd_help, "Show available commands", -1);
    register_command("info", cmd_info, "System information", -1);
    register_command("ext", cmd_extensions, "List extensions", -1);
    register_command("mem", cmd_mem, "Memory status", -1);
    register_command("clear", cmd_clear, "Clear screen", -1);
}

void process_command(const char* input) {
    if (strlen(input) == 0) return;

    char command[MAX_COMMAND_NAME];
    const char* args = input;
    int i = 0;

    while (input[i] != '\0' && input[i] != ' ' && i < MAX_COMMAND_NAME - 1) {
        command[i] = input[i];
        i++;
    }
    command[i] = '\0';

    while (input[i] == ' ') i++;
    args = &input[i];

    terminal_writestring("$ ");
    terminal_writestring(input);
    terminal_writestring("\n");

    // Find and execute command
    command_t* cmd = find_command(command);
    if (cmd) {
        if (cmd->owner && !cmd->owner->active) {
            terminal_writestring("Error: Extension '");
            terminal_writestring(cmd->owner->name);
            terminal_writestring("' is not loaded\n");
        } else {
            cmd->handler(args);
        }
    } else {
        terminal_writestring("Unknown command: ");
        terminal_writestring(command);
        terminal_writestring("\nType 'help' for available commands.\n");
    }

    terminal_writestring("\n");
}


// --- Main Kernel Entry Point ---
void kernel_main(void) {
    terminal_initialize();
    memory_initialize();

    // Initialize core extension and command systems
    extension_count = 0;
    command_count = 0;
    init_core_commands();

    // Display boot message
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("BASE KERNEL LOADING...\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    terminal_writestring("=== BASE KERNEL v1.0 ===\n");
    terminal_writestring("Extensible kernel core initialized\n\n");

    // --- AUTOMATIC EXTENSION DISCOVERY AND INITIALIZATION ---
    initialize_all_extensions(); // This function (from extension_bootstrap.c) will load all extensions
    // --- END AUTOMATIC DISCOVERY ---

    // Welcome message
    terminal_writestring("Welcome to BASE kernel!\n");
    terminal_writestring("This is the minimal core. Extensions add functionality.\n");
    terminal_writestring("Type 'help' for available commands.\n\n");

    // Initial commands for demonstration
    process_command("help");
    process_command("ext");
    process_command("info");

    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("BASE kernel core ready for interaction!\n");
    terminal_writestring("Awaiting keyboard input via 'cli_test' or other extension commands.\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    // Kernel main loop
    while (1) {
        // Halt and wait for interrupts.
        // The Keyboard Extension (IRQ & KB Ext) will handle input via interrupts
        // and its 'cli_test' command can then be used for interactive input.
        asm volatile("hlt");
    }
}

// The entry point for the kernel, called by the bootloader
void _start(void) {
    kernel_main();
}
