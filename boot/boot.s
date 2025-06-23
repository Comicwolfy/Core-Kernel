; boot.s - minimal 64-bit long mode bootloader with paging

[BITS 32]

section .multiboot_header
multiboot_header:
    align 4
    dd 0x1BADB002              ; multiboot magic
    dd 0                       ; flags (no mods)
    dd -(0x1BADB002 + 0)       ; checksum

section .text
global _start
extern kernel_main

_start:
    cli                         ; disable interrupts

    ; load GDT
    lgdt [gdt_descriptor]

    ; Setup paging tables
    call setup_paging

    ; enable PAE in CR4
    mov eax, cr4
    or eax, 1 << 5              ; PAE bit
    mov cr4, eax

    ; enable long mode in EFER MSR
    mov ecx, 0xC0000080         ; MSR_EFER
    rdmsr
    or eax, 1 << 8              ; LME bit
    wrmsr

    ; enable paging and protected mode in CR0
    mov eax, cr0
    or eax, 1 << 31             ; PG bit (paging)
    or eax, 1 << 0              ; PE bit (protected mode)
    mov cr0, eax

    ; far jump to 64-bit code segment
    jmp 0x08:long_mode_start

; -----------------------------------
; 64-bit code
[BITS 64]
long_mode_start:
    ; reload segment registers with 64-bit selectors
    mov ax, 0x10                ; data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    call kernel_main

.hang:
    hlt
    jmp .hang

; -----------------------------------
; GDT

section .data
align 8
gdt_start:
    dq 0x0000000000000000       ; null descriptor
    dq 0x00AF9A000000FFFF       ; code segment descriptor (base=0, limit=4GB, exec, readable)
    dq 0x00AF92000000FFFF       ; data segment descriptor (base=0, limit=4GB, writable)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; -----------------------------------
; Paging tables - 1GB identity map with 2MB pages

section .bss
align 4096
PML4 resq 1
align 4096
PDPT resq 1
align 4096
PD resq 512

section .text
setup_paging:
    ; PML4 -> PDPT
    mov rax, PDPT
    or rax, 0x03                ; present + RW
    mov [PML4], rax

    ; PDPT -> PD
    mov rax, PD
    or rax, 0x03                ; present + RW
    mov [PDPT], rax

    ; Map first 1GB using 2MB pages in PD
    mov rcx, 512               ; 512 entries
    mov rdi, PD                ; start of PD table

    xor rbx, rbx               ; page base address = 0 initially

.map_loop:
    mov rax, rbx               ; physical address for this entry
    or rax, 0x83               ; present + RW + PS (page size 2MB)
    mov [rdi], rax
    add rdi, 8                 ; next entry (8 bytes)
    add rbx, 0x200000          ; next 2MB page
    loop .map_loop

    ; Load PML4 into CR3 to activate paging
    mov rax, PML4
    mov cr3, rax

    ret
