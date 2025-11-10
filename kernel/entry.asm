[bits 16]
section .text.entry
global _start

_start:
    jmp short start_protected_mode

; --- GDT (Global Descriptor Table) ---
gdt_start:
    ; Null descriptor (required)
    dq 0x0
    ; Code Segment (0x08): base=0, limit=4GB, 32-bit, page-granular, r/x
    dw 0xFFFF       ; Limit (low)
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (mid)
    db 0x9A         ; Access (P=1, DPL=0, S=1, Type=Code, R/E)
    db 0xCF         ; Granularity (G=1, D=1), Limit (high)
    db 0x00         ; Base (high)
    ; Data Segment (0x10): base=0, limit=4GB, 32-bit, page-granular, r/w
    dw 0xFFFF       ; Limit (low)
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (mid)
    db 0x92         ; Access (P=1, DPL=0, S=1, Type=Data, R/W)
    db 0xCF         ; Granularity (G=1, D=1), Limit (high)
    db 0x00         ; Base (high)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; GDT limit
    dd gdt_start              ; GDT base (will be patched)

CODE_SEG equ 0x08
DATA_SEG equ 0x10

start_protected_mode:
    cli

    ; Patch the GDT descriptor with the absolute address of the GDT.
    ; The kernel is loaded at 0x10000.
    mov dword [gdt_descriptor + 2], 0x10000 + gdt_start
    lgdt [gdt_descriptor]

    ; Enable protected mode (PE bit in CR0)
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush the pipeline and load CS with the 32-bit code segment.
    jmp CODE_SEG:enter_32bit

[bits 32]
enter_32bit:
    ; Now in 32-bit protected mode.
    ; Set up the data segment registers.
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up the stack.
    mov esp, stack_top

    ; Call the C kernel.
    extern kmain
    call kmain

    ; If kmain returns, hang.
    cli
    hlt

section .bss
resb 8192 ; 8KB stack
stack_top:
