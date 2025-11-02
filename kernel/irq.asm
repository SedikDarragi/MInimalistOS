; IRQ handlers
[bits 32]

global irq0

extern default_irq_handler

%macro IRQ 1
global irq%1
irq%1:
    pusha
    push %1
    call default_irq_handler
    add esp, 4
    popa
    iret
%endmacro

; Define IRQ handlers
IRQ 0  ; Timer
IRQ 1  ; Keyboard
IRQ 2  ; Cascade
IRQ 3  ; COM2
IRQ 4  ; COM1
IRQ 5  ; LPT2
IRQ 6  ; Floppy
IRQ 7  ; LPT1
IRQ 8  ; CMOS RTC
IRQ 9  ; Free
IRQ 10 ; Free
IRQ 11 ; Free
IRQ 12 ; PS/2 Mouse
IRQ 13 ; FPU
IRQ 14 ; Primary ATA
IRQ 15 ; Secondary ATA
