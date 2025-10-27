[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Print '1'
    mov al, '1'
    int 0x10
    
    ; Load kernel to 0x10000 (64KB)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02    ; Read sectors
    mov al, 8       ; Sectors to read (4KB)
    mov ch, 0       ; Cylinder 0
    mov cl, 2       ; Sector 2 (1-based)
    mov dh, 0       ; Head 0
    int 0x13
    jc disk_error
    
    ; Print '2'
    mov al, '2'
    int 0x10
    
    ; Enable A20
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

disk_error:
    mov al, 'E'
    int 0x10
    jmp $

; GDT
gdt_start:
    dq 0                ; Null descriptor
    
    ; Code segment (0x08)
    dw 0xFFFF           ; Limit 0-15
    dw 0                ; Base 0-15
    db 0                ; Base 16-23
    db 0x9A             ; Access byte
    db 0xCF             ; Flags + Limit 16-19
    db 0                ; Base 24-31
    
    ; Data segment (0x10)
    dw 0xFFFF           ; Limit 0-15
    dw 0                ; Base 0-15
    db 0                ; Base 16-23
    db 0x92             ; Access byte
    db 0xCF             ; Flags + Limit 16-19
    db 0                ; Base 24-31

gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
pm_start:
    ; Set up segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Copy kernel from 0x10000 to 1MB
    cld
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 0x1000  ; 4KB
    rep movsd
    
    ; Show 'K' in top-left corner
    mov byte [0xB8000], 'K'
    mov byte [0xB8001], 0x0F
    
    ; Jump to kernel
    jmp 0x100000
    
    ; Halt if we return (should never happen)
    cli
    hlt

times 510-($-$$) db 0
dw 0xAA55
