[bits 16]
section .text.entry
global _start

_start:
    cli
    ; Load the GDT. We use a trick here: `lgdt [cs:gdt_descriptor]` tells the CPU
    ; to use the CS segment for addressing, which is what we need in real mode.
    lgdt [cs:gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to our 32-bit code segment to flush the CPU pipeline
    jmp CODE_SEG:enter_32bit

; --- GDT and Descriptor --- 
; This section is still part of the 16-bit code segment for addressing purposes.
gdt_start:
    ; Null descriptor
    dq 0x0
    ; Code Segment (0x08)
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF
    ; Data Segment (0x10)
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd 0x10000 + gdt_start ; Absolute physical address of the GDT

CODE_SEG equ 0x08
DATA_SEG equ 0x10

[bits 32]
enter_32bit:
    ; Now we are in 32-bit Protected Mode
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up the stack
    mov esp, stack_top

    ; Call the C kernel
    extern kmain
    call kmain

    ; Hang if kmain returns
    cli
    hlt

section .bss
resb 8192 ; 8KB stack
stack_top:
