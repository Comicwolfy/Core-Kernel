; File: src/irq_stubs.asm

section .text

; External C handlers
extern generic_isr_handler
extern keyboard_handler_c
extern timer_handler_c

; Define kernel data segment selector (usually 0x10 for flat model)
KERNEL_DATA_SEG equ 0x10

; Common handler for IRQs
; %1 is the interrupt number
%macro IRQ_COMMON 1
    cli             ; Clear interrupts to prevent re-entrancy issues during handler execution
    pusha           ; Push all general-purpose registers (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
    push ds
    push es
    push fs
    push gs

    ; Set up kernel data segment for C functions
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Push interrupt number onto stack for C handler
    push byte %1

    ; Call the specific C handler based on interrupt number
    %if %1 == 0x20 ; IRQ0 (Timer)
        call timer_handler_c
    %elif %1 == 0x21 ; IRQ1 (Keyboard)
        call keyboard_handler_c
    %else
        ; For other IRQs (exceptions, or unhandled IRQs), call the generic handler
        call generic_isr_handler
    %endif

    add esp, 4      ; Pop the interrupt number argument

    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds
    popa            ; Pop all general-purpose registers
    sti             ; Set interrupts (enable interrupts)
    iret            ; Return from interrupt (pops CS, EIP, EFLAGS, ESP, SS for stack switch if privileged change occurred)
%endmacro

; --- IRQ Handlers ---
; IRQ0-7 are mapped to 0x20-0x27 after PIC remap
; IRQ8-15 are mapped to 0x28-0x2F after PIC remap

global irq0
irq0:
    IRQ_COMMON 0x20 ; IRQ0 (Timer)

global irq1
irq1:
    IRQ_COMMON 0x21 ; IRQ1 (Keyboard)

; Add more global irqX entries here as you implement more IRQs.
; For example:
; global irq2
; irq2:
;     IRQ_COMMON 0x22

; --- Exception Handlers (ISR - Interrupt Service Routines) ---
; These are for CPU exceptions (interrupts 0-31)
; Some exceptions push an error code automatically, others do not.
; We push a dummy error code (0) for those that don't, so the stack frame is consistent.

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli             ; Disable interrupts
    push byte 0     ; Push a dummy error code (0)
    push byte %1    ; Push the interrupt number
    jmp common_isr_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli             ; Disable interrupts
    push byte %1    ; Push the interrupt number (error code is already on stack)
    jmp common_isr_stub
%endmacro

; Exceptions with error codes: 8, 10-14, 17
ISR_ERRCODE 8
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13 ; General Protection Fault - Very common!
ISR_ERRCODE 14 ; Page Fault - Very common!
ISR_ERRCODE 17

; Exceptions without error codes (0-7, 9, 15-16, 18-31)
ISR_NOERRCODE 0  ; Divide-by-zero
ISR_NOERRCODE 1  ; Debug
ISR_NOERRCODE 2  ; NMI
ISR_NOERRCODE 3  ; Breakpoint
ISR_NOERRCODE 4  ; Overflow
ISR_NOERRCODE 5  ; Bound Range Exceeded
ISR_NOERRCODE 6  ; Invalid Opcode
ISR_NOERRCODE 7  ; Device Not Available (No Math Coprocessor)
ISR_NOERRCODE 9  ; Coprocessor Segment Overrun (Reserved)
ISR_NOERRCODE 15 ; Reserved
ISR_NOERRCODE 16 ; x87 FPU Floating-Point Error
ISR_NOERRCODE 18 ; Alignment Check
ISR_NOERRCODE 19 ; Machine Check
ISR_NOERRCODE 20 ; SIMD Floating-Point Exception
; ISR_NOERRCODE 21-31 (Reserved by Intel)
; You might extend these up to 31 if you want to catch all reserved exceptions.

; Common stub for all ISRs (exceptions)
common_isr_stub:
    pusha           ; Push all general-purpose registers
    push ds
    push es
    push fs
    push gs

    mov ax, KERNEL_DATA_SEG ; Load the kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; The interrupt number is now at [esp + 8*4 + 4] (after segment registers and pusha)
    ; The error code is at [esp + 8*4]
    ; So, the interrupt number is at [esp + 36] (if 32-bit registers)
    ; We need to pass the interrupt number to generic_isr_handler.
    ; This depends on how the stack looks *before* the pusha.
    ; For simplicity, we just passed the number before the jmp to common_isr_stub.
    ; The number is at [esp + 8 + 4*4] = [esp+24] if pusha pushes 8 registers (32-bit).
    ; No, it's easier: the C function expects the number as first arg on stack.
    ; It was pushed as `push byte %1` before `jmp common_isr_stub`.
    ; So, it's currently at `[esp+32]` (after pusha and segment regs).
    ; Let's just call the handler directly if ISR_ERRCODE/NOERRCODE already pushed it.

    ; Correct stack setup for calling C handler:
    ; From ISR_NOERRCODE: [dummy_err_code][int_no][EIP][CS][EFLAGS]
    ; From ISR_ERRCODE:   [err_code][int_no][EIP][CS][EFLAGS]
    ; After pusha, segment registers, the `int_no` is at `[esp+36]`
    ; Push `int_no` again for the C function's argument:
    push dword [esp + 36] ; Push the interrupt number for the C function argument
    call generic_isr_handler
    add esp, 4      ; Pop the argument we pushed

    ; Clean up the stack frame from the ISR itself:
    ; Pop segment registers
    pop gs
    pop fs
    pop es
    pop ds
    popa            ; Pop general-purpose registers

    ; Clean up the error code and interrupt number pushed by ISR_NOERRCODE/ISR_ERRCODE
    ; The error code is directly below the int_no on the stack
    add esp, 8      ; Pop interrupt number (4 bytes) and error code (4 bytes)

    sti             ; Re-enable interrupts
    iret            ; Return from interrupt
