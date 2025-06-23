BITS 64
global idt_load

section .data
align 16
idtp:
    dw idt_end - idt - 1      ; limit (size of IDT - 1)
    dq idt                    ; base address of IDT

section .text
idt_load:
    lidt [idtp]
    ret

section .data
idt:
    times 256 dq 0            ; reserve space for 256 IDT entries (each 16 bytes)
idt_end:
