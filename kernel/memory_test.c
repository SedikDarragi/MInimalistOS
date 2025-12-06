#include "memory.h"
#include "syscall.h"
#include "../include/vga.h"

// Memory test process
void memory_test_process(void) {
    char msg[] = "Memory Test: Starting...\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    // Test heap allocation
    char* buffer1 = (char*)kmalloc(100);
    char* buffer2 = (char*)kmalloc(200);
    
    if (buffer1 && buffer2) {
        char success_msg[] = "Memory Test: Heap allocation successful!\n";
        syscall(SYS_WRITE, 1, (uint32_t)success_msg, sizeof(success_msg) - 1);
        
        // Write to allocated memory
        for (int i = 0; i < 99; i++) {
            buffer1[i] = 'A' + (i % 26);
        }
        buffer1[99] = '\0';
        
        syscall(SYS_WRITE, 1, (uint32_t)buffer1, 50);
        char newline[] = "\n";
        syscall(SYS_WRITE, 1, (uint32_t)newline, 1);
        
        // Test page allocation
        uint32_t page1 = alloc_page();
        uint32_t page2 = alloc_page();
        
        if (page1 && page2) {
            char page_msg[] = "Memory Test: Page allocation successful!\n";
            syscall(SYS_WRITE, 1, (uint32_t)page_msg, sizeof(page_msg) - 1);
            
            // Map pages to virtual memory
            map_page(0x10000000, page1, PAGE_PRESENT | PAGE_WRITE);
            map_page(0x10001000, page2, PAGE_PRESENT | PAGE_WRITE);
            
            // Write to mapped memory
            volatile uint32_t* mapped1 = (volatile uint32_t*)0x10000000;
            volatile uint32_t* mapped2 = (volatile uint32_t*)0x10001000;
            
            *mapped1 = 0xDEADBEEF;
            *mapped2 = 0xCAFEBABE;
            
            // Verify the values
            if (*mapped1 == 0xDEADBEEF && *mapped2 == 0xCAFEBABE) {
                char verify_msg[] = "Memory Test: Page mapping verification successful!\n";
                syscall(SYS_WRITE, 1, (uint32_t)verify_msg, sizeof(verify_msg) - 1);
            }
            
            // Clean up
            unmap_page(0x10000000);
            unmap_page(0x10001000);
            free_page(page1);
            free_page(page2);
        }
        
        char done_msg[] = "Memory Test: All tests completed!\n";
        syscall(SYS_WRITE, 1, (uint32_t)done_msg, sizeof(done_msg) - 1);
    }
    
    while (1) {
        syscall(SYS_YIELD, 0, 0, 0);
    }
}
