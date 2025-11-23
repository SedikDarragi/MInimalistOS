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
    ; Save the number of sectors
    mov bl, dh
    
    ; Debug: Print 'R' to show we're in disk_load
    mov al, 'R'
    mov ah, 0x0E
    int 0x10
    
    ; Use CHS read for hard disk
    mov ah, 0x02
    mov al, bl  ; Number of sectors to read (saved in bl)
    mov ch, 0x00  ; Cylinder 0
    mov dh, 0x00  ; Head 0
    mov cl, 0x02  ; Sector 2 (after MBR)
    mov dl, 0x80  ; First hard disk
    int 0x13
    ; jc .error  ; Temporarily remove carry check
    
    ; Debug: Print 'D' to confirm disk load succeeded
    mov al, 'D'
    mov ah, 0x0E
    int 0x10
    
    ; Debug: Print 'Q' after 'D'
    mov al, 'Q'
    mov ah, 0x0E
    int 0x10
    
    popa
    ret
.error:
    ; Debug: Print 'E' for error
    mov al, 'E'
    mov ah, 0x0E
    int 0x10
    mov si, msg_err
    call print_str
    jmp $

; GDT
gdt_start:
    ; Null descriptor (required)
    dq 0x0
    
    ; Code segment descriptor
    dw 0xFFFF     ; Limit (bits 0-15)
    dw 0x0        ; Base (bits 0-15)
    db 0x0        ; Base (bits 16-23)
    db 10011010b  ; Access byte
    db 11001111b  ; Flags + Limit (bits 16-19)
    db 0x0        ; Base (bits 24-31)
    
    ; Data segment descriptor
    dw 0xFFFF     ; Limit (bits 0-15)
    dw 0x0        ; Base (bits 0-15)
    db 0x0        ; Base (bits 16-23)
    db 10010010b  ; Access byte
    db 11001111b  ; Flags + Limit (bits 16-19)
    db 0x0        ; Base (bits 24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size of GDT
    dd gdt_start + 0x7C00      ; Base address of GDT (add load address)

[bits 16]
switch_to_pm:
    ; Debug: Print 'S' before switching to protected mode
    mov al, 'S'
    mov ah, 0x0E
    int 0x10
    
    cli
    
    ; Debug: Print 'G' without loading GDT first
    mov al, 'G'
    mov ah, 0x0E
    int 0x10
    
    ; Debug: Print 'L' immediately after G
    mov al, 'L'
    mov ah, 0x0E
    int 0x10
    
    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Debug: Print 'I' immediately after lgdt
    mov al, 'I'
    mov ah, 0x0E
    int 0x10
    
    ; Debug: Print 'G' after loading GDT
    mov al, 'G'
    mov ah, 0x0E
    int 0x10
    
    ; Debug: Write 'M' to VGA to confirm GDT is loaded
    ; mov byte [0xB8000+6], 'M'
    ; mov byte [0xB8000+7], 0x09
    
    ; Debug: Print 'T' after memory operations
    mov al, 'T'
    mov dx, 0x3F8
    out dx, al
    
    ; Debug: Print 'L' after loading GDT
    mov al, 'L'
    mov dx, 0x3F8
    out dx, al
    
    ; Debug: Print 'U' before cli
    mov al, 'U'
    mov dx, 0x3F8
    out dx, al
    
    ; cli  ; Temporarily remove
    
    ; Debug: Print 'C' before accessing CR0
    mov al, 'C'
    mov dx, 0x3F8
    out dx, al
    
    ; Debug: Write 'C' to VGA before CR0 write
    ; mov byte [0xB8000], 'C'
    ; mov byte [0xB8001], 0x0C
    
    ; Enable protected mode
    ; mov eax, cr0 - manual encoding for 16-bit mode
    db 0x0F, 0x20, 0xC0  ; mov eax, cr0
    
    ; Debug: Print 'R' after CR0 read
    mov al, 'R'
    mov dx, 0x3F8
    out dx, al
    
    or eax, 0x1
    
    ; Debug: Print 'O' after OR instruction
    mov al, 'O'
    mov dx, 0x3F8
    out dx, al
    
    ; mov cr0, eax - manual encoding for 16-bit mode
    db 0x0F, 0x22, 0xC0  ; mov cr0, eax
    
    ; Debug: Print 'W' after CR0 write
    mov al, 'W'
    mov dx, 0x3F8
    out dx, al
    
    ; Debug: Print 'J' before far jump
    mov al, 'J'
    mov dx, 0x3F8
    out dx, al
    
    ; Disable interrupts before far jump
    cli
    
    ; Far jump to flush pipeline and enter protected mode
    ; jmp far 0x08:0x7CC5 - manual encoding
    db 0xEA  ; jmp far opcode
    dw 0x7CC5  ; offset
    dw 0x0008  ; segment

[bits 32]
protected_mode_entry:
    ; Set up data segment registers first
    mov ax, 0x10  ; Data selector
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Debug: Write 'P' to VGA in protected mode
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0F
    
    ; Debug: Print 'S' after segment setup
    mov al, 'S'
    mov dx, 0x3F8
    out dx, al
    
    ; Debug: Write '3' to VGA to confirm we're in 32-bit protected mode
    ; mov dword [0xB8004], 0x0F330F33  ; Two '3's
    
    ; Debug: Write 'K' to VGA before jumping to kernel
    ; mov dword [0xB8008], 0x0B4B0B4B  ; Two 'K's
    
    ; Debug: Write 'J' to serial before jumping
    mov dx, 0x3F8
    mov al, 'J'
    out dx, al
    
    ; Jump to kernel entry point
    jmp 0x08:0x102F

[bits 16]
main:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000  ; Use a safer location for stack
    sti
    
    mov [boot_drive], dl
    
    mov si, msg_boot
    call print_str
    
    mov bx, KERNEL_OFFSET
    mov es, bx
    xor bx, bx
    
    ; Fix buffer address - we want to load at 0x1000, not 0x10000
    mov ax, 0x0000
    mov es, ax
    mov bx, KERNEL_OFFSET
    
    mov dh, 32  ; Load 32 sectors (16KB) to get the full kernel
    mov dl, 0x80  ; First hard disk
    
    mov si, msg_load
    call print_str
    call disk_load
    
    ; Debug: Print 'Z' after disk_load returns
    mov al, 'Z'
    mov ah, 0x0E
    int 0x10
    
    ; Debug: Print 'X' before calling switch_to_pm
    mov al, 'X'
    mov ah, 0x0E
    int 0x10
    
    ; Debug: Print 'Y' before calling switch_to_pm
    mov al, 'Y'
    mov ah, 0x0E
    int 0x10
    
    ; Debug: Print 'M' before the jump
    mov al, 'M'
    mov ah, 0x0E
    int 0x10
    
    jmp switch_to_pm
    
    jmp $

msg_boot db 'Boot', 0x0D, 0x0A, 0
msg_load db 'Load', 0
msg_err db 'Err', 0
boot_drive db 0

; Disk packet for LBA reads
disk_packet:
    db 0x10      ; Packet size (16 bytes)
    db 0         ; Reserved
    dw 1         ; Number of sectors to read
    dw KERNEL_OFFSET  ; Buffer offset
    dw 0         ; Buffer segment (0 for real mode)
    dq 1         ; LBA sector number (start from sector 1 after MBR)

%if ($ - $$) > 510
    %error "Bootloader exceeds 510 bytes"
%endif
times 510 - ($ - $$) db 0
dw 0xAA55