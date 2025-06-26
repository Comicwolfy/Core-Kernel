# --- Compiler and Linker Settings ---
CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -fno-pie -c -Wall -Wextra -Iincludes # Added -Iincludes
ASFLAGS = -f elf

# --- Output Files ---
KERNEL_BIN = bin/kernel.bin
KERNEL_ELF = bin/kernel.elf

# --- Source Files ---
# Core C sources
C_SOURCES = src/kernel.c \
            src/extension_bootstrap.c # NEW: Add the extension bootstrap file

# Extension C sources (place your extension files here)
C_SOURCES += src/extensions/irq_kb_extension.c \
             src/extensions/timer_extension.c

# Assembly sources
ASM_SOURCES = src/boot.asm \
              src/irq_stubs.asm # NEW: Add the interrupt stubs

# --- Object Files (derived from sources) ---
OBJECTS = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.asm=.o)

# --- Build Targets ---
.PHONY: all clean run debug

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $< $@

$(KERNEL_ELF): $(OBJECTS)
	$(LD) -m elf_i386 -T linker.ld -o $@ $(OBJECTS)

# --- Compilation Rules ---
%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

%.o: src/extensions/%.c # Rule for compiling files in src/extensions
	$(CC) $(CFLAGS) $< -o $@

%.o: src/%.asm
	$(AS) $(ASFLAGS) $< -o $@

# --- Clean Target ---
clean:
	rm -f $(OBJECTS) $(KERNEL_ELF) $(KERNEL_BIN)

# --- Run in QEMU (ensure QEMU is installed) ---
run: all
	qemu-system-i386 -kernel $(KERNEL_BIN)

debug: all
	qemu-system-i386 -s -S -kernel $(KERNEL_BIN)

# You'll also need a linker.ld file in your root directory.
# A basic linker.ld for 32-bit x86:
# ENTRY(_start)
# SECTIONS {
#     . = 0x100000; /* Kernel starts at 1MB */
#     .text ALIGN (0x1000) : {
#         *(.text)
#     }
#     .rodata ALIGN (0x1000) : {
#         *(.rodata)
#     }
#     .data ALIGN (0x1000) : {
#         *(.data)
#     }
#     .bss ALIGN (0x1000) : {
#         *(.bss)
#     }
#     /* This section will contain the function pointers for auto-registration */
#     .ext_register_fns ALIGN(4) : {
#         _ext_register_start = .;
#         *(.ext_register_fns)
#         _ext_register_end = .;
#     }
# }
