ENTRY(_start)

OUTPUT_FORMAT("elf64-x86-64")

SECTIONS
{
    /* Kernel loaded at 1MB */
    . = 0x100000;

    /* Code section, aligned to 4KB */
    .text ALIGN(4096) : {
        KEEP(*(.multiboot_header))  /* multiboot header must stay */
        *(.text*)
        *(.rodata*)                 /* rodata in text for const data */
    }

    /* Data section, aligned */
    .data ALIGN(4096) : {
        *(.data*)
    }

    /* BSS section, zero-initialized */
    .bss ALIGN(4096) : {
        *(.bss*)
        *(COMMON)
    }

    /* Stack - reserve 16KB, no load (won’t be in file) */
    .stack (NOLOAD) ALIGN(16) : {
        __stack_start = .;
        . += 16K;
        __stack_end = .;
    }

    /* End symbol for kernel image */
    _end = .;
}
