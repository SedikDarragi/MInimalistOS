; Kernel entry point
[bits 32]

; Export the entry point to the linker
global _start

; The C entry point
extern kmain

section .text
_start:
    ; Set up segment registers for protected mode
    mov ax, 0x10    ; Data segment selector (points to GDT entry 2, RPL 0)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack (grows down from 0x90000)
    mov esp, stack_top
    
    ; Clear direction flag (string operations increment)
    cld
    
    ; Clear screen with blue background
    mov edi, 0xB8000
    mov ecx, 80*25
    mov eax, 0x1F201F20  ; Blue background, white spaces
    rep stosd
    
    ; Print kernel message
    mov esi, kernel_msg
    mov edi, 0xB8000 + (80 * 2 + 20) * 2  ; Row 2, column 20
    mov ah, 0x1F  ; White on blue
.print_loop:
    lodsb
    or al, al
    jz .done_print
    stosw
    jmp .print_loop
.done_print:
    
    ; Call the C kernel entry point
    call kmain
    
    ; If we get here, something went wrong
    mov esi, panic_msg
    mov edi, 0xB8000 + (80 * 4 + 10) * 2  ; Row 4, column 10
    mov ah, 0x4F  ; White on red
.panic_loop:
    lodsb
    or al, al
    jz .halt
    stosw
    jmp .panic_loop

.halt:
    cli
.hang:
    hlt
    jmp .hang

section .data
kernel_msg db "MinimalOS Kernel is running!", 0
panic_msg db "Kernel panic: kmain returned!", 0

section .bss
align 16
stack_bottom:
    resb 16384    ; 16KB stack
stack_top:
