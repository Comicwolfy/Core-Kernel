; kernel/idt.asm - Complete assembly handlers for 64-bit
BITS 64

global idt_load
global divide_error_handler, debug_handler, nmi_handler, breakpoint_handler
global overflow_handler, bound_range_exceeded_handler, invalid_opcode_handler
global device_not_available_handler, double_fault_handler, invalid_tss_handler
global segment_not_present_handler, stack_segment_fault_handler
global general_protection_fault_handler, page_fault_handler, fpu_error_handler
global alignment_check_handler, machine_check_handler, simd_fp_exception_handler
global irq0_handler, irq1_handler, irq2_handler, irq3_handler, irq4_handler
global irq5_handler, irq6_handler, irq7_handler, irq8_handler, irq9_handler
global irq10_handler, irq11_handler, irq12_handler, irq13_handler
global irq14_handler, irq15_handler

; External C functions
extern divide_error_handler_c, debug_handler_c, nmi_handler_c, breakpoint_handler_c
extern overflow_handler_c, bound_range_exceeded_handler_c, invalid_opcode_handler_c
extern device_not_available_handler_c, double_fault_handler_c, invalid_tss_handler_c
extern segment_not_present_handler_c, stack_segment_fault_handler_c
extern general_protection_fault_handler_c, page_fault_handler_c, fpu_error_handler_c
extern alignment_check_handler_c, machine_check_handler_c, simd_fp_exception_handler_c
extern irq0_handler_c, irq1_handler_c, irq2_handler_c, irq3_handler_c, irq4_handler_c
extern irq5_handler_c, irq6_handler_c, irq7_handler_c, irq8_handler_c, irq9_handler_c
extern irq10_handler_c, irq11_handler_c, irq12_handler_c, irq13_handler_c
extern irq14_handler_c, irq15_handler_c

section .text

; Load IDT
idt_load:
    lidt [rdi]
    ret

; Macro for exception handlers without error code
%macro ISR_NOERRCODE 2
%1:
    push 0          ; Push dummy error code
    push %2         ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Macro for exception handlers with error code
%macro ISR_ERRCODE 2
%1:
    push %2         ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Macro for IRQ handlers
%macro IRQ 2
%1:
    push 0          ; Push dummy error code
    push %2         ; Push IRQ number
    jmp irq_common_stub
%endmacro

; Exception handlers
ISR_NOERRCODE divide_error_handler, 0
ISR_NOERRCODE debug_handler, 1
ISR_NOERRCODE nmi_handler, 2
ISR_NOERRCODE breakpoint_handler, 3
ISR_NOERRCODE overflow_handler, 4
ISR_NOERRCODE bound_range_exceeded_handler, 5
ISR_NOERRCODE invalid_opcode_handler, 6
ISR_NOERRCODE device_not_available_handler, 7
ISR_ERRCODE   double_fault_handler, 8
ISR_ERRCODE   invalid_tss_handler, 10
ISR_ERRCODE   segment_not_present_handler, 11
ISR_ERRCODE   stack_segment_fault_handler, 12
ISR_ERRCODE   general_protection_fault_handler, 13
ISR_ERRCODE   page_fault_handler, 14
ISR_NOERRCODE fpu_error_handler, 16
ISR_ERRCODE   alignment_check_handler, 17
ISR_NOERRCODE machine_check_handler, 18
ISR_NOERRCODE simd_fp_exception_handler, 19

; IRQ handlers
IRQ irq0_handler, 32
IRQ irq1_handler, 33
IRQ irq2_handler, 34
IRQ irq3_handler, 35
IRQ irq4_handler, 36
IRQ irq5_handler, 37
IRQ irq6_handler, 38
IRQ irq7_handler, 39
IRQ irq8_handler, 40
IRQ irq9_handler, 41
IRQ irq10_handler, 42
IRQ irq11_handler, 43
IRQ irq12_handler, 44
IRQ irq13_handler, 45
IRQ irq14_handler, 46
IRQ irq15_handler, 47

; Common ISR stub
isr_common_stub:
    ; Save all registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Save segment registers
    mov ax, ds
    push rax
    mov ax, es
    push rax
    mov ax, fs
    push rax
    mov ax, gs
    push rax
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call appropriate C handler based on interrupt number
    mov rdi, rsp    ; Pass interrupt frame as parameter
    mov rax, [rsp + 144]  ; Get interrupt number from stack
    
    ; Jump to appropriate handler
    cmp rax, 0
    je .call_divide_error
    cmp rax, 1
    je .call_debug
    cmp rax, 2
    je .call_nmi
    cmp rax, 3
    je .call_breakpoint
    cmp rax, 4
    je .call_overflow
    cmp rax, 5
    je .call_bound_range
    cmp rax, 6
    je .call_invalid_opcode
    cmp rax, 7
    je .call_device_not_available
    cmp rax, 8
    je .call_double_fault
    cmp rax, 10
    je .call_invalid_tss
    cmp rax, 11
    je .call_segment_not_present
    cmp rax, 12
    je .call_stack_segment_fault
    cmp rax, 13
    je .call_general_protection_fault
    cmp rax, 14
    je .call_page_fault
    cmp rax, 16
    je .call_fpu_error
    cmp rax, 17
    je .call_alignment_check
    cmp rax, 18
    je .call_machine_check
    cmp rax, 19
    je .call_simd_fp_exception
    jmp .isr_end

.call_divide_error:
    call divide_error_handler_c
    jmp .isr_end
.call_debug:
    call debug_handler_c
    jmp .isr_end
.call_nmi:
    call nmi_handler_c
    jmp .isr_end
.call_breakpoint:
    call breakpoint_handler_c
    jmp .isr_end
.call_overflow:
    call overflow_handler_c
    jmp .isr_end
.call_bound_range:
    call bound_range_exceeded_handler_c
    jmp .isr_end
.call_invalid_opcode:
    call invalid_opcode_handler_c
    jmp .isr_end
.call_device_not_available:
    call device_not_available_handler_c
    jmp .isr_end
.call_double_fault:
    call double_fault_handler_c
    jmp .isr_end
.call_invalid_tss:
    call invalid_tss_handler_c
    jmp .isr_end
.call_segment_not_present:
    call segment_not_present_handler_c
    jmp .isr_end
.call_stack_segment_fault:
    call stack_segment_fault_handler_c
    jmp .isr_end
.call_general_protection_fault:
    call general_protection_fault_handler_c
    jmp .isr_end
.call_page_fault:
    call page_fault_handler_c
    jmp .isr_end
.call_fpu_error:
    call fpu_error_handler_c
    jmp .isr_end
.call_alignment_check:
    call alignment_check_handler_c
    jmp .isr_end
.call_machine_check:
    call machine_check_handler_c
    jmp .isr_end
.call_simd_fp_exception:
    call simd_fp_exception_handler_c
    jmp .isr_end

.isr_end:
    ; Restore segment registers
    pop rax
    mov gs, ax
    pop rax
    mov fs, ax
    pop rax
    mov es, ax
    pop rax
    mov ds, ax
    
    ; Restore general registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    ; Remove error code and interrupt number
    add rsp, 16
    
    iretq

; Common IRQ stub
irq_common_stub:
    ; Save all registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Save segment registers
    mov ax, ds
    push rax
    mov ax, es
    push rax
    mov ax, fs
    push rax
    mov ax, gs
    push rax
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call appropriate IRQ handler
    mov rax, [rsp + 144]  ; Get IRQ number from stack
    sub rax, 32           ; Convert to IRQ number (0-15)
    
    ; Jump to appropriate IRQ handler
    cmp rax, 0
    je .call_irq0
    cmp rax, 1
    je .call_irq1
    cmp rax, 2
    je .call_irq2
    cmp rax, 3
    je .call_irq3
    cmp rax, 4
    je .call_irq4
    cmp rax, 5
    je .call_irq5
    cmp rax, 6
    je .call_irq6
    cmp rax, 7
    je .call_irq7
    cmp rax, 8
    je .call_irq8
    cmp rax, 9
    je .call_irq9
    cmp rax, 10
    je .call_irq10
    cmp rax, 11
    je .call_irq11
    cmp