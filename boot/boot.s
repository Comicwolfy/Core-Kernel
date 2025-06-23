extern kernel_main
[BITS 64]

section .text
global _start
section .multiboot_header
    align 8
    dd 0xE85250D6              ; magic number
    dd 0                       ; architecture (0 = i386)
    dd (multiboot_header_end - multiboot_header) ; header length
    dd -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header)) ; checksum

multiboot_header_end:

_start:
    call kernel_main

.hang:
    hlt
    jmp .hang
