/* Entry point of the minimal kernel */
ENTRY(_start)

SECTIONS
{
    /* The kernel will be loaded at 0x8000 (32KB) to match the bootloader */
    . = 0x8000;

    /* Kernel code section */
    .text : {
        *(.text) /* Entry code is first */
        *(.text.*)
    }
    
    /* Remove unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
        *(.note.gnu.property)
    }
}