[BITS 16]
[ORG 0x7C00]

jmp 0x0000:main

main:
cli
xor ax, ax
mov ds, ax
mov ss, ax
mov sp, 0x7C00
sti

; Save boot drive
mov [0x7E00], dl

; Reset disk system
xor ax, ax
mov dl, [0x7E00]
int 0x13
jc error

; Load kernel to 0x8000 (512-byte aligned)
mov ax, 0x800
mov es, ax
xor bx, bx

; Read 4 sectors (2KB) starting from sector 2
mov ah, 0x02
mov al, 4
mov ch, 0
mov cl, 2
mov dh, 0
mov dl, [0x7E00]
int 0x13
jc error

; Jump to kernel
jmp 0x800:0x0000

error:
mov ax, 0x0E45
int 0x10
hlt
jmp $

times 508-($-$$) db 0
dw 0xAA55