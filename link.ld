ENTRY(_start)

SECTIONS
{
    /* Load the kernel at 1MB mark (0x100000) */
    . = 0x100000;
    
    /* Kernel code section */
    .text ALIGN(4K) : AT(ADDR(.text) - 0x100000) {
        *(.multiboot)
        *(.text)
    }
    
    /* Read-only data */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0x100000) {
        *(.rodata)
    }
    
    /* Read-write data (initialized) */
    .data ALIGN(4K) : AT(ADDR(.data) - 0x100000) {
        *(.data)
    }
    
    /* Read-write data (uninitialized) and stack */
    .bss ALIGN(4K) : AT(ADDR(.bss) - 0x100000) {
        *(COMMON)
        *(.bss)
        *(.stack)
    }
    
    /* Remove unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
