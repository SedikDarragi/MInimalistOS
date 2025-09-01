BITS 16
ORG 0x7c00

start:
    ; Set up segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    
    ; Print test message
    mov si, test_msg
    call print_string
    
    ; Infinite loop
    jmp $

print_string:
    mov ah, 0x0e
    lodsb
    cmp al, 0
    je done
    int 0x10
    jmp print_string
done:
    ret

test_msg db 'BOOTLOADER TEST - If you see this, bootloader works!', 13, 10, 0

times 510-($-$$) db 0
dw 0xaa55
