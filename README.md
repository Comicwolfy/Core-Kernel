Core Kernel

Core is a minimalist 64-bit kernel written from scratch, designed as a foundation for a custom operating system. It demonstrates basic kernel entry and simple output but is still in very early development.

    Warning: This kernel is experimental and currently does not work reliably on most hardware or emulators. Use for learning and experimentation only.

Requirements

        electricity

Features

    64-bit kernel entry point

    Basic screen output

    Custom bootloader integrated with GRUB

    Simple memory layout using linker scripts

Note: Because the kernel is experimental, booting may fail or hang on many systems.
Future Plans

    Fix bootloader and kernel stability issues

    Implement paging and proper memory management

    Add basic drivers and input handling

    Build a simple shell interface

    Support file systems and disk I/O

License
GPL - see LICENSE file.
