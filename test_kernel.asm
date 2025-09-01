BITS 32

start:
    ; Set up VGA text mode
    mov edi, 0xb8000        ; VGA text buffer
    mov al, 'H'             ; Character to display
    mov ah, 0x0F            ; White on black
    mov [edi], ax
    
    mov al, 'E'
    mov [edi+2], ax
    
    mov al, 'L'
    mov [edi+4], ax
    
    mov al, 'L'
    mov [edi+6], ax
    
    mov al, 'O'
    mov [edi+8], ax
    
    mov al, '!'
    mov [edi+10], ax

    ; Infinite loop
    cli
    hlt
    jmp $

times 512-($-$$) db 0
