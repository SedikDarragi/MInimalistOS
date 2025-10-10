; Kernel entry point
[bits 32]

; Export the entry point to the linker
global _start

; External functions
extern kmain
extern idt_init

section .data
align 4
kernel_msg db "MinimalOS Kernel is running!", 0
panic_msg db "Kernel panic: kmain returned!", 0

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

    ; Clear screen with blue background
    mov edi, 0xB8000    ; VGA text buffer
    mov ecx, 80*25      ; 80x25 characters
    mov eax, 0x1F201F20 ; Blue background, white spaces (two characters at a time)
    rep stosd

    ; Print kernel message
    mov esi, kernel_msg
    mov edi, 0xB8000 + (80 * 1 + 10) * 2  ; Row 1, column 10
    mov ah, 0x1F        ; White on blue
.print_loop:
    lodsb
    or al, al
    jz .print_done
    stosw
    jmp .print_loop
.print_done:

    ; Initialize IDT
    call idt_init

    ; Call the C kernel main function
    call kmain

    ; If kmain returns, show panic message
    mov esi, panic_msg
    mov edi, 0xB8000 + (80 * 3 + 10) * 2  ; Row 3, column 10
    mov ah, 0x4F        ; White on red
.panic_loop:
    lodsb
    or al, al
    jz .halt
    stosw
    jmp .panic_loop

    ; Halt the CPU if we get here
.halt:
    cli
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 16384    ; 16KB stack
stack_top:
