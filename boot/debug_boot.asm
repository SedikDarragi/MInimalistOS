[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x1000  ; Where we'll load our kernel

; Entry point
_start:
    jmp main

; Print a null-terminated string in DS:SI
print_str:
    pusha
    mov ah, 0x0E
.print_loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .print_loop
.done:
    popa
    ret

; Print a single character in AL
print_char:
    pusha
    mov ah, 0x0E
    int 0x10
    popa
    ret

; Print a newline
print_nl:
    pusha
    mov al, 0x0D
    call print_char
    mov al, 0x0A
    call print_char
    popa
    ret

; Load 'dh' sectors from drive 'dl' into ES:BX
disk_load:
    pusha
    push dx
    
    mov ah, 0x02    ; BIOS read sector function
    mov al, dh      ; Read 'dh' sectors
    mov ch, 0x00    ; Cylinder 0
    mov dh, 0x00    ; Head 0
    mov cl, 0x02    ; Sector 2 (1-based)
    
    int 0x13        ; BIOS interrupt
    
    jc .error       ; Jump if error
    
    pop dx          ; Restore DX
    cmp al, dh      ; Check if all sectors were read
    jne .error
    
    popa
    ret
    
.error:
    mov si, disk_error_msg
    call print_str
    mov al, ah      ; Error code
    call print_hex
    jmp $

; Print AL in hex
print_hex:
    pusha
    mov cx, 4       ; 4 hex digits (16 bits)
    mov bx, HEX_OUT + 2
.next_digit:
    rol ax, 4
    mov dx, ax
    and dx, 0x000F
    cmp dl, 9
    jg .letter
    add dl, '0'
    jmp .print
.letter:
    add dl, 'A' - 10
.print:
    mov [bx], dl
    inc bx
    loop .next_digit
    mov si, HEX_OUT
    call print_str
    popa
    ret

; GDT
gdt_start:
    dq 0x0                 ; Null descriptor
    
    ; Code segment
    dw 0xFFFF             ; Limit (bits 0-15)
    dw 0x0                ; Base (bits 0-15)
    db 0x0                ; Base (bits 16-23)
    db 10011010b          ; 1st flags, type flags
    db 11001111b          ; 2nd flags, Limit (bits 16-19)
    db 0x0                ; Base (bits 24-31)
    
    ; Data segment
    dw 0xFFFF             ; Limit (bits 0-15)
    dw 0x0                ; Base (bits 0-15)
    db 0x0                ; Base (bits 16-23)
    db 10010010b          ; 1st flags, type flags
    db 11001111b          ; 2nd flags, Limit (bits 16-19)
    db 0x0                ; Base (bits 24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Constants for GDT
CODE_SEG equ 8
DATA_SEG equ 16

; Switch to protected mode
switch_to_pm:
    cli                     ; Disable interrupts
    
    ; Load the GDT descriptor
    lgdt [gdt_descriptor]
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to flush the pipeline and reload CS
    jmp CODE_SEG:init_pm

[bits 32]
; Initialize registers and stack in protected mode
init_pm:
    ; Update segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Update stack pointer
    mov ebp, 0x90000
    mov esp, ebp
    
    ; Print a message
    mov ebx, msg_pm
    call print_string_pm
    
    ; Jump to the kernel
    jmp CODE_SEG:KERNEL_OFFSET
    
    ; Halt if we return (shouldn't happen)
    jmp $

; Print a null-terminated string in protected mode
; Input: EBX = address of the string
print_string_pm:
    pusha
    mov edx, 0xb8000  ; Start of video memory
    
    ; Set text color (white on black)
    mov ah, 0x0f
    
.pm_loop:
    mov al, [ebx]     ; Get current character
    cmp al, 0         ; Check for null terminator
    je .pm_done
    
    mov [edx], ax     ; Write character and attributes
    add ebx, 1        ; Next character
    add edx, 2        ; Next video memory position
    jmp .pm_loop
    
.pm_done:
    popa
    ret

; Main bootloader code
[bits 16]
main:
    ; Set up segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    ; Save boot drive
    mov [boot_drive], dl

    ; Set video mode
    mov ax, 0x0003  ; 80x25 text mode
    int 0x10

    ; Print loading message
    mov si, msg_loading
    call print_str

    ; Load kernel from disk
    mov bx, KERNEL_OFFSET
    mov es, bx
    xor bx, bx
    mov dh, 15  ; Number of sectors to read
    mov dl, [boot_drive]
    
    ; Print debug info
    mov si, msg_loading_kernel
    call print_str
    call disk_load
    
    ; If we get here, disk load was successful
    mov si, msg_ok
    call print_str
    call print_nl

    ; Switch to protected mode
    mov si, msg_switching
    call print_str
    call switch_to_pm

    ; Halt if we return (shouldn't happen)
    mov si, msg_halted
    call print_str
    jmp $

; Data
msg_loading       db 'Bootloader started', 0x0D, 0x0A, 0
msg_loading_kernel db 'Loading kernel... ', 0
msg_switching     db 'Switching to protected mode...', 0x0D, 0x0A, 0
msg_ok            db 'OK', 0
msg_pm            db 'Entered protected mode!', 0
msg_halted        db 'System halted.', 0
disk_error_msg    db 'Disk error: 0x', 0
boot_drive        db 0
HEX_OUT:          db '0x0000', 0

; Pad to 510 bytes and add boot signature
times 510 - ($ - $$) db 0
dw 0xAA55
