/* Interrupt Service Routines and Request Handlers */
.section .text
.align 4

/* Extern declarations for C handlers */
.extern fault_handler
.extern irq_handler

/* Common ISR Stub (for Exceptions) */
isr_common_stub:
    pusha               /* Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax */
    
    movw %ds, %ax
    push %eax           /* Save data segment descriptor */
    
    movw $0x10, %ax     /* Load kernel data segment descriptor */
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    push %esp           /* Push pointer to stack frame (struct regs*) */
    call fault_handler  /* Call C handler */
    add $4, %esp        /* Pop stack frame pointer */
    
    pop %eax            /* Restore original data segment descriptor */
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    popa                /* Pop edi,esi,ebp... */
    add $8, %esp        /* Clean up pushed error code and ISR number */
    iret                /* Return from interrupt */

/* Common IRQ Stub (for Hardware Interrupts) */
irq_common_stub:
    pusha
    
    movw %ds, %ax
    push %eax
    
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    push %esp
    call irq_handler
    add $4, %esp
    
    pop %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    popa
    add $8, %esp
    iret

/* Macros to define ISRs */
.macro ISR_NOERRCODE num
    .global isr\num
    isr\num:
        push $0         /* Push dummy error code */
        push $\num      /* Push interrupt number */
        jmp isr_common_stub
.endm

.macro ISR_ERRCODE num
    .global isr\num
    isr\num:
        /* Error code is already pushed by CPU */
        push $\num
        jmp isr_common_stub
.endm

.macro IRQ num, idt_num
    .global irq\num
    irq\num:
        push $0
        push $\idt_num
        jmp irq_common_stub
.endm

/* Define Exception Handlers (0-31) */
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

/* Define IRQ Handlers (32-47) */
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47