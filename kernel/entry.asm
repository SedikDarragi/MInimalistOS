; Kernel entry point
[bits 32]

; Export the entry point to the linker
global _start

; The C entry point
extern kmain

section .text
_start:
    ; Set up segment registers for protected mode
    mov ax, 0x10    ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack
    mov esp, stack_top
    
    ; Clear direction flag (string operations increment)
    cld
    
    ; Call the C kernel entry point
    call kmain
    
    ; If we get here, something went wrong
    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384    ; 16KB stack
stack_top:
