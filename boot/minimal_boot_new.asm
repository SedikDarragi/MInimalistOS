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
    
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Print '1'
    mov al, '1'
    call print_char
    
    ; Load kernel from disk to 0x10000 (64KB)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02    ; Read sectors
    mov al, 8       ; Number of sectors to read (4KB)
    mov ch, 0       ; Cylinder 0
    mov cl, 2       ; Sector 2 (1-based)
    mov dh, 0       ; Head 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    ; Print '2' if disk read succeeded
    mov al, '2'
    call print_char
    
    ; Enable A20 (fast method)
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Load GDT
    cli
    lgdt [gdt_desc]
    
    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax
    
    ; Far jump to 32-bit code with segment selector 0x08
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

disk_error:
    mov al, 'E'
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
    cld
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 0x1000  ; 4KB
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
