[BITS 16]
[ORG 0x7C00]

start:
    ; Initialize segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Clear screen
    mov ax, 0x0003
    int 0x10
    
    ; Print '1'
    mov si, msg1
    call print
    
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Print '2'
    mov si, msg2
    call print
    
    ; Load kernel at 0x10000
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 16
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc error
    
    ; Print '3'
    mov si, msg3
    call print
    
    ; Print '4'
    mov si, msg4
    call print
    
    ; Print '5' (last BIOS call before protected mode)
    mov si, msg5
    call print
    
    ; Try to enable A20 line using different methods
    call enable_a20
    jc error_a20
    
    ; Print 'A' if A20 enabled
    mov si, msg_a
    call print
    
    ; Load GDT
    cli
    lgdt [gdt_desc]
    
    ; Print 'B' if GDT loaded
    mov si, msg_b
    call print
    
    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax
    
    ; Far jump to 32-bit code (this is where we switch to protected mode)
    jmp 0x08:pm_start
    
    ; Far jump to 32-bit code (use explicit 32-bit operand size)
    jmp dword 0x08:pm_start

error:
    mov si, msg_err
    call print
    jmp $
    
error_a20:
    mov si, msg_a20_err
    call print
    jmp $
    
; Function to enable A20 line
; Returns with carry set on error
enable_a20:
    ; Try BIOS method first
    mov ax, 0x2401
    int 0x15
    jnc .success
    
    ; Try keyboard controller method
    cli
    call .wait_kbd
    mov al, 0xAD    ; Disable keyboard
    out 0x64, al
    
    call .wait_kbd
    mov al, 0xD0    ; Read output port
    out 0x64, al
    
    call .wait_kbd2
    in al, 0x60
    push eax
    
    call .wait_kbd
    mov al, 0xD1    ; Write output port
    out 0x64, al
    
    call .wait_kbd
    pop eax
    or al, 2        ; Set A20 bit
    out 0x60, al
    
    call .wait_kbd
    mov al, 0xAE    ; Enable keyboard
    out 0x64, al
    
    sti
    
    ; Try fast A20 method
    in al, 0x92
    test al, 2
    jnz .success
    or al, 2
    out 0x92, al
    
    ; Verify A20 is enabled
    call .test_a20
    jc .error
    
.success:
    clc
    ret
    
.error:
    stc
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
    
.test_a20:
    pushad
    
    ; Save old values
    mov edi, 0x112345  ; Odd megabyte address
    mov esi, 0x012345  ; Even megabyte address
    mov [es:edi], byte 0x00
    mov [ds:esi], byte 0xFF
    cmpsb
    
    ; Restore values and set carry if A20 is not enabled
    mov [es:edi], byte 0x00
    mov [ds:esi], byte 0x00
    stc
    jne .test_done
    clc
    
.test_done:
    popad
    ret

print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print
.done:
    ret

boot_drive: db 0
msg1: db '1', 0
msg2: db '2', 0
msg3: db '3', 0
msg4: db '4', 0
msg5: db '5', 0
msg_a:  db 'A', 0
msg_b:  db 'B', 0
msg_c:  db 'C', 0
msg_err: db 'E', 0
msg_a20_err: db 'F', 0

; GDT
gdt_start:
    dq 0                ; Null descriptor
    
    ; Code segment
    dw 0xFFFF           ; Limit low
    dw 0x0000           ; Base low
    db 0x00             ; Base middle
    db 10011010b        ; Access: present, ring 0, code, executable, readable
    db 11001111b        ; Flags: 4K granularity, 32-bit
    db 0x00             ; Base high
    
    ; Data segment
    dw 0xFFFF           ; Limit low
    dw 0x0000           ; Base low
    db 0x00             ; Base middle
    db 10010010b        ; Access: present, ring 0, data, writable
    db 11001111b        ; Flags: 4K granularity, 32-bit
    db 0x00             ; Base high
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
    
    ; Print 'D' to VGA (first protected mode code)
    mov byte [0xB8000], 'D'
    mov byte [0xB8001], 0x0F
    
    ; Print '6' to VGA
    mov byte [0xB8002], '6'
    mov byte [0xB8003], 0x0F
    
    ; Copy kernel to 1MB
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 2048
    rep movsd
    
    ; Print '8'
    mov byte [0xB8002], '8'
    mov byte [0xB8003], 0x0F
    
    ; Jump to kernel
    jmp 0x100000
    
    ; Hang if we return
    cli
    hlt

times 510-($-$$) db 0
dw 0xAA55
