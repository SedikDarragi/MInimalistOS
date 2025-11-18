[bits 32]
section .text.entry
global _start

; Constants for VGA text mode
VGA_BUFFER equ 0xB8000
VGA_WIDTH  equ 80
VGA_HEIGHT equ 25

; Function to write a character to VGA memory
; Input: AL = character, AH = color, EDI = offset from VGA_BUFFER (in bytes)
vga_putc_at:
    mov [VGA_BUFFER + edi], ax
    ret

; Function to print a string
; Input: ESI = string, AH = color, EDI = offset from VGA_BUFFER (in bytes)
vga_puts_at:
    pusha
    xor ecx, ecx
.print_loop:
    lodsb
    test al, al
    jz .done
    mov [VGA_BUFFER + edi + ecx*2], ax
    inc ecx
    jmp .print_loop
.done:
    popa
    ret

_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Clear screen (first line only for now)
    mov edi, 0
    mov ecx, 80
    mov ax, 0x0F20  ; space with white on black
.clear_loop:
    mov [VGA_BUFFER + edi], ax
    add edi, 2
    loop .clear_loop
    
    ; Print kernel entry message at top of screen
    mov esi, msg_kernel_entry
    mov edi, 0
    mov ah, 0x0F  ; White on black
    call vga_puts_at
    
    ; Print 'K' in second position
    mov edi, 2
    mov ax, 0x0F4B  ; 'K' in white on black
    call vga_putc_at
    
    ; Call the C kernel
    extern kmain
    call kmain

    ; If kmain returns, hang the system
    cli
    hlt
    jmp $

section .rodata
msg_kernel_entry db "Kernel Entry", 0

section .bss
resb 8192  ; 8KB stack
stack_top:
