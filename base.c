#include <stdint.h>
#include <stddef.h>

// vga constrant
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// vga colors
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// terminal state
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// memory management 
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

// utility functions
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// terminal functions
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

// memory management functions
void memory_initialize(void) {
    // Initialize memory blocks
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++) {
        memory_blocks[i].address = NULL;
        memory_blocks[i].size = 0;
        memory_blocks[i].is_free = 1;
        memory_blocks[i].next = NULL;
    }
    
    // set up initial free block 
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
    
    // align to block boundary
    size = (size + MEMORY_BLOCK_SIZE - 1) & ~(MEMORY_BLOCK_SIZE - 1);
    
    // find free block
    memory_block_t* current = free_list;
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            current->is_free = 0;
            
            // Split block if it's larger than needed
            if (current->size > size) {
                // Find unused block structure
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

void interrupt_handler(void) {
    terminal_writestring("Interrupt received!\n");
}

void print_system_info(void) {
    terminal_writestring("=== BASE KERNEL v1.0 ===\n");
    terminal_writestring("System Information:\n");
    terminal_writestring("- Architecture: x86\n");
    terminal_writestring("- Memory Management: Basic allocator\n");
    terminal_writestring("- Terminal: VGA text mode\n");
    terminal_writestring("- Status: Running\n\n");
}

// command processor
void process_command(const char* command) {
    if (strlen(command) == 0) return;
    
    terminal_writestring("$ ");
    terminal_writestring(command);
    terminal_writestring("\n");
    
    // command parsing
    if (command[0] == 'h' && command[1] == 'e' && command[2] == 'l' && command[3] == 'p') {
        terminal_writestring("Available commands:\n");
        terminal_writestring("  help    - Show this help\n");
        terminal_writestring("  info    - System information\n");
        terminal_writestring("  mem     - Memory status\n");
        terminal_writestring("  clear   - Clear screen\n");
    } else if (command[0] == 'i' && command[1] == 'n' && command[2] == 'f' && command[3] == 'o') {
        print_system_info();
    } else if (command[0] == 'm' && command[1] == 'e' && command[2] == 'm') {
        terminal_writestring("Memory Status:\n");
        terminal_writestring("- Memory manager: Active\n");
        terminal_writestring("- Free blocks available\n");
    } else if (command[0] == 'c' && command[1] == 'l' && command[2] == 'e' && command[3] == 'a' && command[4] == 'r') {
        terminal_initialize();
        print_system_info();
    } else {
        terminal_writestring("Unknown command. Type 'help' for available commands.\n");
    }
    
    terminal_writestring("\n");
}

void kernel_main(void) {
    // Initialize subsystems
    terminal_initialize();
    memory_initialize();

    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("BASE KERNEL LOADING...\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    print_system_info();

    terminal_writestring("Welcome to BASE kernel!\n");
    terminal_writestring("Type 'help' for available commands.\n\n");

    process_command("help");
    process_command("info");
    process_command("mem");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("BASE kernel initialized successfully!\n");
    terminal_writestring("System ready for operation.\n");
    
    while (1) {

        asm volatile("hlt");
    }
}

// Boot sector (simplified - would normally be in separate assembly file)
void _start(void) {
    kernel_main();
}
