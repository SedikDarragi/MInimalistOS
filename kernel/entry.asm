; Kernel entry point
[bits 32]
[extern _start]

global _start_asm

section .text
_start_asm:
    ; Set up stack
    mov esp, 0x90000
    
    ; Call the C kernel
    call _start
    
    ; If we get here, something went wrong
    cli
    hlt
    jmp $
