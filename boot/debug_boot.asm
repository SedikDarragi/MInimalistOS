[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x1000
CODE_SEG equ 8
DATA_SEG equ 16

_start:
    jmp main

main:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    
    ; Save boot drive (passed by BIOS in DL)
    mov [boot_drive], dl
    
    ; Write 'B' to VGA to show we're in main
    ; Use segment:offset addressing for VGA memory
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0000], 'B'
    mov byte [es:0x0001], 0x0F
    pop es
    
    ; Load kernel to 0x1000
    mov ax, 0x0000
    mov es, ax
    mov bx, KERNEL_OFFSET
    mov dh, 80  ; Load 80 sectors = 40KB (enough for 33KB kernel)
    mov dl, [boot_drive]  ; Use boot drive passed by BIOS
    call disk_load
    
    ; Write 'L' to VGA to show disk load completed
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0002], 'L'
    mov byte [es:0x0003], 0x0F
    pop es
    
    ; Switch to protected mode
    call switch_to_pm

disk_load:
    pusha
    mov bl, dh
    mov ah, 0x02
    mov al, bl
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x01  ; Sector 1 (where kernel is placed)
    mov dl, 0x80
    int 0x13
    jc .error
    popa
    ret
.error:
    jmp $

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
    ; Data segment - with I/O access
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
    cli
    
    ; Write 'S' to VGA to show we're in switch_to_pm
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0004], 'S'
    mov byte [es:0x0005], 0x0F
    pop es
    
    lgdt [gdt_descriptor]
    
    ; Write 'G' to VGA to show GDT loaded
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0006], 'G'
    mov byte [es:0x0007], 0x0F
    pop es
    
    ; Write '1' to show we're about to read CR0
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x000A], '1'
    mov byte [es:0x000B], 0x0F
    pop es
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 1  ; Set PE bit (Protection Enable)
    mov cr0, eax
    
    ; Write '2' to show we wrote CR0
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x000C], '2'
    mov byte [es:0x000D], 0x0F
    pop es
    
    ; Write 'P' to VGA to show protected mode enabled
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x000E], 'P'
    mov byte [es:0x000F], 0x0F
    pop es
    
    ; Far jump to 32-bit code to complete the switch
    ; Jump to kernel directly at 0x100C
    jmp CODE_SEG:0x100C

; Data
boot_drive db 0

times 510 - ($-$$) db 0
dw 0xAA55