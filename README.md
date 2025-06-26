# Core-Kernel

## Core is a minimalist modular kernel written from scratch, designed as a foundation for a modular OS.

Warning: This kernel is experimental and currently does not work reliably on most hardware or emulators YET.
## Requirements

    qemu

## Features

    Kernel entry point

    Basic screen output

    Custom bootloader integrated with GRUB

    Simple memory layout using linker scripts

    Modular extension system with automatic discovery

    Basic interrupt handling and keyboard input

    System timer and uptime counter

    A simple interactive shell interface

Note: Because the kernel is extremely experimental, booting will likely fail or hang on most systems.

## Adding New Extensions

Want to add your own module? Just drop your extension's C source file (e.g., my_cool_module.c) into the src/extensions/ directory. Make sure your extension includes a function like this, marked with the __attribute__((section(".ext_register_fns"))) magic, which handles registering and loading itself:

     // In src/extensions/my_cool_module.c 
    #include "base_kernel.h"

    // Your extension's init/cleanup/command handlers go here

    __attribute__((section(".ext_register_fns")))
    void __my_cool_module_auto_register(void) {
        int ext_id = register_extension("MyCoolModule", "1.0",
                                    my_cool_module_init,
                                    my_cool_module_cleanup);
    if (ext_id >= 0) {
        load_extension(ext_id);
    } else {
        terminal_writestring("Failed to auto-register MyCoolModule!\n");
    }
    } 

After that, simply update your Makefile by adding your new .c file to the C_SOURCES variable:

     C_SOURCES += src/extensions/my_cool_module.c 

Run make, and your module will be part of the kernel, automatically found and loaded at boot.
Future Plans

    Networking

    Fix bootloader and kernel stability issues

    Implement paging and proper memory management

    Support file systems and disk I/O

License

GPL - see LICENSE file.
