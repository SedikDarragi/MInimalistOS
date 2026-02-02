BITS 16
ORG 0x7C00

_start:
    ; Minimal setup - no stack, no segments, no interrupts
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
    
    ; Print loading message
    mov si, loading_msg
    call print_string
    
    ; Load kernel to 0x10000 (64KB)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    ; Reset disk system
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    
    ; Read kernel from disk
    mov ah, 0x02    ; Read sectors
    mov al, 8       ; Sectors to read (4KB)
    mov ch, 0       ; Cylinder
    mov cl, 2       ; Sector (1-based)
    mov dh, 0       ; Head
    mov dl, [boot_drive]
    
    int 0x13
    jc disk_error
    
    ; Print dot for success
    mov si, dot_msg
    call print_string
    
    ; Enable A20
    call enable_a20
    
    ; Set up GDT and enter protected mode
    cli
    lgdt [gdt_descriptor]
    
    ; Set PE bit in CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to 32-bit code
    jmp CODE_SEG:protected_mode

; Function to enable A20 line (minimal method)
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

print_string:
    lodsb
    or al, al
    jz .done
    cmp al, 0x0D
    je .cr
    cmp al, 0x0A
    je .lf
    mov ah, 0x0E
    int 0x10
    jmp print_string
.cr:
    ; Handle carriage return - move cursor to beginning of line
    mov ah, 0x03    ; Get cursor position
    mov bh, 0       ; Page number
    int 0x10
    mov ah, 0x02    ; Set cursor position
    mov bh, 0       ; Page number
    xor dl, dl      ; Column 0
    int 0x10
    jmp print_string
.lf:
    ; Handle line feed - move cursor down one line
    mov ah, 0x03    ; Get cursor position
    mov bh, 0       ; Page number
    int 0x10
    inc dh          ; Increment row
    mov ah, 0x02    ; Set cursor position
    mov bh, 0       ; Page number
    int 0x10
    jmp print_string
.done:
    ret

; Print error and halt
disk_error:
    mov si, error_msg
    call print_string
    cli
    hlt

; Print character (destroys AX, BX)
print_char:
    mov ah, 0x0E
    int 0x10
    ret

; Protected mode data
pmode_msg db 0x0A, 0x0D, 'Protected mode!', 0x0A, 0x0D, 0

; GDT
gdt_start:
    dq 0x0
    ; Code segment
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 0x9A         ; Access byte
    db 0xCF         ; Granularity
    db 0x00         ; Base high
    ; Data segment
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 0x92         ; Access byte
    db 0xCF         ; Granularity
    db 0x00         ; Base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
    
    ; Segment selectors
    CODE_SEG equ 0x08
    DATA_SEG equ 0x10

loading_msg db 'Loading...', 0
dot_msg db '.', 0
error_msg db 0x0D, 0x0A, 'Boot error!', 0
boot_drive db 0

times 510-($-$$) db 0
dw 0xAA55

times 510-($-$$) db 0
dw 0xAA55
