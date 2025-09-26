BITS 16
ORG 0x7C00

start:
    ; Set up segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Set video mode to 80x25 text mode
    mov ax, 0x0003  ; 80x25 text mode
    int 0x10
    
    ; Print boot message
    mov si, boot_msg
    call print_string
    
    ; Print loading kernel message
    mov si, loading_kernel_msg
    call print_string
    
    ; Enable A20 line before loading kernel to 1MB
    call enable_a20
    
    ; Load kernel to 1MB (0x100000)
    ; We'll use ES:EDI for the target address
    mov ax, 0x1000
    mov es, ax
    xor bx, bx      ; ES:BX = 0x1000:0x0000 (0x10000 physical)
    
    ; Set up disk read parameters
    mov ah, 0x02    ; Read sectors function
    mov al, 64      ; Number of sectors to read (32KB)
    mov ch, 0       ; Cylinder 0
    mov cl, 2       ; Sector 2 (1-based)
    mov dh, 0       ; Head 0
    mov dl, 0x80    ; Drive 0x80 (first hard disk)
    
    ; Try reading from hard disk first
    int 0x13
    jnc .read_success
    
    ; If hard disk read failed, try floppy
    mov dl, 0x00    ; Drive 0x00 (first floppy)
    int 0x13
    jc disk_error   ; If both fail, show error
    
.read_success:
    ; Print success message
    mov si, kernel_loaded_msg
    call print_string
    
    ; Switch to protected mode
    cli
    lgdt [gdt_descriptor]
    
    ; Enable A20 line
    call enable_a20
    
    ; Set protected mode bit
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Far jump to clear pipeline and set CS
    jmp 0x08:protected_mode

; Function to enable A20 line
enable_a20:
    ; Try BIOS method first
    mov ax, 0x2401
    int 0x15
    ret
    
    ; If BIOS method fails, try keyboard controller method
    cli
    call .wait_kbd
    mov al, 0xAD    ; Disable keyboard
    out 0x64, al
    call .wait_kbd
    mov al, 0xD0    ; Read output port
    out 0x64, al
    call .wait_kbd2
    in al, 0x60     ; Read output port value
    push eax
    call .wait_kbd
    mov al, 0xD1    ; Write output port
    out 0x64, al
    call .wait_kbd
    pop eax
    or al, 2        ; Set A20 bit
    out 0x60, al    ; Write back to output port
    call .wait_kbd
    mov al, 0xAE    ; Enable keyboard
    out 0x64, al
    call .wait_kbd
    sti
    ret
    
.wait_kbd:
    in al, 0x64
    test al, 2
    jnz .wait_kbd
    ret
    
.wait_kbd2:
    in al, 0x64
    test al, 1
    jz .wait_kbd2
    ret

disk_error:
    mov si, disk_error_msg
    call print_string
    hlt

print_string:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

BITS 32
protected_mode:
    ; Set up protected mode segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Jump directly to kernel at 0x8000
    jmp 0x8000

; GDT
gdt_start:
    ; Null descriptor
    dd 0x0
    dd 0x0
    
    ; Code segment
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10011010b    ; Access byte
    db 11001111b    ; Granularity
    db 0x00         ; Base high
    
    ; Data segment
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10010010b    ; Access byte
    db 11001111b    ; Granularity
    db 0x00         ; Base high
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

boot_msg db 'MinimalOS Bootloader v1.0', 0x0D, 0x0A, 0
loading_kernel_msg db 'Loading kernel from disk... ', 0x0D, 0x0A, 0
kernel_loaded_msg db 'Kernel loaded successfully!', 0x0D, 0x0A, 'Switching to protected mode...', 0x0D, 0x0A, 0
disk_error_msg db 'Disk read error!', 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
