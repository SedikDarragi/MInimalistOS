#include "../include/syscall.h"

// Simple user program that runs in ring 3
void user_program_main(void) {
    // Use the syscall function from user mode
    char msg[] = "User Mode: Hello from Ring 3!\n";
    
    // Write message using system call
    syscall(SYS_WRITE, 1, (uint32_t)msg, 27);
    
    // Count and display
    int count = 0;
    while (1) {
        char count_msg[] = "User Mode: Count 0\n";
        count_msg[16] = '0' + (count % 10);
        
        syscall(SYS_WRITE, 1, (uint32_t)count_msg, 20);
        
        count++;
        
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 100000; i++);
    }
}
