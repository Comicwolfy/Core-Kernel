section .text

extern generic_isr_handler
extern keyboard_handler_c
extern timer_handler_c

KERNEL_DATA_SEG equ 0x10

%macro IRQ_COMMON 1
    cli
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push byte %1

    %if %1 == 0x20
        call timer_handler_c
    %elif %1 == 0x21
        call keyboard_handler_c
    %else
        call generic_isr_handler
    %endif

    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    sti
    iret
%endmacro

global irq0
irq0:
    IRQ_COMMON 0x20

global irq1
irq1:
    IRQ_COMMON 0x21

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push byte 0
    push byte %1
    jmp common_isr_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte %1
    jmp common_isr_stub
%endmacro

ISR_ERRCODE 8
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_ERRCODE 17

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_NOERRCODE 9
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20

common_isr_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [esp + 36]
    call generic_isr_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8

    sti
    iret
