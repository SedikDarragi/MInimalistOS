BITS 32
EXTERN kmain

section .text
global _start

_start:
    ; Clear interrupts
    cli
    
    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack with proper alignment
    mov esp, 0x90000
    and esp, 0xFFFFFFF0  ; Align stack to 16-byte boundary
    
    ; Clear direction flag
    cld
    
    ; Call the C kernel main function
    call kmain
    
    ; If kmain returns (it shouldn't), halt the system
    cli
.halt:
    hlt
    jmp .halt
    mov al, ' '
    mov [edi+20], ax
    mov al, 'O'
    mov [edi+22], ax
    mov al, 'K'
    mov [edi+24], ax
    
    ; Call C kernel main function
    push 0
    call kmain
    add esp, 4
    
halt_loop:
    cli
    hlt
    jmp halt_loop

section .bss
stack_bottom:
    resb 8192       ; 8KB stack
stack_top:
