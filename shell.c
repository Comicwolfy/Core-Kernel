#include <stdint.h>
#include <stddef.h>

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Colors
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN 14
#define VGA_COLOR_WHITE 15

// Keyboard scancodes
#define ENTER_KEY 0x1C
#define BACKSPACE_KEY 0x0E
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define LEFT_SHIFT_RELEASE 0xAA
#define RIGHT_SHIFT_RELEASE 0xB6

// Shell constants
#define MAX_INPUT_LENGTH 256
#define MAX_ARGS 16

// Global variables
static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = VGA_COLOR_LIGHT_GREY | VGA_COLOR_BLACK << 4;
static char input_buffer[MAX_INPUT_LENGTH];
static size_t input_length = 0;
static int shift_pressed = 0;

// Scancode to ASCII mapping
static char scancode_to_ascii[] = {
    0,  0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '
};

static char scancode_to_ascii_shift[] = {
    0,  0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
};

// Basic I/O functions
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// String functions
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

int strncmp(const char* a, const char* b, size_t n) {
    while (n && *a && (*a == *b)) {
        ++a;
        ++b;
        --n;
    }
    if (n == 0) return 0;
    return *(unsigned char*)a - *(unsigned char*)b;
}

void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

// Memory functions
void* memset(void* bufptr, int value, size_t size) {
    unsigned char* buf = (unsigned char*) bufptr;
    for (size_t i = 0; i < size; i++)
        buf[i] = (unsigned char) value;
    return bufptr;
}

// Terminal functions
void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = ((uint16_t)terminal_color << 8) | ' ';
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ((uint16_t)terminal_color << 8) | ' ';
    }
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }
    
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            const size_t index = terminal_row * VGA_WIDTH + terminal_column;
            vga_buffer[index] = ((uint16_t)terminal_color << 8) | ' ';
        }
        return;
    }
    
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    vga_buffer[index] = ((uint16_t)terminal_color << 8) | c;
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
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

// Shell command functions
void cmd_help(int argc, char** argv) {
    terminal_writestring("Available commands:\n");
    terminal_writestring("  help    - Show this help message\n");
    terminal_writestring("  clear   - Clear the screen\n");
    terminal_writestring("  echo    - Print arguments\n");
    terminal_writestring("  version - Show kernel version\n");
    terminal_writestring("  reboot  - Restart the system\n");
    terminal_writestring("  halt    - Stop the system\n");
}

void cmd_clear(int argc, char** argv) {
    terminal_clear();
}

void cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        terminal_writestring(argv[i]);
        if (i < argc - 1) terminal_writestring(" ");
    }
    terminal_writestring("\n");
}

void cmd_version(int argc, char** argv) {
    terminal_writestring("Core Kernel v0.1\n");
    terminal_writestring("A simple 64-bit kernel\n");
}

void cmd_reboot(int argc, char** argv) {
    terminal_writestring("Rebooting...\n");
    // Triple fault to reboot
    asm volatile ("int $0x3");
}

void cmd_halt(int argc, char** argv) {
    terminal_writestring("System halted.\n");
    while (1) {
        asm volatile ("hlt");
    }
}

// Command structure
typedef struct {
    const char* name;
    void (*function)(int argc, char** argv);
    const char* description;
} command_t;

static command_t commands[] = {
    {"help", cmd_help, "Show available commands"},
    {"clear", cmd_clear, "Clear the screen"},
    {"echo", cmd_echo, "Print arguments"},
    {"version", cmd_version, "Show kernel version"},
    {"reboot", cmd_reboot, "Restart the system"},
    {"halt", cmd_halt, "Stop the system"},
    {NULL, NULL, NULL}
};

// Parse command line into arguments
int parse_command(char* input, char** argv) {
    int argc = 0;
    char* token = input;
    
    while (*token && argc < MAX_ARGS - 1) {
        // Skip whitespace
        while (*token == ' ' || *token == '\t') token++;
        if (!*token) break;
        
        argv[argc++] = token;
        
        // Find end of token
        while (*token && *token != ' ' && *token != '\t') token++;
        if (*token) *token++ = '\0';
    }
    
    argv[argc] = NULL;
    return argc;
}

// Execute command
void execute_command(char* input) {
    char* argv[MAX_ARGS];
    int argc = parse_command(input, argv);
    
    if (argc == 0) return;
    
    // Find and execute command
    for (int i = 0; commands[i].name; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].function(argc, argv);
            return;
        }
    }
    
    // Command not found
    terminal_writestring("Command not found: ");
    terminal_writestring(argv[0]);
    terminal_writestring("\nType 'help' for available commands.\n");
}

// Keyboard input handling
uint8_t keyboard_get_scancode(void) {
    while (!(inb(0x64) & 1));
    return inb(0x60);
}

char scancode_to_char(uint8_t scancode) {
    if (scancode >= sizeof(scancode_to_ascii)) return 0;
    
    if (shift_pressed) {
        return scancode_to_ascii_shift[scancode];
    } else {
        return scancode_to_ascii[scancode];
    }
}

void handle_keyboard_input(void) {
    uint8_t scancode = keyboard_get_scancode();
    
    // Handle shift keys
    if (scancode == LEFT_SHIFT || scancode == RIGHT_SHIFT) {
        shift_pressed = 1;
        return;
    }
    if (scancode == LEFT_SHIFT_RELEASE || scancode == RIGHT_SHIFT_RELEASE) {
        shift_pressed = 0;
        return;
    }
    
    // Handle key releases (ignore)
    if (scancode & 0x80) return;
    
    // Handle special keys
    if (scancode == ENTER_KEY) {
        terminal_putchar('\n');
        input_buffer[input_length] = '\0';
        if (input_length > 0) {
            execute_command(input_buffer);
        }
        input_length = 0;
        shell_prompt();
        return;
    }
    
    if (scancode == BACKSPACE_KEY) {
        if (input_length > 0) {
            input_length--;
            terminal_putchar('\b');
        }
        return;
    }
    
    // Convert scancode to character
    char c = scancode_to_char(scancode);
    if (c && input_length < MAX_INPUT_LENGTH - 1) {
        input_buffer[input_length++] = c;
        terminal_putchar(c);
    }
}

// Shell prompt
void shell_prompt(void) {
    terminal_writestring("core> ");
}

// Initialize shell
void shell_init(void) {
    terminal_clear();
    terminal_writestring("Core Kernel Shell v0.1\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
    shell_prompt();
}

// Main shell loop
void shell_run(void) {
    shell_init();
    
    while (1) {
        handle_keyboard_input();
    }
}

// Entry point for kernel to call
void start_shell(void) {
    shell_run();
}