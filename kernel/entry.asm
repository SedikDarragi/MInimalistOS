[bits 32]

global _start
extern kmain

section .text
_start:
    ; Point the stack to the top of our stack space.
    mov esp, stack_top

    ; --- 7. Kernel Entry ---
    mov byte [0xB8004], '7'
    mov byte [0xB8005], 0x0F

    ; Call the C kernel.
    call kmain

    ; If kmain returns, hang.
    cli
    hlt

section .bss
resb 8192               ; 8KB for stack
stack_top:
