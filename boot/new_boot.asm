[org 0x7c00]
[bits 16]

KERNEL_LOAD_ADDR equ 0x1000 ; Load kernel to 0x10000 (segment 0x1000)
KERNEL_SECTORS   equ 60    ; Number of sectors to read

_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    ; Load kernel from disk
    mov ah, 0x02 ; Read sectors
    mov al, KERNEL_SECTORS
    mov ch, 0    ; Cylinder
    mov cl, 2    ; Sector
    mov dh, 0    ; Head
    mov bx, KERNEL_LOAD_ADDR
    mov es, bx
    xor bx, bx
    int 0x13
    jc read_error

    ; Switch to Protected Mode
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; The kernel is loaded at 0x10000, we want it at 0x100000
    mov esi, KERNEL_LOAD_ADDR * 16 ; Source: 0x10000
    mov edi, 0x100000              ; Destination: 1MB
    mov ecx, (KERNEL_SECTORS * 512) / 4
    rep movsd

    ; Jump to kernel entry point
    jmp 0x100000

read_error:
    mov si, read_error_msg
print_loop:
    lodsb
    cmp al, 0
    je halt
    mov ah, 0x0e
    int 0x10
    jmp print_loop

halt:
    cli
    hlt

read_error_msg db 'Kernel read error', 0

gdt_start:
    ; Null descriptor
    dq 0x0
    ; Code Segment (0x08)
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF
    ; Data Segment (0x10)
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ 0x08
DATA_SEG equ 0x10

times 510 - ($ - $$) db 0
dw 0xaa55
