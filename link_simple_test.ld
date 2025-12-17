/* Entry point of the simple test kernel */
ENTRY(kmain_tests)

/* Define the program headers to ensure the bootloader loads us where we want */
PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
    bss     PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

SECTIONS
{
    /* The kernel will be loaded at 0x1000 (4KB) to match the bootloader */
    . = 0x1000;

    /* Kernel code section */
    .text : {
        *(.text .text.*)
    } :text
    
    /* Read-only data */
    .rodata : {
        *(.rodata .rodata.*)
    } :rodata
    
    /* Read-write data (initialized) */
    .data : {
        *(.data .data.*)
    } :data
    
    /* Read-write data (uninitialized) */
    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    } :bss
    
    /* End of kernel */
    _end = .;
    
    /* Remove unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
