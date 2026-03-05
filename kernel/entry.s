/* Entry point for the kernel - GAS syntax version */

.set VGA_BUFFER, 0xB8000
.set VGA_WIDTH, 80
.set VGA_HEIGHT, 25

/* Stack configuration */
.set STACK_SIZE, 0x4000  /* 16KB stack */

.section .text
.global _start

_start:
    cli
    /* Initialize segment registers for Protected Mode (0x10 is Data Segment) */
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    
    /* Set up stack using the symbol from our BSS section */
    movl $stack_top, %esp
    
    cld
    /* Call C kernel main function */
    call kmain
    
    /* If kmain returns, hang */
    cli
.hang:
    hlt
    jmp .hang

.section .rodata
msg_kernel_loaded:
    .asciz "Kernel loaded successfully!"
msg_hello:
    .asciz "Protected mode active!"
msg_cpuid:
    .asciz "CPU: "

.section .bss
.align 16
stack_bottom:
    .space STACK_SIZE
stack_top:
