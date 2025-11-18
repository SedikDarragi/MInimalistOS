[bits 16]
; Switch to protected mode
switch_to_pm:
    cli                     ; Disable interrupts
    
    ; Load the GDT descriptor
    lgdt [gdt_descriptor]
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to flush the pipeline and reload CS
    jmp CODE_SEG:init_pm

[bits 32]
; Initialize registers and stack in protected mode
init_pm:
    ; Update segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Update stack pointer
    mov ebp, 0x90000
    mov esp, ebp
    
    ; Jump to protected mode code
    jmp BEGIN_PM
