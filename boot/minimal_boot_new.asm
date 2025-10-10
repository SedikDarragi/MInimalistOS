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

    ; Enable A20 line to access memory above 1MB
    call enable_a20
    jc a20_error

    ; Load kernel at 1MB (0x100000)
    ; We'll load it in 64KB chunks using INT 0x13 with LBA
    
    ; Set up disk address packet for INT 0x13 extensions
    mov dword [dap.start_lba], 1      ; Start from sector 1 (after boot sector)
    mov dword [dap.start_lba + 4], 0  ; 32-bit LBA
    mov word [dap.segment], 0x1000    ; Target segment (0x1000:0x0000 = 0x10000)
    mov word [dap.offset], 0x0000     ; Target offset
    mov word [dap.count], 128         ; Number of sectors to read (64KB)
    mov byte [dap.zero], 0
    
    ; Read sectors using INT 0x13 with LBA
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap
    int 0x13
    jc disk_error
    
    ; Print 'K' to indicate kernel loaded
    mov ah, 0x0E
    mov al, 'K'
    int 0x10
    
    ; Set up GDT and enter protected mode
    cli
    lgdt [gdt_descriptor]
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to clear pipeline and set CS
    jmp 0x08:protected_mode

BITS 32
protected_mode:
    ; Set up segment registers for flat memory model
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; Set up stack at 0x90000
    
    ; Jump to kernel at 1MB (0x100000)
    jmp 0x08:0x100000

; Enable A20 line using various methods
enable_a20:
    ; Method 1: BIOS
    mov ax, 0x2401
    int 0x15
    jc .try_fast_a20  ; If failed, try next method
    call test_a20
    jnc .done        ; If A20 is enabled, we're done
    
.try_fast_a20:
    ; Method 2: Fast A20
    in al, 0x92
    test al, 0x02
    jnz .skip_fast_a20  ; Already enabled
    or al, 2
    and al, 0xFE        ; Don't accidentally reset the system
    out 0x92, al
    call test_a20
    jnc .done
    
.skip_fast_a20:
    ; Method 3: Keyboard controller
    call .a20_wait_input
    mov al, 0xAD
    out 0x64, al
    
    call .a20_wait_input
    mov al, 0xD0
    out 0x64, al
    
    call .a20_wait_output
    in al, 0x60
    push eax
    
    call .a20_wait_input
    mov al, 0xD1
    out 0x64, al
    
    call .a20_wait_input
    pop eax
    or al, 2
    out 0x60, al
    
    call .a20_wait_input
    mov al, 0xAE
    out 0x64, al
    
    call test_a20
    jnc .done
    
    ; If we get here, A20 couldn't be enabled
    stc
    ret
    
.a20_wait_input:
    in al, 0x64
    test al, 2
    jnz .a20_wait_input
    ret
    
.a20_wait_output:
    in al, 0x64
    test al, 1
    jz .a20_wait_output
    ret
    
.done:
    clc
    ret

; Test if A20 is enabled
; Returns with carry clear if A20 is enabled
test_a20:
    pushad
    
    ; Save interrupt state and disable interrupts
    pushfd
    cli
    
    ; Save values at 0x0000:0x7DFE and 0xFFFF:0x7E0E
    mov ax, [0x7DFE]  ; 0x7C00 + 0x1FE = 0x7DFE (last word of boot sector)
    mov bx, [0x7E0E]  ; 0xFFFF0 + 0x7E0E = 0x107DFE (wraps around to 0x7DFE if A20 is off)
    
    ; Write different values
    mov word [0x7DFE], 0x1234
    mov word [0x7E0E], 0x5678
    
    ; Check if values are different
    cmp word [0x7DFE], 0x1234
    jne .a20_off
    
    ; Restore original values
    mov [0x7DFE], ax
    mov [0x7E0E], bx
    
    ; Check if they match (A20 is on if they don't)
    cmp ax, [0x7E0E]
    jne .a20_on
    
.a20_off:
    stc
    jmp .done
    
.a20_on:
    clc
    
.done:
    ; Restore original values
    mov [0x7DFE], ax
    mov [0x7E0E], bx
    
    ; Restore interrupt state
    popfd
    popad
    ret

disk_error:
    mov si, disk_error_msg
    call print_string_16
    jmp $

a20_error:
    mov si, a20_error_msg
    call print_string_16
    jmp $

; Print a null-terminated string in real mode
; DS:SI = string address
print_string_16:
    pusha
    mov ah, 0x0E
    mov bh, 0
.repeat:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .repeat
.done:
    popa
    ret

; Disk Address Packet for INT 0x13 extensions
dap:
    .size:      db 0x10
    .zero:      db 0
    .count:     dw 0
    .offset:    dw 0
    .segment:   dw 0
    .start_lba: dq 0

disk_error_msg db 'Disk error!', 0
a20_error_msg  db 'A20 error!', 0

; GDT
gdt_start:
    gdt_null:
        dd 0
        dd 0
    
    gdt_code:
        ; Base=0, Limit=4GB, Type=0x9A (Present, Ring 0, Code, Exec/Read)
        dw 0xFFFF       ; Limit (bits 0-15)
        dw 0x0000       ; Base (bits 0-15)
        db 0x00         ; Base (bits 16-23)
        db 0x9A         ; Access (Present, Ring 0, Code, Exec/Read)
        db 0xCF         ; Granularity (4KB, 32-bit) + Limit (bits 16-19)
        db 0x00         ; Base (bits 24-31)
    
    gdt_data:
        ; Base=0, Limit=4GB, Type=0x92 (Present, Ring 0, Data, Read/Write)
        dw 0xFFFF       ; Limit (bits 0-15)
        dw 0x0000       ; Base (bits 0-15)
        db 0x00         ; Base (bits 16-23)
        db 0x92         ; Access (Present, Ring 0, Data, Read/Write)
        db 0xCF         ; Granularity (4KB, 32-bit) + Limit (bits 16-19)
        db 0x00         ; Base (bits 24-31)
    
    gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size of GDT
    dd gdt_start                ; Start of GDT

; Boot signature
TIMES 510-($-$$) db 0
dw 0xAA55
