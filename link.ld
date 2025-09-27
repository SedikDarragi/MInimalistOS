ENTRY(_start)

SECTIONS
{
    /* Load the kernel at 0x10000 (64KB) */
    . = 0x10000;
    
    /* Kernel code section */
    .text : AT(0x10000) {
        *(.text)
        *(.rodata*)
    }
    
    /* Read-write data (initialized) */
    .data : {
        *(.data)
    }
    
    /* Read-write data (uninitialized) and stack */
    .bss : {
        *(COMMON)
        *(.bss)
    }
    
    /* End of kernel */
    _end = .;
    
    /* Remove unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
