; kernel/gdt.asm - Fixed for 64-bit
BITS 64
global gdt_flush

section .text
gdt_flush:
    lgdt [rdi]          ; Load GDT (rdi contains pointer to GDT descriptor)
    
    ; Reload segment registers
    mov ax, 0x10        ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump to reload CS with kernel code segment
    push 0x08           ; Kernel code segment
    lea rax, [rel .reload_cs]
    push rax
    retfq               ; Far return to reload CS
    
.reload_cs:
    ret