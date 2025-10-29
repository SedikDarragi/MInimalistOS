[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [boot_drive], dl

    ; Set up destination (0x1000:0x0000 = 0x10000)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    ; Read kernel (single sector)
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc error

    ; Jump to kernel
    jmp 0x1000:0x0000

error:
    ; Simple error indicator
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    jmp $

boot_drive db 0

; Pad to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55