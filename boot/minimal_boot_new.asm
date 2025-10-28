[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Load kernel to 0x10000 (64KB)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    ; Reset disk system
    mov ah, 0x00
    int 0x13
    jc disk_error
    
    ; Read kernel (2 sectors = 1KB)
    mov ah, 0x02
    mov al, 2
    mov ch, 0
    mov cl, 2
    mov dh, 0
    int 0x13
    jc disk_error
    
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
    mov si, error_msg
    call print_string
    jmp $

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

; Minimal GDT (code segment only)
gdt:
    dq 0
    dw 0xFFFF, 0, 0x9A00, 0x00CF  ; Code

gdt_desc:
    dw 0x0F
    dd gdt

error_msg db 'E', 0

times 446-($-$$) db 0
dw 0xAA55

[BITS 32]
pm_start:
    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    
    ; Jump to kernel at 0x10000
    jmp 0x08:0x10000
