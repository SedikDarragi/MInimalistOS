BITS 16
ORG 0x7C00

_start:
    jmp 0x0000:start

boot_drive db 0

start:
    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Clear screen first
    mov ax, 0x0003
    int 0x10

    ; --- 1. Bootloader started ---
    mov ah, 0x0E
    mov al, '1'
    int 0x10

    mov [boot_drive], dl

    ; --- 2. Boot drive saved ---
    mov ah, 0x0E
    mov al, '2'
    int 0x10

    ; Load kernel from disk into 0x10000
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 16
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; --- 3. Kernel loaded to 0x10000 ---
    mov ah, 0x0E
    mov al, '3'
    int 0x10

    ; Enable A20 line
    call enable_a20

    ; --- 4. A20 enabled ---
    mov ah, 0x0E
    mov al, '4'
    int 0x10

    ; Load GDT and switch to protected mode
    cli
    lgdt [gdt_descriptor]

    ; --- 5. GDT loaded ---
    mov ah, 0x0E
    mov al, '5'
    int 0x10

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Far jump to flush CPU pipeline
    jmp CODE_SEG:protected_mode

; --- Error Handlers ---
disk_error:
    mov ah, 0x0E
    mov al, 'D'
    int 0x10
    jmp $

halt:
    mov ah, 0x0E
    mov al, 'H'
    int 0x10
    cli
    hlt

enable_a20:
    in al, 0x92
    test al, 0x02
    jnz .a20_done
    or al, 0x02
    and al, 0xFE
    out 0x92, al
.a20_done:
    ret

; ===============================================
; Protected Mode Code
; ===============================================
BITS 32
protected_mode:
    ; Set up data segments
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9FFFF

    ; --- 6. Entered Protected Mode ---
    mov byte [0xB8000], '6'
    mov byte [0xB8001], 0x0F

    ; Copy kernel from 0x10000 to 0x100000
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 16 * 512 / 4 ; 16 sectors, 512 bytes each, copy 4 bytes at a time
    rep movsd

    ; --- 8. Kernel copied to 1MB ---
    mov byte [0xB8002], '8'
    mov byte [0xB8003], 0x0F

    ; Jump to kernel entry point
    call 0x100000

    ; Should not return
    mov byte [0xB8004], 'F' ; F for Fail
    mov byte [0xB8005], 0x0C
    jmp $

; ===============================================
; GDT
; ===============================================
gdt_start:
    ; Null descriptor
    dq 0x0

    ; Code Segment (0x08)
    dw 0xFFFF       ; Limit
    dw 0x0000       ; Base
    db 0x00         ; Base
    db 0x9A         ; P, DPL=0, S, Type=Code, R/E
    db 0xCF         ; G, D/B, L, AVL, Limit
    db 0x00         ; Base

    ; Data Segment (0x10)
    dw 0xFFFF       ; Limit
    dw 0x0000       ; Base
    db 0x00         ; Base
    db 0x92         ; P, DPL=0, S, Type=Data, R/W
    db 0xCF         ; G, D/B, L, AVL, Limit
    db 0x00         ; Base
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ 0x08
DATA_SEG equ 0x10

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
