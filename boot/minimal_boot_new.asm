[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    push dx
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 1
    xor ch, ch
    mov cl, 2
    xor dh, dh
    pop dx
    int 0x13
    jc error
    jmp 0x1000:0x0000

error:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    jmp $

times 508-($-$$) db 0
db 0x55, 0xAA