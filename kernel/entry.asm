[bits 32]
section .text.entry
global _start

_start:
    ; The bootloader has already put us in 32-bit protected mode.
    ; It has also set up the segment registers.
    ; All we need to do is set up the stack and call our C kernel.
    mov esp, stack_top

    extern kmain
    call kmain

    ; If kmain returns, hang the system.
    cli
    hlt

section .bss
resb 8192 ; 8KB for the stack
stack_top:
