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
    mov dh, 48  ; Load 48 sectors = 24KB (enough for kernel)
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
    ; Code segment - base 0x7C00 to match bootloader
    dw 0xFFFF       ; Limit low
    dw 0x7C00       ; Base low
    db 0x00         ; Base middle
    db 10011010b    ; Access
    db 11001111b    ; Flags + Limit high
    db 0x00         ; Base high
    ; Data segment - base 0x7C00 with I/O access
    dw 0xFFFF       ; Limit low
    dw 0x7C00       ; Base low
    db 0x00         ; Base middle
    db 10010010b    ; Access
    db 11001111b    ; Flags + Limit high
    db 0x00         ; Base high
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
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 1  ; Set PE bit (Protection Enable)
    mov cr0, eax
    
    ; Write 'P' to VGA to show protected mode enabled
    push es
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0x0008], 'P'
    mov byte [es:0x0009], 0x0F
    pop es
    
    ; Far jump to 32-bit code to complete the switch
    ; Use direct encoding for reliability
    db 0xEA
    dw protected_mode_entry
    dw CODE_SEG

[bits 32]
protected_mode_entry:
    ; Set up data segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov esp, 0x90000
    
    ; Write 'P' to VGA to show we're in protected mode
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0F
    
    ; Write '3' to VGA to show we're about to jump to kernel
    mov byte [0xB8002], '3'
    mov byte [0xB8003], 0x0F
    
    ; Write 'J' to VGA to show we're jumping
    mov byte [0xB8004], 'J'
    mov byte [0xB8005], 0x0F
    
    ; Write 'X' to show we're in protected mode
    mov byte [0xB8006], 'X'
    mov byte [0xB8007], 0x0F
    
    ; Hang here instead of jumping to kernel
    cli
.hang:
    hlt
    jmp .hang

; Data
boot_drive db 0

times 510 - ($-$$) db 0
dw 0xAA55