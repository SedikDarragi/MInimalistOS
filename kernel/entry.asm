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
    ; Set up stack
    mov esp, stack_top
    
    ; Set up segment registers with data selector (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Clear screen (first 4K of VGA memory)
    cld                 ; Clear direction flag (increment EDI)
    mov edi, VGA_BUFFER
    mov ecx, 0x0800     ; 2048 words = 4096 bytes
    xor eax, eax
    mov ah, 0x0F        ; White on black
    mov al, ' '         ; Space character
    rep stosw           ; Fill screen with spaces
    
    ; Print a test message directly to VGA
    mov edi, 0          ; Start of first line
    mov esi, msg_kernel_loaded
    mov ah, 0x0F        ; White on black
    call vga_puts_at
    
    ; Print a second message to verify VGA output is working
    mov edi, 160        ; Start of second line (80 chars * 2 bytes)
    mov esi, msg_hello
    mov ah, 0x0A        ; Light green on black
    call vga_puts_at
    
    ; Print a third message with CPU information
    mov edi, 320        ; Start of third line
    mov esi, msg_cpuid
    mov ah, 0x0E        ; Yellow on black
    call vga_puts_at
    
    ; Try to get CPU vendor ID
    xor eax, eax
    cpuid
    
    ; Store vendor string at VGA position (4th line)
    mov [VGA_BUFFER + 480], ebx
    mov [VGA_BUFFER + 484], edx
    mov [VGA_BUFFER + 488], ecx
    
    ; Debug: Write a magic number to VGA to confirm kernel is running
    mov edi, 640        ; 5th line
    mov eax, 0xDEADBEEF
    mov [VGA_BUFFER + edi], eax

    ; Halt the CPU with interrupts disabled
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
