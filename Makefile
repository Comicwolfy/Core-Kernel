ASM = nasm
ASMFLAGS = -f elf64
CC = gcc
CFLAGS = -m64 -ffreestanding -O2 -Wall -Wextra -c
LD = ld
LDFLAGS = -nostdlib -T linker.ld

# object files added shell.o here
OBJ = boot.o main.o shell.o \
	drivers/keyboard.o \
	drivers/pit.o \
	drivers/serial.o \
	drivers/vga.o \
	drivers/rtc.o \
	drivers/ata.o \
	drivers/mm.o

BIN = kernel.bin

all: $(BIN)

# assembly files
boot.o: boot.s
	$(ASM) $(ASMFLAGS) boot.s -o boot.o

# main kernel files
main.o: main.c
	$(CC) $(CFLAGS) main.c -o main.o

# shell compilation - added this rule
shell.o: shell.c shell.h
	$(CC) $(CFLAGS) shell.c -o shell.o

# driver compilations
drivers/keyboard.o: drivers/keyboard.c drivers/keyboard.h
	$(CC) $(CFLAGS) drivers/keyboard.c -o drivers/keyboard.o

drivers/pit.o: drivers/pit.c drivers/pit.h
	$(CC) $(CFLAGS) drivers/pit.c -o drivers/pit.o

drivers/serial.o: drivers/serial.c drivers/serial.h
	$(CC) $(CFLAGS) drivers/serial.c -o drivers/serial.o

drivers/vga.o: drivers/vga.c drivers/vga.h
	$(CC) $(CFLAGS) drivers/vga.c -o drivers/vga.o

drivers/rtc.o: drivers/rtc.c drivers/rtc.h
	$(CC) $(CFLAGS) drivers/rtc.c -o drivers/rtc.o

drivers/ata.o: drivers/ata.c drivers/ata.h
	$(CC) $(CFLAGS) drivers/ata.c -o drivers/ata.o

drivers/mm.o: drivers/mm.c drivers/mm.h
	$(CC) $(CFLAGS) drivers/mm.c -o drivers/mm.o

# link everything together
$(BIN): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o kernel.elf
	objcopy -O binary kernel.elf $(BIN)

# clean up build files
clean:
	rm -f *.o kernel.elf $(BIN) drivers/*.o

# optional add a run target if you're using QEMU for testing
run: $(BIN)
	qemu-system-x86_64 -kernel $(BIN)

# optional add an ISO target if you want to create a bootable ISO
iso: $(BIN)
	mkdir -p iso/boot/grub
	cp $(BIN) iso/boot/
	cp boot/grub/grub.cfg iso/boot/grub/
	grub-mkrescue -o kernel.iso iso/

.PHONY: all clean run iso