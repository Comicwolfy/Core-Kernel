ENTRY(_start)
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

SECTIONS
{
    /* Kernel loaded at 1MB */
    . = 0x100000;
    
    /* Multiboot header must be early in the file */
    .multiboot ALIGN(4096) : {
        KEEP(*(.multiboot_header))
    }
    
    /* Code section, aligned to 4KB */
    .text ALIGN(4096) : {
        *(.text*)
        *(.rodata*)                 /* rodata in text for const data */
    }
    
    /* Data section, aligned */
    .data ALIGN(4096) : {
        *(.data*)
    }
    
    /* BSS section, zero-initialized */
    .bss ALIGN(4096) : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }
    
    /* Mark end of kernel image */
    . = ALIGN(4096);
    __kernel_end = .;
    
    /* Stack - place at a fixed high address to avoid conflicts */
    .stack 0x7E000 (NOLOAD) : {
        __stack_start = .;
        . += 16K;
        __stack_end = .;
    }
    
    /* Heap area symbol (for reference) */
    __heap_start = 0x200000;
    __heap_size = 0x100000;
    
    /* End symbol for entire memory layout */
    _end = .;
    
    /* Discard debug sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.debug*)
    }
}