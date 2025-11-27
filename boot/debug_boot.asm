[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x1000  ; Where we'll load our kernel
CODE_SEG equ 8
DATA_SEG equ 16

_start:
    jmp main

print_str:
    pusha
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

disk_load:
    pusha
    mov bl, dh
    
    ; Debug: Print 'R' to show we're in disk_load
    mov al, 'R'
    mov ah, 0x0E
    int 0x10
    
    ; Use CHS read for hard disk
    mov ah, 0x02
    mov al, bl
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    mov dl, 0x80
    int 0x13
    jc .error
    
    ; Debug: Print 'D' to confirm disk load succeeded
    mov al, 'D'
    mov ah, 0x0E
    int 0x10
    popa
    ret
    
.error:
    mov al, 'E'
    mov ah, 0x0E
    int 0x10
    jmp $

; GDT
gdt_start:
    ; Null descriptor (required)
    dq 0x0
    
    ; Code segment descriptor
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0
    
    ; Data segment descriptor
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start + 0x7C00

[bits 16]
switch_to_pm:
    ; Debug: Print 'S' before switching to protected mode
    mov al, 'S'
    mov ah, 0x0E
    int 0x10
    
    cli
    
    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Debug: Print 'G' after loading GDT
    mov al, 'G'
    mov ah, 0x0E
    int 0x10
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Debug: Print 'P' after enabling protected mode
    mov al, 'P'
    mov ah, 0x0E
    int 0x10
    
    ; Far jump to 32-bit code
    jmp CODE_SEG:protected_mode_entry

[bits 32]
protected_mode_entry:
    ; Set up data segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Set up stack
    mov esp, 0x90000
    
    ; Debug: Write '3' to VGA memory
    mov byte [0xB8000], '3'
    mov byte [0xB8001], 0x0F
    
    ; Jump to kernel
    jmp KERNEL_OFFSET

[bits 16]
main:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    sti
    
    mov [boot_drive], dl
    
    mov si, msg_boot
    call print_str
    
    ; Load kernel
    mov bx, KERNEL_OFFSET
    mov es, bx
    xor bx, bx
    mov ax, 0x0000
    mov es, ax
    mov bx, KERNEL_OFFSET
    mov dh, 32
    mov dl, 0x80
    call disk_load
    
    ; Switch to protected mode
    call switch_to_pm

msg_boot: db "Booting...", 0
boot_drive: db 0

times 510 - ($-$$) db 0
dw 0xAA55