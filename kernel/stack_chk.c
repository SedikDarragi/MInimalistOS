/* Stack protection implementation for kernel */

void __stack_chk_fail_local(void) {
    /* Stack protection failure - halt the system */
    while (1) {
        __asm__ volatile("cli; hlt");
    }
}

void __stack_chk_fail(void) {
    /* Stack protection failure - halt the system */
    while (1) {
        __asm__ volatile("cli; hlt");
    }
}
