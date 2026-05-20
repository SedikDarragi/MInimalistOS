/* Entry point for the kernel - GAS syntax version */

.set VGA_BUFFER, 0xB8000
.set VGA_WIDTH, 80
.set VGA_HEIGHT, 25

/* Stack configuration - 32KB dedicated stack */
.set STACK_SIZE, 0x8000

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

/* Dedicated stack section to prevent collision with .data/.rodata */
.section .stack, "aw", @nobits
.align 4096
stack_bottom:
    .space STACK_SIZE
stack_top:
