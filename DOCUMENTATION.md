# base kernel documentation

## overview

base is a minimal extensible kernel designed as a foundation for modular operating system development. the core provides essential services while extensions add specific functionality.

## architecture

### core components

the base kernel consists of:

- **terminal subsystem** - vga text mode output with color support
- **memory manager** - basic block-based allocator
- **extension system** - dynamic loading and command registration
- **command processor** - extensible command line interface
- **boot sequence** - multiboot compliant initialization

### design philosophy

base follows a microkernel-inspired approach where the core remains minimal and stable while extensions provide specialized functionality. this design promotes modularity, maintainability, and allows for incremental feature development.

## core api reference

### terminal functions

```c
void terminal_initialize(void)
```
initializes the vga text mode terminal with 80x25 character display.

```c
void terminal_setcolor(uint8_t color)
```
sets the current text color using vga color constants.

```c
void terminal_writestring(const char* data)
```
outputs a null-terminated string to the terminal.

```c
void terminal_putchar(char c)
```
outputs a single character, handling newlines and scrolling.

### memory management

```c
void* kmalloc(size_t size)
```
allocates memory block of specified size. returns null on failure.

```c
void kfree(void* ptr)
```
frees previously allocated memory block.

### extension system

```c
int register_extension(const char* name, const char* version, 
                      int (*init_func)(void), void (*cleanup_func)(void))
```
registers a new extension with the kernel. returns extension id on success, -1 on failure.

parameters:
- name: extension name (max 31 characters)
- version: version string (max 15 characters)
- init_func: initialization function pointer
- cleanup_func: cleanup function pointer

```c
int load_extension(int ext_id)
```
loads and initializes an extension. returns 0 on success, -1 on failure.

```c
int unload_extension(int ext_id)
```
unloads an extension and calls its cleanup function.

```c
int register_command(const char* name, void (*handler)(const char*), 
                    const char* description, int ext_id)
```
registers a new command with the command processor.

parameters:
- name: command name (max 15 characters)
- handler: function to handle command execution
- description: command description (max 63 characters)  
- ext_id: owning extension id (-1 for core commands)

## building the kernel

### prerequisites

required tools:
- gcc cross-compiler (i686-elf-gcc recommended)
- gnu binutils
- grub tools (grub-mkrescue, grub-file)
- qemu for testing
- xorriso for iso creation

### build commands

```bash
# check dependencies
make deps

# build kernel binary
make kernel

# create bootable iso
make iso

# test in qemu
make run

# debug with gdb
make debug

# clean build files
make clean
```

### build output

- `build/base.bin` - kernel binary
- `base.iso` - bootable iso image
- `build/` directory contains object files

## extension development

### basic extension structure

```c
#include "base_kernel.h"

// extension metadata
static int ext_id = -1;

// command handler
void my_command(const char* args) {
    terminal_writestring("extension command executed\n");
}

// initialization function
int my_extension_init(void) {
    register_command("mycmd", my_command, "my custom command", ext_id);
    return 0; // success
}

// cleanup function
void my_extension_cleanup(void) {
    // cleanup resources
}

// extension registration (called from kernel)
void register_my_extension(void) {
    ext_id = register_extension("myext", "1.0", 
                               my_extension_init, 
                               my_extension_cleanup);
    if (ext_id >= 0) {
        load_extension(ext_id);
    }
}
```

### extension guidelines

- keep extensions focused on single functionality
- use descriptive command names to avoid conflicts
- handle errors gracefully in command handlers
- clean up resources in cleanup function
- test extensions thoroughly before integration

### available core services

extensions can use these core services:
- terminal output functions
- memory allocation (kmalloc/kfree)
- command registration
- basic string utilities (strlen)

## command reference

### core commands

**help**
displays all available commands with descriptions.

**info**
shows system information including architecture, memory status, and loaded extensions.

**ext**
lists loaded and available extensions with their versions and status.

**mem**
displays memory manager status and usage information.

**clear**
clears the terminal screen and displays kernel banner.

### command format

commands follow the format: `command [arguments]`

arguments are passed as a single string to the command handler. individual commands are responsible for parsing their arguments.

## memory layout

### physical memory

```
0x00000000 - 0x000FFFFF : reserved (bios, boot)
0x00100000 - 0x001FFFFF : kernel code and data
0x00200000+            : available for allocation
```

### virtual memory

base kernel currently operates in physical memory mode. virtual memory support can be added as an extension.

## technical specifications

### supported architectures
- x86 (32-bit)

### boot protocol
- multiboot specification compliant
- loaded by grub or compatible bootloader

### memory requirements
- minimum: 512kb ram
- recommended: 4mb+ ram

### display
- vga text mode (80x25 characters)
- 16 color support

## troubleshooting

### build issues

**cross-compiler not found**
install gcc-multilib and binutils packages for your distribution.

**grub tools missing**
install grub-pc-bin and xorriso packages.

**qemu not available**
install qemu-system-x86 package.

### runtime issues

**kernel doesn't boot**
verify multiboot compliance with `grub-file --is-x86-multiboot base.bin`

**commands not working**
check extension loading status with `ext` command.

**memory allocation failures**
monitor memory usage and implement better allocation strategies.

## development roadmap

### planned core improvements
- interrupt handling system
- timer subsystem  
- keyboard input support
- better memory management

### suggested extensions
- filesystem support
- network stack
- process management
- device drivers
- user interface enhancements

## contributing

### code style
- use lowercase with underscores for function names
- keep functions focused and well-documented
- follow existing code patterns
- test changes thoroughly

### extension submission
- provide clear documentation
- include usage examples
- test with multiple scenarios
- follow extension development guidelines

## license and credits

base kernel is designed for educational and experimental use. extensions may have their own licensing terms.

developed as a foundation for kernel development learning and experimentation.
