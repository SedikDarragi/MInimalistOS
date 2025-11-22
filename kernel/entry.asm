[bits 32]
section .text.entry
global _start

; Constants for VGA text mode
VGA_BUFFER equ 0xB8000
VGA_WIDTH  equ 80
VGA_HEIGHT equ 25

; Magic number for multiboot header
MAGIC     equ 0x1BADB002
FLAGS     equ 0x0
CHECKSUM  equ -(MAGIC + FLAGS)

; Multiboot header (required by some bootloaders)
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Kernel entry point
section .text.entry
global _start

; Stack configuration
STACK_SIZE equ 0x4000  ; 16KB stack

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
    cld                 ; Clear direction flag (increment ESI)
.print_loop:
    lodsb               ; Load byte from [DS:ESI] into AL
    test al, al
    jz .done
    
    ; Write character and attribute to VGA memory
    mov [es:VGA_BUFFER + edi + ecx*2], ax
    
    inc ecx
    jmp .print_loop
.done:
    popa
    ret

_start:
    ; Simple signature: Fill VGA with 'K' to show we're here
    mov edi, 0xB8000
    mov eax, 0x0C4B  ; Red 'K' on black background
    mov ecx, 2000     ; Fill entire screen
    cld
    rep stosw
    
    ; Debug: Write to serial port
    mov dx, 0x3F8
    mov al, 'K'
    out dx, al
    
    ; Infinite loop to test if we reach here
    cli
.hang:
    hlt
    jmp .hang

section .rodata
msg_kernel_loaded db "Kernel loaded successfully!", 0
msg_hello        db "Protected mode active!", 0
msg_cpuid       db "CPU: ", 0

section .bss
align 16
stack_bottom:
    resb STACK_SIZE
stack_top:
    ; Add a magic number at the top of the stack for debugging
    dd 0xDEADBEEF
