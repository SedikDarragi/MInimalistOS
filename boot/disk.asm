; load 'dh' sectors from drive 'dl' into ES:BX
disk_load:
    pusha
    push dx
    
    mov ah, 0x02    ; BIOS read sector function
    mov al, dh      ; Read 'dh' sectors
    mov ch, 0x00    ; Cylinder 0
    mov dh, 0x00    ; Head 0
    mov cl, 0x02    ; Sector 2 (1-based)
    
    int 0x13        ; BIOS interrupt
    
    jc disk_error   ; Jump if error
    
    pop dx          ; Restore DX
    cmp al, dh      ; Check if all sectors were read
    jne disk_error
    
    popa
    ret
    
disk_error:
    mov si, disk_error_msg
    call print_str
    jmp $
    
disk_error_msg db "Disk read error!", 0
