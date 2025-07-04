#ifndef BASE_KERNEL_H
#define BASE_KERNEL_H

#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

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
    VGA_COLOR_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str);

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "dN"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "dN"(port) );
    return ret;
}

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_writestring(const char* data);
void terminal_putchar(char c);

void* kmalloc(size_t size);
void kfree(void* ptr);

typedef struct extension {
    char name[32];
    char version[16];
    int (*init)(void);
    void (*cleanup)(void);
    int active;
} extension_t;

typedef struct command {
    char name[16];
    void (*handler)(const char* args);
    char description[64];
    extension_t* owner;
} command_t;


int register_extension(const char* name, const char* version,
                       int (*init_func)(void), void (*cleanup_func)(void));
int load_extension(int ext_id);
int unload_extension(int ext_id);
int register_command(const char* name, void (*handler)(const char*),
                     const char* description, int ext_id);
command_t* find_command(const char* name);
void process_command(const char* input);

typedef void (*extension_auto_register_func_t)(void);

extern extension_auto_register_func_t __ext_register_start[];
extern extension_auto_register_func_t __ext_register_end[];

void initialize_all_extensions(void);

extern void generic_isr_handler(int int_no);
extern void keyboard_handler_c(void);
extern void timer_handler_c(void);

extern char read_char_from_kb_buffer();
extern char wait_for_char_from_kb_buffer();


#endif
