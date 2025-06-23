AS = nasm
ASFLAGS = -f elf64

CC = gcc
CFLAGS = -m64 -ffreestanding -O2 -Wall -Wextra

LD = ld
LDFLAGS = -m elf_x86_64 -T linker.ld

OBJS = boot/boot.o kernel/main.o

all: kernel.bin

boot/boot.o: boot/boot.s
	$(AS) $(ASFLAGS) $< -o $@

kernel/main.o: kernel/main.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o kernel.bin

clean:
	rm -f boot/*.o kernel/*.o kernel.bin
