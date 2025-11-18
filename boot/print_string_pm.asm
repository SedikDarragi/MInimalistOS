[bits 32]
; Print a null-terminated string in protected mode
; Input: EBX = address of the string
print_string_pm:
    pusha
    mov edx, 0xb8000  ; Start of video memory
    
    ; Set text color (white on black)
    mov ah, 0x0f
    
.print_loop:
    mov al, [ebx]     ; Get current character
    cmp al, 0         ; Check for null terminator
    je .done
    
    mov [edx], ax     ; Write character and attributes
    add ebx, 1        ; Next character
    add edx, 2        ; Next video memory position
    jmp .print_loop
    
.done:
    popa
    ret

; Data for protected mode
msg_pm db "Entered protected mode!", 0
