[BITS 16]
[ORG 0x7C00]

start:
    ; Minimal setup
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Clear screen and print '1'
    mov ax, 0x0003
    int 0x10
    mov al, '1'
    call print_char
    
    ; Print '2'
    mov al, '2'
    call print_char
    
    ; Print '3'
    mov al, '3'
    call print_char
    
    ; Try to enable A20 using the BIOS (most reliable method)
    mov ax, 0x2401
    int 0x15
    jnc .a20_enabled
    
    ; If BIOS method failed, try fast A20
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Test if A20 is enabled
    call test_a20
    jnz .a20_enabled
    
    ; If still not enabled, show error
    mov al, 'A'
    call print_char
    jmp $
    
.a20_enabled:
    ; Print '4' if A20 is enabled
    mov al, '4'
    call print_char
    
    ; Load GDT and enter protected mode
    cli
    lgdt [gdt_desc]
    
    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax
    
    ; Far jump to 32-bit code
    jmp 0x08:pm_start

; Test if A20 is enabled
; Returns: ZF=0 if enabled, ZF=1 if disabled
test_a20:
    pusha
    
    ; Use BIOS memory area for testing (0x0000:0x0500 and 0xFFFF:0x0510)
    xor ax, ax
    mov es, ax
    mov di, 0x0500
    
    ; Save original values
    mov al, [es:di]
    push ax
    
    ; Write test pattern
    mov byte [es:di], 0x00
    
    ; Try to read from 1MB higher address
    mov ax, 0xFFFF
    mov es, ax
    mov si, 0x0510
    mov al, [es:si]
    push ax
    mov byte [es:si], 0xFF
    
    ; Check if the values are different
    xor ax, ax
    mov es, ax
    cmp byte [es:di], 0xFF
    
    ; Restore original values
    pop ax
    mov bx, 0xFFFF
    mov es, bx
    mov [es:si], al
    pop ax
    xor bx, bx
    mov es, bx
    mov [es:di], al
    
    ; Set ZF based on test
    jne .a20_enabled
    xor ax, ax  ; ZF=1 (disabled)
    jmp .done
.a20_enabled:
    or ax, 1    ; ZF=0 (enabled)
.done:
    popa
    ret

disk_error:
    mov al, 'E'
    call print_char
    jmp $

print_char:
    mov ah, 0x0E
    int 0x10
    ret

boot_drive: db 0

; Minimal GDT
gdt_start:
    dq 0                ; Null descriptor
    
    ; Code segment (0x08)
    dw 0xFFFF           ; Limit 0-15
    dw 0x0000           ; Base 0-15
    db 0x00             ; Base 16-23
    db 0x9A             ; Access byte
    db 0xCF             ; Flags + Limit 16-19
    db 0x00             ; Base 24-31
    
    ; Data segment (0x10)
    dw 0xFFFF           ; Limit 0-15
    dw 0x0000           ; Base 0-15
    db 0x00             ; Base 16-23
    db 0x92             ; Access byte
    db 0xCF             ; Flags + Limit 16-19
    db 0x00             ; Base 24-31

gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
pm_start:
    ; Setup segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Print 'P' to show we're in protected mode
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0F
    
    ; Copy kernel from 0x10000 to 1MB (0x100000)
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 1024  ; 4KB
    rep movsd
    
    ; Print 'K' to show kernel copied
    mov byte [0xB8002], 'K'
    mov byte [0xB8003], 0x0F
    
    ; Jump to kernel
    jmp 0x100000
    
    ; Halt if we return
    cli
    hlt

times 510-($-$$) db 0
dw 0xAA55
