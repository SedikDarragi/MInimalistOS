/* Minimal entry point for testing - GAS syntax version */

.set VGA_BUFFER, 0xB8000

.global _start

_start:
    /* Debug: Write to VGA to show we reached here */
    movb $'X', %al
    movb $0x0F, %ah
    movw %ax, (VGA_BUFFER)
    
    /* Write 'K' to show kernel is running */
    movb $'K', %al
    movb $0x0F, %ah
    movw %ax, (VGA_BUFFER + 6)
    
    /* Write '!' to show we're about to hang */
    movb $'!', %al
    movb $0x0F, %ah
    movw %ax, (VGA_BUFFER + 12)
    
    /* Just hang here */
    cli
.hang:
    hlt
    jmp .hang