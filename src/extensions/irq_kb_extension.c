#include <stdint.h>
#include <stddef.h>
#include "base_kernel.h"

#define KB_BUFFER_SIZE 256
static char keyboard_buffer[KB_BUFFER_SIZE];
static volatile size_t kb_buffer_head = 0;
static volatile size_t kb_buffer_tail = 0;

static int irq_kb_ext_id = -1;

static const unsigned char kbd_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*',
    0,
    ' ',
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry global_idt[256];
static struct idt_ptr global_idt_p;

static void set_local_idt_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    global_idt[num].base_low = base & 0xFFFF;
    global_idt[num].base_high = (base >> 16) & 0xFFFF;
    global_idt[num].sel = sel;
    global_idt[num].always0 = 0;
    global_idt[num].flags = flags;
}

static void pic_remap(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void generic_isr_handler(int int_no) {
    terminal_writestring("Interrupt: ");
    char num_str[10];
    int i = 0;
    if (int_no == 0) { num_str[0] = '0'; i = 1; }
    else { int temp = int_no; while (temp > 0) { num_str[i++] = (temp % 10) + '0'; temp /= 10; } }
    num_str[i] = '\0';
    for (int start = 0, end = i - 1; start < end; start++, end--) { char temp = num_str[start]; num_str[start] = num_str[end]; num_str[end] = temp; }
    terminal_writestring(num_str);
    terminal_writestring("\n");
}

void keyboard_handler_c() {
    uint8_t scancode = inb(0x60);

    if (!(scancode & 0x80)) {
        char ascii = kbd_us[scancode];
        if (ascii != 0) {
            size_t next_head = (kb_buffer_head + 1) % KB_BUFFER_SIZE;
            if (next_head != kb_buffer_tail) {
                keyboard_buffer[kb_buffer_head] = ascii;
                kb_buffer_head = next_head;
            }
        }
    }

    outb(0x20, 0x20);
}

char read_char_from_kb_buffer() {
    if (kb_buffer_head == kb_buffer_tail) {
        return 0;
    }
    char c = keyboard_buffer[kb_buffer_tail];
    kb_buffer_tail = (kb_buffer_tail + 1) % KB_BUFFER_SIZE;
    return c;
}

char wait_for_char_from_kb_buffer() {
    while (kb_buffer_head == kb_buffer_tail) {
        asm volatile("hlt");
    }
    char c = keyboard_buffer[kb_buffer_tail];
    kb_buffer_tail = (kb_buffer_tail + 1) % KB_BUFFER_SIZE;
    return c;
}

void cmd_cli_input(const char* args) {
    terminal_writestring("Enter command (press Enter to execute, Backspace works, Ctrl+C to exit):\n");
    terminal_writestring("$ ");

    char input_buffer[VGA_WIDTH + 1];
    size_t input_idx = 0;

    while (1) {
        char c = wait_for_char_from_kb_buffer();

        if (c == '\n' || c == '\r') {
            input_buffer[input_idx] = '\0';
            terminal_putchar('\n');
            if (input_idx > 0) {
                process_command(input_buffer);
            }
            terminal_writestring("$ ");
            input_idx = 0;
        } else if (c == '\b' || c == 0x7F) {
            if (input_idx > 0) {
                input_idx--;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        } else if (c == 0x03) {
            terminal_writestring("^C\n");
            return;
        }
        else if (input_idx < VGA_WIDTH) {
            input_buffer[input_idx++] = c;
            terminal_putchar(c);
        }
    }
}

int irq_kb_extension_init(void) {
    terminal_writestring("IRQ & Keyboard Extension: Initializing...\n");

    global_idt_p.limit = (sizeof(struct idt_entry) * 256) - 1;
    global_idt_p.base = (uint32_t)&global_idt;

    for (int i = 0; i < 256; i++) {
        set_local_idt_gate(i, 0, 0x08, 0x8E);
    }

    set_local_idt_gate(0x20, (uint32_t)irq0, 0x08, 0x8E);
    set_local_idt_gate(0x21, (uint32_t)irq1, 0x08, 0x8E);

    asm volatile("lidt %0" : : "m"(global_idt_p));

    pic_remap();

    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);

    asm volatile("sti");

    terminal_writestring("IRQ & Keyboard Extension: IDT loaded, PIC remapped, Interrupts enabled.\n");
    terminal_writestring("IRQ & Keyboard Extension: Keyboard ready.\n");

    register_command("cli_test", cmd_cli_input, "Test basic keyboard input", irq_kb_ext_id);

    return 0;
}

void irq_kb_extension_cleanup(void) {
    terminal_writestring("IRQ & Keyboard Extension: Cleaning up...\n");
    asm volatile("cli");
    outb(0x21, inb(0x21) | 0x03);
    terminal_writestring("IRQ & Keyboard Extension: Cleanup complete.\n");
}

__attribute__((section(".ext_register_fns")))
void __irq_kb_auto_register(void) {
    irq_kb_ext_id = register_extension("IRQ_KB", "1.0",
                                       irq_kb_extension_init,
                                       irq_kb_extension_cleanup);
    if (irq_kb_ext_id >= 0) {
        load_extension(irq_kb_ext_id);
    } else {
        terminal_writestring("Failed to register IRQ & Keyboard Extension (auto)!\n");
    }
}
