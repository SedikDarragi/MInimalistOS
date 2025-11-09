[bits 16]
section .text.entry
global _start

_start:
    ; The bootloader loads us at 0x10000. We are in 16-bit real mode.
    ; We need to switch to 32-bit protected mode.
    cli
    lgdt [gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush the pipeline and load CS with our 32-bit code segment
    jmp CODE_SEG:protected_mode_entry

[bits 32]
protected_mode_entry:
    ; Now in 32-bit protected mode.
    ; Set up segment registers.
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up the stack
    mov esp, stack_top

    ; Call the C kernel main function
    extern kmain
    call kmain

    ; If kmain returns, hang the system
    cli
    hlt

; --- GDT (Global Descriptor Table) ---
gdt_start:
    ; Null descriptor
    dq 0x0

    ; Code Segment (0x08)
    ; Base=0, Limit=0xFFFFF, Granularity=4KB, 32-bit, P, DPL=0, S, Type=Code, R/E
    dw 0xFFFF       ; Limit (low)
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (mid)
    db 0x9A         ; Access (P=1, DPL=0, S=1, Type=Code, R/E)
    db 0xCF         ; Granularity (G=1, D=1, L=0, AVL=0), Limit (high)
    db 0x00         ; Base (high)

    ; Data Segment (0x10)
    ; Base=0, Limit=0xFFFFF, Granularity=4KB, 32-bit, P, DPL=0, S, Type=Data, R/W
    dw 0xFFFF       ; Limit (low)
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (mid)
    db 0x92         ; Access (P=1, DPL=0, S=1, Type=Data, R/W)
    db 0xCF         ; Granularity (G=1, D=1, L=0, AVL=0), Limit (high)
    db 0x00         ; Base (high)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ 0x08
DATA_SEG equ 0x10

section .bss
resb 8192 ; 8KB stack
stack_top:
