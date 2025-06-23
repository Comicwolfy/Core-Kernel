Core Kernel

Core is a minimalist 64-bit kernel written from scratch, designed as a foundation for a custom operating system. It demonstrates basic kernel entry and simple output but is still in very early development.

    Warning: This kernel is experimental and currently does not work reliably on most hardware or emulators. Use for learning and experimentation only.

Features

    64-bit kernel entry point

    Basic screen output

    Custom bootloader integrated with GRUB

    Simple memory layout using linker scripts

Project Structure

boot/
  ├─ grub/          # GRUB configuration files
  ├─ boot.s         # Assembly bootloader code
kernel/             # Kernel source code (C/ASM)
kernel.bin          # Linked kernel binary
linker.ld           # Linker script for kernel layout
Makefile            # Build automation
README.md           # This documentation
LICENSE             # Project license

Getting Started
Prerequisites

    NASM (Netwide Assembler)

    Cross-compiler targeting x86_64 (e.g., x86_64-elf-gcc)

    GNU Make

    GRUB bootloader (used to boot kernel)

    QEMU or a real x86_64 machine for testing (note: may not boot reliably yet)

Building

Run:

make

This will build the bootloader and kernel, producing kernel.bin and prepare everything for booting with GRUB.
Running

Use QEMU to test:

qemu-system-x86_64 -cdrom your-iso-or-grub-setup.iso

Note: Because the kernel is experimental, booting may fail or hang on many systems.
Future Plans

    Fix bootloader and kernel stability issues

    Implement paging and proper memory management

    Add basic drivers and input handling

    Build a simple shell interface

    Support file systems and disk I/O

License
GPL - see LICENSE file.
