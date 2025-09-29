BITS 16
ORG 0x7C00

_start:
    ; Basic setup
    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Set text mode
    mov ax, 0x0003
    int 0x10
    
    ; Print boot message
    mov si, boot_msg
    call print_string
    
    ; Load kernel to 0x10000 (64KB)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    ; Reset disk system
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    ; Read kernel from disk
    mov ah, 0x02    ; Read sectors
    mov al, 8       ; Sectors to read (4KB)
    mov ch, 0       ; Cylinder
    mov cl, 2       ; Sector (1-based)
    mov dh, 0       ; Head
    mov dl, [boot_drive]
    
    int 0x13
    jc disk_error
    
    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Set up GDT
    lgdt [gdt_descriptor]
    
    ; Enter protected mode
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    cli
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to 32-bit code
    jmp CODE_SEG:protected_mode

; Print character in AL
print_char:
    pusha
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    popa
    ret

; Print newline
print_newline:
    pusha
    mov al, 0x0D
    call print_char
    mov al, 0x0A
    call print_char
    popa
    ret

; Print hex digit in AL (0-15)
print_hex_digit:
    pusha
    and al, 0x0F
    add al, '0'
    cmp al, '9'
    jbe .print
    add al, 7
.print:
    call print_char
    popa
    ret

; Print byte in AL as hex
print_hex_byte:
    pusha
    push ax
    shr al, 4
    call print_hex_digit
    pop ax
    call print_hex_digit
    popa
    ret

; Print word in AX as hex
print_hex_word:
    pusha
    push ax
    mov al, ah
    call print_hex_byte
    pop ax
    call print_hex_byte
    popa
    ret

; Print string (SI = string)
print_string:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    call print_char
    jmp .loop
.done:
    popa
    ret

; Error handler
disk_error:
    mov si, disk_error_msg
    call print_string
    
    ; Print error code in AH
    mov al, ah
    call print_hex_byte
    
    mov si, error_halt_msg
    call print_string
    
    cli
    hlt

BITS 32
protected_mode:
    ; Set up segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Clear screen
    mov edi, 0xB8000
    mov ecx, 80*25
    mov ah, 0x1F    ; White on blue
    mov al, ' '
    rep stosw
    
    ; Jump to kernel
    jmp 0x10000

; GDT
gdt_start:
    dq 0x0
    ; Code segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00
    ; Data segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
    
    ; Segment selectors
    CODE_SEG equ 0x08
    DATA_SEG equ 0x10

; Messages
boot_msg        db 'Booting...', 0x0D, 0x0A, 0
ok_msg          db 'OK', 0x0D, 0x0A, 0
disk_error_msg  db 0x0D, 0x0A, 'Disk error: 0x', 0
error_halt_msg  db 0x0D, 0x0A, 'Halted.', 0

boot_drive db 0

times 510-($-$$) db 0
dw 0xAA55
