#include "../include/vm.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/string.h"

// Virtual memory test process
void vm_test_process(void) {
    char msg[] = "VM Test: Virtual memory test started!\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    // Test memory allocation
    uint32_t addr = syscall(SYS_VM_ALLOC, 0x1000, PAGE_WRITE, 0);
    if (addr != 0) {
        char alloc_msg[] = "VM Test: Memory allocated at 0x00000000\n";
        // Simple hex conversion
        for (int i = 7; i >= 0; i--) {
            char nibble = (addr >> (i * 4)) & 0xF;
            alloc_msg[28 + (7-i)] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
        }
        syscall(SYS_WRITE, 1, (uint32_t)alloc_msg, sizeof(alloc_msg) - 1);
        
        // Test memory mapping
        int result = syscall(SYS_VM_MAP, 0x20000000, addr, PAGE_WRITE | PAGE_USER);
        if (result == 0) {
            char map_msg[] = "VM Test: Memory mapping successful!\n";
            syscall(SYS_WRITE, 1, (uint32_t)map_msg, sizeof(map_msg) - 1);
        }
        
        // Test memory access (write to mapped address)
        char* test_ptr = (char*)0x20000000;
        *test_ptr = 'A';
        
        if (*test_ptr == 'A') {
            char access_msg[] = "VM Test: Memory access successful!\n";
            syscall(SYS_WRITE, 1, (uint32_t)access_msg, sizeof(access_msg) - 1);
        }
    }
    
    // Dump memory regions
    // vm_dump_regions(); // Commented out for now
    
    while (1) {
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 50000; i++);
    }
}
