[BITS 16]
[ORG 0x7C00]

start:
    ; Initialize segments and stack
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
    
    ; Reset disk system first
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    ; Read disk sectors
    mov ah, 0x02    ; Read sectors
    mov al, 8        ; Number of sectors to read
    mov ch, 0        ; Cylinder 0
    mov cl, 2        ; Sector 2 (1-based)
    mov dh, 0        ; Head 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error    ; Jump if error (carry flag set)
    
    ; Verify number of sectors read
    cmp al, 8
    jl disk_error
    
    ; Print '3'
    mov si, msg3
    call print
    
    ; Print 'A' before A20
    mov si, msg_a
    call print
    
    ; Enable A20 line (fast method)
    in al, 0x92
    test al, 2
    jnz .a20_done
    or al, 2
    out 0x92, al
.a20_done:
    
    ; Print '4' if A20 enabled
    mov si, msg4
    call print
    
    ; Print 'B' before GDT
    mov si, msg_b
    call print
    
    ; Load GDT and enter protected mode
    cli
    lgdt [gdt_desc]
    
    ; Print 'C' before PM
    mov si, msg_c
    call print
    
    ; Enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax
    
    ; Print 'D' after PM enable (shouldn't be reached if PM works)
    mov si, msg_d
    call print
    
    ; Far jump to 32-bit code (use explicit 32-bit operand size)
    jmp dword 0x08:pm_start

disk_error:
    mov si, disk_err_msg
    call print
    jmp $

error:
    mov si, msg_err
    call print
    jmp $

print:
    pusha
    mov ah, 0x0E
.print_loop:
    lodsb
    or al, al
    jz .done
    int 0x10
    jmp .print_loop
.done:
    popa
    ret

boot_drive: db 0
msg1: db '1', 0
msg2: db '2', 0
msg3: db '3', 0
msg4: db '4', 0
msg5: db '5', 0
msg_a: db 'A', 0
msg_b: db 'B', 0
msg_c: db 'C', 0
msg_d: db 'D', 0
msg_err: db 'E', 0
disk_err_msg: db 'Disk error!', 0

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
    ; First instruction in PM - print 'E'
    mov byte [0xB8000], 'E'
    mov byte [0xB8001], 0x0F
    
    ; Setup segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Print 'F' after segment setup
    mov byte [0xB8002], 'F'
    mov byte [0xB8003], 0x0F
    
    ; Print '6' to VGA (first protected mode code)
    mov byte [0xB8000], '6'
    mov byte [0xB8001], 0x0F
    
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
