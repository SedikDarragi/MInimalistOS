; Kernel entry point
[bits 32]

; Export the entry point to the linker
global _start

; The C entry point
extern kmain

section .text
_start:
    ; Set up segment registers for flat memory model
    mov ax, 0x10        ; Data segment selector (0x10 points to our data segment in GDT)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; Set up stack at 0x90000 (grows down)

    ; Clear direction flag (string operations increment)
    cld

    ; Early debug output: print "Kernel start" to VGA text buffer
    mov edi, 0xB8000 + (80 * 1) * 2  ; Row 1, column 0
    mov esi, kernel_start_msg
    mov ah, 0x1F        ; White on blue
.print_loop:
    lodsb
    or al, al           ; Check for null terminator
    jz .done_print
    stosw               ; Write character and attribute
    jmp .print_loop
.done_print:

    ; Clear screen with blue background
    mov edi, 0xB8000    ; VGA text buffer
    mov ecx, 80*25      ; 80x25 characters
    mov eax, 0x1F201F20 ; Blue background, white spaces (two characters at a time)
    rep stosd

    ; Print kernel running message
    mov esi, kernel_msg
    mov edi, 0xB8000 + (80 * 2 + 20) * 2  ; Row 2, column 20
    mov ah, 0x1F        ; White on blue
.print_loop2:
    lodsb
    or al, al           ; Check for null terminator
    jz .done_print2
    stosw               ; Write character and attribute
    jmp .print_loop2
.done_print2:

    ; Call the C kernel entry point
    call kmain

    ; If we get here, something went wrong
    mov esi, panic_msg
    mov edi, 0xB8000 + (80 * 4 + 10) * 2  ; Row 4, column 10
    mov ah, 0x4F        ; White on red
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
kernel_start_msg db "Kernel start", 0
kernel_msg db "MinimalOS Kernel is running!", 0
panic_msg db "Kernel panic: kmain returned!", 0

section .bss
align 16
stack_bottom:
    resb 16384    ; 16KB stack
stack_top:
