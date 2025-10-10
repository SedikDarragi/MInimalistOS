/* Entry point of the kernel */
ENTRY(kmain)

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
    /* The kernel will be loaded at 1MB (0x100000) */
    . = 0x100000;
    
    /* Kernel code section */
    .text : ALIGN(4K) {
        *(.text .text.*)
    } :text
    
    /* Read-only data */
    .rodata : ALIGN(4K) {
        *(.rodata .rodata.*)
    } :rodata
    
    /* Read-write data (initialized) */
    .data : ALIGN(4K) {
        *(.data .data.*)
    } :data
    
    /* Read-write data (uninitialized) */
    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss .bss.*)
    } :bss
    
    /* End of kernel */
    _end = .;
    
    /* Remove unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
        *(.note.gnu.property)
    }
}
