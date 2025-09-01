BITS 16
ORG 0x7C00

start:
    ; Set up segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Print boot message
    mov si, boot_msg
    call print_string
    
    ; Simple kernel load - load directly to 0x8000
    mov bx, 0x8000      ; Load kernel at 0x8000
    mov dh, 0           ; Head 0
    mov dl, 0           ; Drive 0 (floppy)
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Start from sector 2
    mov al, 36          ; Read 36 sectors (18KB kernel needs ~36 sectors)
    mov ah, 0x02        ; Read sectors function
    int 0x13            ; BIOS disk interrupt
    
    jc disk_error
    
    ; Print success message
    mov si, kernel_loaded_msg
    call print_string
    
    ; Switch to protected mode
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp 0x08:protected_mode

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

boot_msg db 'MinimalOS Bootloader v1.0', 13, 10, 'Loading kernel...', 13, 10, 0
kernel_loaded_msg db 'Kernel loaded successfully!', 13, 10, 'Switching to protected mode...', 13, 10, 0
disk_error_msg db 'Disk read error!', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55
