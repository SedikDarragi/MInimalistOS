[org 0x7c00]
[bits 16]

_start:
    cli
    cld

    ; Reset segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    sti

    ; Load kernel from disk
    mov ah, 0x02        ; BIOS read sector function
    mov al, 60          ; Read 60 sectors (30 KB)
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Start from sector 2 (sector 1 is the bootloader)
    mov dh, 0           ; Head 0
    mov bx, 0x1000      ; Load address (0x0000:0x1000)
    mov es, bx
    xor bx, bx

    int 0x13
    jc read_error

    ; Jump to kernel (0x1000:0000 which is 0x10000)
    jmp 0x1000:0x0000

read_error:
    ; Simple error message
    mov si, error_msg
    mov ah, 0x0e
.loop:
    lodsb
    cmp al, 0
    je .halt
    int 0x10
    jmp .loop
.halt:
    cli
    hlt

error_msg db 'Kernel load failed!', 0

; Padding and boot signature
times 510 - ($ - $$) db 0
dw 0xaa55
