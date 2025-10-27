[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Load kernel directly to 1MB (0x100000)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02    ; Read sectors
    mov al, 1       ; 1 sector only
    xor ch, ch      ; Cylinder 0
    mov cl, 2       ; Sector 2 (1-based)
    xor dh, dh      ; Head 0
    int 0x13
    jc $
    
    ; Enable A20 (fast method)
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Load GDT and enter protected mode
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:pm_start

; Minimal GDT
gdt:
    ; Null descriptor
    dw 0, 0, 0, 0
    
    ; Code segment (0x08)
    dw 0xFFFF    ; Limit 0-15
    dw 0x0000    ; Base 0-15
    db 0x00      ; Base 16-23
    db 0x9A      ; Present, Ring 0, Code, Execute/Read
    db 0xCF      ; 4K granularity, 32-bit
    db 0x00      ; Base 24-31

gdt_desc:
    dw 0x17      ; GDT size - 1
    dd gdt       ; GDT address

times 8 db 0     ; Padding to align the protected mode code

[BITS 32]
pm_start:
    ; Set up segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Jump to kernel at 1MB
    jmp 0x08:0x100000

; Fill to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55
