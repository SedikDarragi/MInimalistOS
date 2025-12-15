#include "../include/usermode.h"
#include "../include/memory.h"
#include "../include/process.h"
#include "../include/idt.h"
#include "../include/vga.h"
#include "../include/string.h"

// TSS for task switching
static tss_t kernel_tss;

// Initialize user mode
void usermode_init(void) {
    // Set up TSS
    setup_tss(0x200000);  // Kernel stack at 2MB
    
    // For now, skip GDT setup and just set up TSS
    // In a real implementation, we'd set up user mode segments in GDT
}

// Set up TSS
void setup_tss(uint32_t kernel_stack) {
    memset(&kernel_tss, 0, sizeof(tss_t));
    kernel_tss.esp0 = kernel_stack;
    kernel_tss.ss0 = 0x10;  // Kernel data segment
    kernel_tss.cs = USER_CS;
    kernel_tss.ds = USER_DS;
    kernel_tss.es = USER_DS;
    kernel_tss.fs = USER_DS;
    kernel_tss.gs = USER_DS;
    kernel_tss.ss = USER_DS;
}

// Enter user mode
void enter_usermode(uint32_t entry_point, uint32_t stack_top) {
    asm volatile (
        "cli\n"
        "mov $0x20, %%ax\n"     // User data segment
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        
        "mov %0, %%esp\n"        // User stack
        "push $0x20\n"           // User data segment
        "push %%esp\n"           // User stack pointer
        "pushf\n"                // EFLAGS
        "pop %%eax\n"
        "or $0x200, %%eax\n"     // Enable interrupts
        "push %%eax\n"
        "push $0x18\n"           // User code segment
        "push %1\n"              // Entry point
        "iret\n"
        : : "r" (stack_top), "r" (entry_point)
        : "eax"
    );
}

// Create user process
uint32_t create_user_process(void (*entry_point)()) {
    (void)entry_point; // Suppress unused parameter warning
    
    // Allocate user memory space (4MB)
    uint32_t user_base = 0x40000000;  // 1GB virtual address
    
    // Allocate physical pages for user process
    for (uint32_t i = 0; i < 1024; i++) {  // 4MB = 1024 pages
        uint32_t phys_page = alloc_page();
        if (!phys_page) return 0;  // Out of memory
        
        map_page(user_base + i * PAGE_SIZE, phys_page, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    }
    
    // Copy entry point to user memory
    uint32_t user_entry = user_base;
    
    // Simple: just use the entry point directly for now
    // In a real OS, we'd load an ELF executable
    
    return user_entry;
}

// System call to enter user mode
void syscall_enter_usermode(void) {
    process_t* current = process_get_current();
    if (current && current->pid > 1) {  // Not kernel process
        // Set up user stack
        uint32_t user_stack = 0x40000000 + 4 * PAGE_SIZE - 16;  // Near end of first page
        
        // Enter user mode
        enter_usermode(0x40000000, user_stack);
    }
}
