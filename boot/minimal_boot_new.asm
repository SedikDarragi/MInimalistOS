[BITS 16]
[ORG 0x7C00]

start:
    ; Minimal setup
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Print '1'
    mov al, '1'
    call print_char
    
    ; Print '2'
    mov al, '2'
    call print_char
    
    ; Enable A20 (fast method)
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Load GDT and enter protected mode
    cli
    lgdt [gdt_desc]
    
    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax
    
    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax
    
    ; Print 'E' if we're about to jump to protected mode
    mov al, 'E'
    call print_char
    
    ; Far jump to 32-bit code
    jmp 0x08:pm_start

; Test if A20 is enabled (simplified, always returns enabled)
test_a20:
    or ax, 1    ; Set ZF=0 (enabled)
    ret

disk_error:
    mov al, 'E'
    call print_char
    mov al, 'R'
    call print_char
    mov al, 'R'
    call print_char
    jmp $

print_char:
    pusha
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    popa
    ret

boot_drive: db 0

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
    ; Setup segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Copy kernel from 0x10000 to 1MB (0x100000)
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 1024  ; 4KB
    rep movsd
    
    ; Print 'K' to show we're in kernel
    mov byte [0xB8000], 'K'
    mov byte [0xB8001], 0x0F
    
    ; Jump to kernel
    jmp 0x100000
    
    ; Halt if we return
    cli
    hlt

times 510-($-$$) db 0
dw 0xAA55
