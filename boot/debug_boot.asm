[org 0x7c00]
[bits 16]

_start:
    jmp main

main:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Write 'B' to VGA
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0000], 'B'
    mov byte [es:0x0001], 0x0F
    pop es
    
    ; Write 'L' to VGA
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0002], 'L'
    mov byte [es:0x0003], 0x0F
    pop es
    
    ; Load kernel to 0x8000 (32KB)
    mov ax, 0x0800
    mov es, ax
    mov bx, 0x0000
    mov dh, 65  ; Load 65 sectors (33KB kernel)
    mov dl, [boot_drive]
    
    ; Reset disk system
    mov ah, 0x00
    int 0x13
    jc .disk_error
    
    ; Read 65 sectors from sector 16
    mov ah, 0x02
    mov al, 65
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x10
    int 0x13
    jc .disk_error
    
    ; Write 'S' to VGA
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0004], 'S'
    mov byte [es:0x0005], 0x0F
    pop es
    
    ; Jump to protected mode code
    jmp .disk_ok
    
.disk_error:
    ; Write 'E' for error
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0006], 'E'
    mov byte [es:0x0007], 0x0F
    pop es
    
    ; Write error code
    mov al, ah
    shr al, 4
    call .hex_digit
    mov bl, al
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0008], bl
    mov byte [es:0x0009], 0x0F
    pop es
    
    mov al, ah
    and al, 0x0F
    call .hex_digit
    mov bl, al
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x000A], bl
    mov byte [es:0x000B], 0x0F
    pop es
    
    jmp $

.hex_digit:
    cmp al, 9
    jle .hex_digit_num
    add al, 'A' - 10
    ret
.hex_digit_num:
    add al, '0'
    ret

.disk_ok:
    
    ; Write 'C' to show we're about to switch to PM
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0006], 'C'
    mov byte [es:0x0007], 0x0F
    pop es
    
    ; Switch to protected mode (inline)
    cli
    
    ; Write 'G' to show GDT loading
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0008], 'G'
    mov byte [es:0x0009], 0x0F
    pop es
    
    lgdt [gdt_descriptor]
    
    ; Write 'P' to show protected mode enabling
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x000A], 'P'
    mov byte [es:0x000B], 0x0F
    pop es
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Far jump to kernel with code segment selector
    ; This will load CS and transition to protected mode
    jmp 0x08:0x8000

; GDT
gdt_start:
    dq 0x0
    ; Code segment
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0
    ; Data segment
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

; Data
boot_drive db 0

times 510 - ($-$$) db 0
dw 0xAA55
