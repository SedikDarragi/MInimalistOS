; Kernel entry point
[bits 32]

; Export the entry point to the linker
global _start

; External functions
extern kmain
extern idt_init

section .text
; Entry point
_start:
    ; Print 'E' to indicate we've entered the kernel
    mov byte [0xB8000 + 0], 'E'
    mov byte [0xB8001 + 0], 0x0A  ; Green on black
    
    ; Set up segment registers for flat memory model
    mov ax, 0x10        ; Data segment selector (0x10 points to our data segment in GDT)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9FFFF    ; Set up stack at 0x9FFFF (grows down)
    
    ; Print 'S' to indicate segment registers are set
    mov byte [0xB8000 + 2], 'S'
    mov byte [0xB8001 + 2], 0x0A  ; Green on black
    
    ; Clear direction flag (string operations increment)
    cld
    
    ; Print 'C' to indicate direction flag is cleared
    mov byte [0xB8000 + 4], 'C'
    mov byte [0xB8001 + 4], 0x0A  ; Green on black

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
    
    ; Call the main kernel function
    call kmain
    
    ; If kmain returns, show panic message and halt
    mov esi, panic_msg


section .data
align 4
kernel_msg db "MinimalOS Kernel is running!", 0
panic_msg db "Kernel panic: kmain returned!", 0
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
