#include "../drivers/vga.h"
#include "../include/string.h"
#include "../kernel/log.h"
#include "../kernel/process.h"
#include "../drivers/timer.h"
#include "../drivers/serial.h"
#include "../include/filesystem.h"
#include "../include/syscall.h"
#include "../drivers/keyboard_intl.h"
#include "../include/program_loader.h"
#include "../include/net.h"
#include "../drivers/net_ne2k.h"
#include "../include/pci.h"
#include "../include/idt.h"
#include "../include/memory.h"
#include "../include/power.h"

extern void shell_init(void);
extern void shell_run(void);
extern void keyboard_init(void);
extern void fs_init(void);
extern int ramfs_mount(void);

void kmain(void) {
    log_init();
    log_info("Kernel started");

    idt_init();
    log_info("IDT initialized");
    memory_init();
    log_info("Memory initialized");
    paging_init();
    log_info("Paging initialized");
    
    vga_init();
    
    vga_print("Initializing serial port...\n");
    if (serial_init()) {
        vga_print("Serial port: OK\n");
        serial_info("Serial port initialized for debugging");
    } else {
        vga_print("Serial port: FAILED\n");
    }

    vga_print("Networking: SKIPPED\n");
    
    vga_print("Initializing process management...\n");
    process_init();
    vga_print("Process management: READY\n");
    
    vga_print("Initializing timer...\n");
    timer_init();
    vga_print("Timer: READY\n");
    
    vga_print("Initializing filesystem...\n");
    fs_init();
    ramfs_mount();
    vga_print("Filesystem: READY\n");
    
    vga_print("Initializing system call interface...\n");
    syscall_init();
    vga_print("Syscalls: READY\n");
    
    vga_print("Initializing keyboard...\n");
    keyboard_init();
    vga_print("Keyboard driver: READY\n");
    
    vga_print("Enabling IRQ 1...\n");
    enable_irq(1);
    
    // Perform final initializations
    power_init();
    program_loader_init();
    
    // Clear screen and initialize shell
    vga_clear();
    shell_init();

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("Minimalist OS v1.0 - Welcome\n");
    vga_print("----------------------------\n");

    // Globally enable interrupts so the keyboard driver works
    __asm__ volatile("sti");

    // Enter the shell loop - this function should not return
    shell_run();

    while (1) {
        __asm__ volatile("hlt");
    }
}
