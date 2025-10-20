BITS 16
ORG 0x7C00

_start:
    jmp 0x0000:start     ; Ensure CS = 0x0000

; Boot disk number
boot_drive db 0

start:
    ; Basic setup
    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Save boot drive (passed from BIOS in DL)
    mov [boot_drive], dl
    
    ; Set text mode (80x25, 16 colors)
    mov ax, 0x0003
    int 0x10
    
    ; Print 'B' in top-left corner
    mov ah, 0x0E
    mov al, 'B'
    int 0x10
    
    ; Set up destination (0x1000:0x0000 = 0x10000 for temporary storage)
    ; We'll load the kernel at 1MB (0x100000) in chunks
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    ; Read kernel from disk (sectors 2-17, 8KB)
    mov ah, 0x02        ; Read sectors
    mov al, 16          ; Sectors to read (8KB)
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Sector 2 (1-based)
    mov dh, 0           ; Head 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    ; Enable A20 line to access memory above 1MB
    call enable_a20
    
    ; Set up to copy kernel to 1MB (0x100000)
    mov ax, 0x1000
    mov ds, ax
    xor si, si          ; Source: 0x10000 (0x1000:0x0000)
    
    mov ax, 0x1000
    mov es, ax
    mov di, 0x0000      ; Destination: 0x100000 (0x1000:0x0000)
    
    ; Copy 8KB (16 sectors * 512 bytes) from 0x10000 to 0x100000
    mov cx, 0x2000      ; 8KB = 0x2000 bytes
    cld
    rep movsb
    
    ; Print 'K' to indicate kernel loaded
    mov ah, 0x0E
    mov al, 'K'
    int 0x10
    
    ; Disable interrupts
    cli
    
    ; Enable A20 line (fast method)
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Load GDT
    cli
    lgdt [gdt_descriptor]
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to flush pipeline and set CS
    jmp 0x08:protected_mode
    
enable_a20:
    ; Try fast A20 method first
    in al, 0x92
    test al, 0x02
    jnz .a20_done
    or al, 0x02
    and al, 0xFE
    out 0x92, al
.a20_done:
    ret
    
; No longer needed - using simple A20 enable method

; Print character in AL
print_char:
    pusha
    mov ah, 0x0E        ; Teletype output
    xor bh, bh          ; Page 0
    int 0x10
    popa
    ret

; Print string (SI = string)
print_string:
    pusha
    cld
.loop:
    lodsb
    or al, al
    jz .done
    call print_char
    jmp .loop
.done:
    popa
    ret

; Print byte in AL as hex
print_hex_byte:
    pusha
    mov bl, al
    mov cx, 2
    mov ah, 0x0E
.next_nibble:
    rol bl, 4
    mov al, bl
    and al, 0x0F
    cmp al, 0x0A
    jl .is_digit
    add al, 7        ; 'A' - '0' - 10
.is_digit:
    add al, '0'
    int 0x10
    loop .next_nibble
    popa
    ret

BITS 32
protected_mode:
    ; Set up segment registers for flat memory model
    mov ax, 0x10        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; Set up stack at 0x90000
    
    ; Print 'K' in top-left corner
    mov byte [0xB8000], 'K'
    mov byte [0xB8001], 0x1F  ; White on blue
    
    ; Copy kernel to 1MB
    mov esi, 0x10000    ; Source (where we loaded the kernel)
    mov edi, 0x100000   ; Destination (1MB)
    mov ecx, 0x2000     ; 8KB (16 sectors * 512 bytes)
    rep movsd
    
    ; Set up a basic stack
    mov esp, 0x9FFFF    ; Stack grows down from 0x9FFFF
    
    ; Print 'K' to indicate we're about to jump to kernel
    mov byte [0xB8000 + 160], 'K'
    mov byte [0xB8001 + 160], 0x0E  ; Yellow on black
    
    ; Small delay to see the message
    mov ecx, 0x00FFFFFF
.delay_loop:
    dec ecx
    jnz .delay_loop
    
    ; Jump to kernel entry point
    jmp 0x08:0x100000   ; Use code segment selector 0x08 (from GDT)
    
    ; This will be reached only if the jump fails
    mov byte [0xB8000 + 320], '!'
    mov byte [0xB8001 + 320], 0x0C  ; Red on black
    jmp $

; GDT
gdt_start:
    ; Null descriptor (required)
    dq 0x0000000000000000
    
    ; Code segment (0x08) - 4GB flat, 32-bit, ring 0
    dw 0xFFFF             ; Limit (bits 0-15)
    dw 0x0000             ; Base (bits 0-15)
    db 0x00               ; Base (bits 16-23)
    db 0x9A               ; Access: Present, Ring 0, Code, Exec/Read
    db 0xCF               ; Flags: 4KB granularity, 32-bit, Limit[16:19]
    db 0x00               ; Base (bits 24-31)
    
    ; Data segment (0x10) - 4GB flat, 32-bit, ring 0
    dw 0xFFFF             ; Limit (bits 0-15)
    dw 0x0000             ; Base (bits 0-15)
    db 0x00               ; Base (bits 16-23)
    db 0x92               ; Access: Present, Ring 0, Data, Read/Write
    db 0xCF               ; Flags: 4KB granularity, 32-bit, Limit[16:19]
    db 0x00               ; Base (bits 24-31)
    
    ; Add more segments here if needed
    
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1    ; Size of GDT - 1
    dd gdt_start                  ; Start of GDT
    
    ; Segment selectors
    CODE_SEG equ 0x08    ; 0x08 = 00001 0 00 (index 1, GDT, RPL 0)
    DATA_SEG equ 0x10    ; 0x10 = 00010 0 00 (index 2, GDT, RPL 0)


; Pad to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55
