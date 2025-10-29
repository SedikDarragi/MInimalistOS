[BITS 16]
[ORG 0x7C00]

; Data
boot_drive db 0
attempt db '0'

start:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [boot_drive], dl

    ; Print 'S'
    mov ah, 0x0E
    mov al, 'S'
    int 0x10

    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc error

    ; Set up destination (0x1000:0x0000 = 0x10000)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    ; Read kernel
    mov ah, 0x02
    mov al, 8
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc error

    ; Jump to kernel
    jmp 0x1000:0x0000

error:
    ; Print 'E' and error code
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    mov al, ah
    call print_hex
    hlt

print_hex:
    push ax
    mov cl, 4
    shr al, cl
    call print_digit
    pop ax
    and al, 0x0F
    call print_digit
    ret

print_digit:
    push ax
    cmp al, 10
    jb .digit
    add al, 7
.digit:
    add al, '0'
    mov ah, 0x0E
    int 0x10
    pop ax
    ret

; Pad to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55