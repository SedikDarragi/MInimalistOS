BITS 16
ORG 0x7C00

_start:
    jmp 0x0000:start     ; Ensure CS = 0x0000
    nop
    
; BIOS Parameter Block (FAT12 format)
oem_name          db 'MINIMALOS'
bytes_per_sector  dw 512
sectors_per_clust db 1
reserved_sects    dw 1
num_fats          db 2
root_entries      dw 224
total_sects       dw 2880
media             db 0xF0
sectors_per_fat    dw 9
sectors_per_track  dw 18
num_heads         dw 2
hidden_sects      dd 0
total_sects_large  dd 0
drive_num         db 0
reserved          db 0
boot_sig          db 0x29
volume_id         dd 0x12345678
volume_label      db 'MINIMAL OS  '
file_system       db 'FAT12   '

start:
    ; Basic setup
    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Save boot drive (passed from BIOS in DL)
    mov [boot_drive], dl
    
    ; Set text mode (80x25, 16 colors)
    mov ax, 0x0003
    int 0x10
    
    ; Print boot message
    mov si, boot_msg
    call print_string
    
    ; Load kernel to 0x100000 (1MB)
    ; We'll load it in chunks since we're in real mode
    
    ; First, reset disk system
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    ; Set up destination (0x10000 for temporary storage)
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
    
    ; Print success message
    mov si, read_ok_msg
    call print_string
    
    ; Disable interrupts
    cli
    
    ; Enable A20 line (fast method)
    call enable_a20
    
    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Set protected mode bit
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Far jump to 32-bit code to flush the pipeline
    jmp CODE_SEG:protected_mode
    
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

; Error handler
disk_error:
    mov si, error_msg
    call print_string
    mov al, ah
    call print_hex_byte
    mov si, crlf
    call print_string
    cli
.hang:
    hlt
    jmp .hang

print_hex_byte:
    pusha
    mov bl, al
    shr al, 4
    call .nibble
    mov al, bl
    and al, 0x0F
    call .nibble
    popa
    ret
.nibble:
    add al, '0'
    cmp al, '9'
    jbe .print
    add al, 7
.print:
    call print_char
    ret

BITS 32
protected_mode:
    ; Set up segment registers for flat memory model
    mov ax, 0x10        ; Data segment selector (0x10 points to our data segment in GDT)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; Set up stack at 0x90000 (grows down)
    
    ; Clear direction flag (string operations increment)
    cld
    
    ; Clear screen with blue background
    mov edi, 0xB8000    ; VGA text buffer
    mov ecx, 80*25      ; 80x25 characters
    mov eax, 0x1F201F20 ; Blue background, white spaces (two characters at a time)
    rep stosd
    
    ; Print message
    mov esi, pm_msg
    mov edi, 0xB8000 + (80 * 2 + 20) * 2  ; Row 2, column 20
    mov ah, 0x1F        ; White on blue
.print_loop:
    lodsb
    or al, al           ; Check for null terminator
    jz .copy_kernel
    stosw               ; Write character and attribute
    jmp .print_loop
    
.copy_kernel:
    ; Copy kernel from 0x10000 to 0x100000 (1MB)
    mov esi, 0x10000    ; Source (where we loaded the kernel)
    mov edi, 0x100000   ; Destination (1MB)
    mov ecx, 0x2000     ; Copy 8KB (16 sectors * 512 bytes)
    rep movsd           ; Copy dwords (faster than bytes)
    
    ; Jump to the kernel
    jmp 0x08:0x100000   ; Use code segment selector with proper privilege level
    
    ; Halt if we return (shouldn't happen)
    cli
.hang:
    hlt
    jmp .hang

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

; Messages
boot_msg        db 'Booting...', 0x0D, 0x0A, 0
read_ok_msg     db 'Kernel loaded', 0x0D, 0x0A, 0
error_msg       db 'Error: 0x', 0
crlf            db 0x0D, 0x0A, 0
pm_msg          db 'Entered protected mode!', 0

boot_drive db 0

times 510-($-$$) db 0
dw 0xAA55
