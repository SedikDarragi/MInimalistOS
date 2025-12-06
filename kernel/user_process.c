#include "syscall.h"
#include "../include/vga.h"

// User process that uses system calls
void user_process_1(void) {
    int count = 0;
    char msg[] = "Process 1: Hello World!\n";
    
    while (1) {
        // Use system call to write message
        syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
        
        // Display count using system call
        char count_msg[] = "Count: 0\n";
        count_msg[7] = '0' + (count % 10);
        syscall(SYS_WRITE, 1, (uint32_t)count_msg, sizeof(count_msg) - 1);
        
        count++;
        
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 50000; i++);
    }
}

void user_process_2(void) {
    int count = 0;
    char msg[] = "Process 2: System Calls!\n";
    
    while (1) {
        // Use system call to write message
        syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
        
        // Get and display PID
        uint32_t pid = syscall(SYS_GETPID, 0, 0, 0);
        char pid_msg[] = "PID: 0\n";
        pid_msg[5] = '0' + (pid % 10);
        syscall(SYS_WRITE, 1, (uint32_t)pid_msg, sizeof(pid_msg) - 1);
        
        count++;
        
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 75000; i++);
    }
}
