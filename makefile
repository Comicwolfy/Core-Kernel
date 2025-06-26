# Makefile for BASE Kernel
# Cross-compiler toolchain configuration

# Target architecture
ARCH = i686
TARGET = $(ARCH)-elf

# Cross-compiler tools
CC = $(TARGET)-gcc
LD = $(TARGET)-ld
AS = $(TARGET)-as
OBJCOPY = $(TARGET)-objcopy
OBJDUMP = $(TARGET)-objdump

# Alternative: Use system GCC if cross-compiler not available
# Uncomment these lines if you don't have a cross-compiler
# CC = gcc
# LD = ld
# AS = as

# Project configuration
KERNEL_NAME = base
VERSION = 1.0

# Directories
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = iso
BOOT_DIR = $(ISO_DIR)/boot
GRUB_DIR = $(BOOT_DIR)/grub

# Source files
KERNEL_SOURCES = base_kernel.c
ASM_SOURCES = boot.s
LINKER_SCRIPT = linker.ld

# Object files
KERNEL_OBJECTS = $(KERNEL_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.s=.o)
ALL_OBJECTS = $(ASM_OBJECTS) $(KERNEL_OBJECTS)

# Compiler flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Werror
CFLAGS += -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs
CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2
CFLAGS += -m32 -march=i686

# Assembler flags
ASFLAGS = --32

# Linker flags
LDFLAGS = -m elf_i386 -nostdlib
LDFLAGS += -T $(LINKER_SCRIPT)

# QEMU settings for testing
QEMU = qemu-system-i386
QEMU_FLAGS = -m 512M -serial stdio
QEMU_FLAGS += -drive file=$(KERNEL_NAME).iso,media=cdrom

# Build targets
.PHONY: all clean kernel iso run debug help install deps

# Default target
all: kernel

# Help target
help:
	@echo "BASE Kernel Build System"
	@echo "========================"
	@echo ""
	@echo "Available targets:"
	@echo "  all      - Build the kernel (default)"
	@echo "  kernel   - Build kernel binary"
	@echo "  iso      - Create bootable ISO image"
	@echo "  run      - Run kernel in QEMU"
	@echo "  debug    - Run kernel in QEMU with debugging"
	@echo "  clean    - Clean build files"
	@echo "  deps     - Check/install dependencies"
	@echo "  install  - Install to /boot (requires sudo)"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Configuration:"
	@echo "  ARCH     = $(ARCH)"
	@echo "  TARGET   = $(TARGET)"
	@echo "  CC       = $(CC)"
	@echo "  VERSION  = $(VERSION)"

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C source files
%.o: %.c | $(BUILD_DIR)
	@echo "CC $<"
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@

# Assemble assembly files
%.o: %.s | $(BUILD_DIR)
	@echo "AS $<"
	$(AS) $(ASFLAGS) $< -o $(BUILD_DIR)/$@

# Create linker script if it doesn't exist
$(LINKER_SCRIPT):
	@echo "Creating linker script..."
	@echo "ENTRY(_start)" > $(LINKER_SCRIPT)
	@echo "" >> $(LINKER_SCRIPT)
	@echo "SECTIONS" >> $(LINKER_SCRIPT)
	@echo "{" >> $(LINKER_SCRIPT)
	@echo "    . = 1M;" >> $(LINKER_SCRIPT)
	@echo "" >> $(LINKER_SCRIPT)
	@echo "    .text BLOCK(4K) : ALIGN(4K)" >> $(LINKER_SCRIPT)
	@echo "    {" >> $(LINKER_SCRIPT)
	@echo "        *(.multiboot)" >> $(LINKER_SCRIPT)
	@echo "        *(.text)" >> $(LINKER_SCRIPT)
	@echo "    }" >> $(LINKER_SCRIPT)
	@echo "" >> $(LINKER_SCRIPT)
	@echo "    .rodata BLOCK(4K) : ALIGN(4K)" >> $(LINKER_SCRIPT)
	@echo "    {" >> $(LINKER_SCRIPT)
	@echo "        *(.rodata)" >> $(LINKER_SCRIPT)
	@echo "    }" >> $(LINKER_SCRIPT)
	@echo "" >> $(LINKER_SCRIPT)
	@echo "    .data BLOCK(4K) : ALIGN(4K)" >> $(LINKER_SCRIPT)
	@echo "    {" >> $(LINKER_SCRIPT)
	@echo "        *(.data)" >> $(LINKER_SCRIPT)
	@echo "    }" >> $(LINKER_SCRIPT)
	@echo "" >> $(LINKER_SCRIPT)
	@echo "    .bss BLOCK(4K) : ALIGN(4K)" >> $(LINKER_SCRIPT)
	@echo "    {" >> $(LINKER_SCRIPT)
	@echo "        *(COMMON)" >> $(LINKER_SCRIPT)
	@echo "        *(.bss)" >> $(LINKER_SCRIPT)
	@echo "    }" >> $(LINKER_SCRIPT)
	@echo "}" >> $(LINKER_SCRIPT)

# Create boot assembly file if it doesn't exist
boot.s:
	@echo "Creating boot assembly file..."
	@echo "# Multiboot header" > boot.s
	@echo ".set ALIGN,    1<<0" >> boot.s
	@echo ".set MEMINFO,  1<<1" >> boot.s
	@echo ".set FLAGS,    ALIGN | MEMINFO" >> boot.s
	@echo ".set MAGIC,    0x1BADB002" >> boot.s
	@echo ".set CHECKSUM, -(MAGIC + FLAGS)" >> boot.s
	@echo "" >> boot.s
	@echo ".section .multiboot" >> boot.s
	@echo ".align 4" >> boot.s
	@echo ".long MAGIC" >> boot.s
	@echo ".long FLAGS" >> boot.s
	@echo ".long CHECKSUM" >> boot.s
	@echo "" >> boot.s
	@echo ".section .bss" >> boot.s
	@echo ".align 16" >> boot.s
	@echo "stack_bottom:" >> boot.s
	@echo ".skip 16384 # 16 KiB" >> boot.s
	@echo "stack_top:" >> boot.s
	@echo "" >> boot.s
	@echo ".section .text" >> boot.s
	@echo ".global _start" >> boot.s
	@echo ".type _start, @function" >> boot.s
	@echo "_start:" >> boot.s
	@echo "    mov $$stack_top, %esp" >> boot.s
	@echo "    call kernel_main" >> boot.s
	@echo "    cli" >> boot.s
	@echo "1:  hlt" >> boot.s
	@echo "    jmp 1b" >> boot.s
	@echo ".size _start, . - _start" >> boot.s

# Build kernel binary
kernel: $(BUILD_DIR) $(LINKER_SCRIPT) boot.s $(ALL_OBJECTS)
	@echo "LD $(KERNEL_NAME).bin"
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/$(KERNEL_NAME).bin $(addprefix $(BUILD_DIR)/, $(ALL_OBJECTS))
	@echo "Kernel built successfully: $(BUILD_DIR)/$(KERNEL_NAME).bin"
	@echo "Size: $$(du -h $(BUILD_DIR)/$(KERNEL_NAME).bin | cut -f1)"

# Verify multiboot compliance
verify: kernel
	@if command -v grub-file > /dev/null 2>&1; then \
		if grub-file --is-x86-multiboot $(BUILD_DIR)/$(KERNEL_NAME).bin; then \
			echo "✓ Multiboot compliant"; \
		else \
			echo "✗ Not multiboot compliant"; \
		fi; \
	else \
		echo "grub-file not found, skipping multiboot verification"; \
	fi

# Create GRUB configuration
$(GRUB_DIR)/grub.cfg: | $(GRUB_DIR)
	@echo "Creating GRUB configuration..."
	@echo "menuentry \"$(KERNEL_NAME) $(VERSION)\" {" > $@
	@echo "    multiboot /boot/$(KERNEL_NAME).bin" >> $@
	@echo "}" >> $@

# Create directories for ISO
$(BOOT_DIR) $(GRUB_DIR):
	mkdir -p $@

# Create bootable ISO
iso: kernel verify $(BOOT_DIR) $(GRUB_DIR) $(GRUB_DIR)/grub.cfg
	@echo "Creating ISO image..."
	cp $(BUILD_DIR)/$(KERNEL_NAME).bin $(BOOT_DIR)/
	@if command -v grub-mkrescue > /dev/null 2>&1; then \
		grub-mkrescue -o $(KERNEL_NAME).iso $(ISO_DIR); \
		echo "ISO created: $(KERNEL_NAME).iso"; \
	else \
		echo "Error: grub-mkrescue not found. Install GRUB tools."; \
		exit 1; \
	fi

# Run in QEMU
run: iso
	@if command -v $(QEMU) > /dev/null 2>&1; then \
		echo "Starting $(KERNEL_NAME) in QEMU..."; \
		$(QEMU) $(QEMU_FLAGS); \
	else \
		echo "Error: $(QEMU) not found. Install QEMU."; \
		exit 1; \
	fi

# Debug in QEMU with GDB support
debug: iso
	@if command -v $(QEMU) > /dev/null 2>&1; then \
		echo "Starting $(KERNEL_NAME) in QEMU with debugging..."; \
		$(QEMU) $(QEMU_FLAGS) -s -S & \
		echo "QEMU started. Connect with: gdb $(BUILD_DIR)/$(KERNEL_NAME).bin"; \
		echo "In GDB, run: target remote localhost:1234"; \
	else \
		echo "Error: $(QEMU) not found. Install QEMU."; \
		exit 1; \
	fi

# Install kernel to /boot (requires sudo)
install: kernel
	@echo "Installing $(KERNEL_NAME) to /boot..."
	@if [ "$$(id -u)" -ne 0 ]; then \
		echo "Installation requires root privileges. Use: sudo make install"; \
		exit 1; \
	fi
	cp $(BUILD_DIR)/$(KERNEL_NAME).bin /boot/$(KERNEL_NAME)-$(VERSION).bin
	@echo "Kernel installed to /boot/$(KERNEL_NAME)-$(VERSION).bin"

# Check dependencies
deps:
	@echo "Checking build dependencies..."
	@echo -n "Cross-compiler ($(CC)): "
	@if command -v $(CC) > /dev/null 2>&1; then \
		echo "✓ Found"; \
	else \
		echo "✗ Not found"; \
		echo "Install with: apt-get install gcc-multilib binutils"; \
	fi
	@echo -n "GRUB tools: "
	@if command -v grub-mkrescue > /dev/null 2>&1; then \
		echo "✓ Found"; \
	else \
		echo "✗ Not found"; \
		echo "Install with: apt-get install grub-pc-bin xorriso"; \
	fi
	@echo -n "QEMU: "
	@if command -v $(QEMU) > /dev/null 2>&1; then \
		echo "✓ Found"; \
	else \
		echo "✗ Not found"; \
		echo "Install with: apt-get install qemu-system-x86"; \
	fi

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)
	rm -rf $(ISO_DIR)
	rm -f $(KERNEL_NAME).iso
	rm -f $(LINKER_SCRIPT)
	rm -f boot.s
	rm -f *.o
	@echo "Clean complete."

# Show build information
info:
	@echo "BASE Kernel Build Information"
	@echo "============================="
	@echo "Kernel Name: $(KERNEL_NAME)"
	@echo "Version: $(VERSION)"
	@echo "Architecture: $(ARCH)"
	@echo "Target: $(TARGET)"
	@echo "Compiler: $(CC)"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo ""
	@echo "Source Files:"
	@for file in $(KERNEL_SOURCES) $(ASM_SOURCES); do \
		echo "  $$file"; \
	done
