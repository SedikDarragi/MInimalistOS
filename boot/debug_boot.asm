[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x1000
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
    mov ah, 0x02
    mov al, dh
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    int 0x13
    jc .error
    ; Debug: Print 'D' to confirm disk load succeeded
    mov al, 'D'
    mov ah, 0x0E
    int 0x10
    popa
    ret
.error:
    mov si, msg_err
    call print_str
    jmp $

gdt_start:
    dq 0x0
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

switch_to_pm:
    ; Debug: Print 'S' before switching to protected mode
    mov al, 'S'
    mov ah, 0x0E
    int 0x10
    
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp
    
    ; Debug: Write 'P' to VGA to confirm we're in protected mode
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0F
    
    ; Debug: Write 'J' to VGA before jumping to kernel
    mov byte [0xB8002], 'J'
    mov byte [0xB8003], 0x0A
    
    ; Halt here temporarily to see if we reach this point
    cli
.halt:
    hlt
    jmp .halt
    
    jmp CODE_SEG:KERNEL_OFFSET

[bits 16]
main:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti
    
    mov [boot_drive], dl
    
    mov si, msg_boot
    call print_str
    
    mov bx, KERNEL_OFFSET
    mov es, bx
    xor bx, bx
    mov dh, 15
    mov dl, [boot_drive]
    
    mov si, msg_load
    call print_str
    call disk_load
    
    ; Skip OK message, go directly to protected mode
    call switch_to_pm
    
    jmp $

msg_boot db 'Boot', 0x0D, 0x0A, 0
msg_load db 'Load', 0
msg_err db 'Err', 0
boot_drive db 0

%if ($ - $$) > 510
    %error "Bootloader exceeds 510 bytes"
%endif
times 510 - ($ - $$) db 0
dw 0xAA55