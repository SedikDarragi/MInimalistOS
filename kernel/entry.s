/* Entry point for the kernel - GAS syntax version */

.set VGA_BUFFER, 0xB8000
.set VGA_WIDTH, 80
.set VGA_HEIGHT, 25

/* Stack configuration */
.set STACK_SIZE, 0x4000  /* 16KB stack */

.global _start

_start:
    /* Debug: Write to VGA FIRST to see if we reach here */
    movb $'X', %al
    movb $0x0F, %ah
    movw %ax, (VGA_BUFFER)
    
    /* Write 'K' to show kernel is running */
    movb $'K', %al
    movb $0x0F, %ah
    movw %ax, (VGA_BUFFER + 6)
    
    /* Write '!' to show we're about to call kmain */
    movb $'!', %al
    movb $0x0F, %ah
    movw %ax, (VGA_BUFFER + 12)
    
    /* Jump to kernel main function */
    call kmain
    
    /* Set up stack for C code */
    movl $0x90000, %esp
    
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
