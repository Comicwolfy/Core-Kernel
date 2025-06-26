CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -fno-pie -c -Wall -Wextra -Iincludes
ASFLAGS = -f elf

KERNEL_BIN = bin/kernel.bin
KERNEL_ELF = bin/kernel.elf

C_SOURCES = src/kernel.c \
            src/extension_bootstrap.c

C_SOURCES += src/extensions/irq_kb_extension.c \
             src/extensions/timer_extension.c

ASM_SOURCES = src/boot.asm \
              src/irq_stubs.asm

OBJECTS = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.asm=.o)

.PHONY: all clean run debug

all: $(KERNEL_BIN)

$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $< $@

$(KERNEL_ELF): $(OBJECTS)
	$(LD) -m elf_i386 -T linker.ld -o $@ $(OBJECTS)

%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

%.o: src/extensions/%.c
	$(CC) $(CFLAGS) $< -o $@

%.o: src/%.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(KERNEL_ELF) $(KERNEL_BIN)

run: all
	qemu-system-i386 -kernel $(KERNEL_BIN)

debug: all
	qemu-system-i386 -s -S -kernel $(KERNEL_BIN)
