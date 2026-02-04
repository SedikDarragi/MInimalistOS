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
    
    jc .disk_error  ; Jump if error
    
    pop dx          ; Restore DX
    cmp al, dh      ; Check if all sectors were read
    jne .disk_error
    
    popa
    ret
    
.disk_error:
    mov si, disk_error_msg
    call print_str
    jmp $

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
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

gdt_code equ 8
gdt_data equ 16

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
    
    ; Jump to protected mode code
    jmp BEGIN_PM

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
    mov dh, 15  ; Number of sectors to read
    mov dl, [boot_drive]
    call disk_load

    ; Print success message
    mov si, msg_loaded
    call print_str

    ; Switch to protected mode
    call switch_to_pm

    ; Halt if we return (shouldn't happen)
    jmp $

; Data
msg_loading db 'Loading kernel... ', 0
msg_loaded  db 'OK', 0
msg_pm      db 'Entered protected mode!', 0
disk_error_msg db 'Disk read error!', 0
boot_drive  db 0

; Pad to 510 bytes and add boot signature
times 510 - ($ - $$) db 0
dw 0xAA55
