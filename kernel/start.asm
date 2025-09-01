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
    
    ; Write debug message directly to VGA
    mov edi, 0xb8000
    mov al, 'K'
    mov ah, 0x0F
    mov [edi], ax
    mov al, 'E'
    mov [edi+2], ax
    mov al, 'R'
    mov [edi+4], ax
    mov al, 'N'
    mov [edi+6], ax
    mov al, 'E'
    mov [edi+8], ax
    mov al, 'L'
    mov [edi+10], ax
    
    ; Write "ASM OK" message to test if assembly works
    mov al, ' '
    mov [edi+12], ax
    mov al, 'A'
    mov [edi+14], ax
    mov al, 'S'
    mov [edi+16], ax
    mov al, 'M'
    mov [edi+18], ax
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
