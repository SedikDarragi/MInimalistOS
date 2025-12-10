#include "../include/device.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/string.h"

// Device driver test process
void device_test_process(void) {
    char msg[] = "DEV Test: Device driver test started!\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    // Test block device
    char block_msg[] = "DEV Test: Testing block device...\n";
    syscall(SYS_WRITE, 1, (uint32_t)block_msg, sizeof(block_msg) - 1);
    
    // Open block device
    int result = syscall(SYS_DEVICE_OPEN, (uint32_t)"block0", 0, 0);
    if (result == 0) {
        char open_msg[] = "DEV Test: Block device opened successfully!\n";
        syscall(SYS_WRITE, 1, (uint32_t)open_msg, sizeof(open_msg) - 1);
        
        // Test writing to block device
        char test_data[] = "This is test data for block device!";
        int write_result = syscall(SYS_DEVICE_WRITE, (uint32_t)"block0", (uint32_t)test_data, strlen(test_data));
        if (write_result > 0) {
            char write_msg[] = "DEV Test: Block device write successful!\n";
            syscall(SYS_WRITE, 1, (uint32_t)write_msg, sizeof(write_msg) - 1);
        }
        
        // Test reading from block device
        char read_buffer[64];
        int read_result = syscall(SYS_DEVICE_READ, (uint32_t)"block0", (uint32_t)read_buffer, sizeof(read_buffer));
        if (read_result > 0) {
            char read_msg[] = "DEV Test: Block device read successful!\n";
            syscall(SYS_WRITE, 1, (uint32_t)read_msg, sizeof(read_msg) - 1);
        }
        
        // Close block device
        syscall(SYS_DEVICE_CLOSE, (uint32_t)"block0", 0, 0);
    }
    
    // Test character device
    char char_msg[] = "DEV Test: Testing character device...\n";
    syscall(SYS_WRITE, 1, (uint32_t)char_msg, sizeof(char_msg) - 1);
    
    // Open character device
    result = syscall(SYS_DEVICE_OPEN, (uint32_t)"char0", 0, 0);
    if (result == 0) {
        char open_msg[] = "DEV Test: Character device opened successfully!\n";
        syscall(SYS_WRITE, 1, (uint32_t)open_msg, sizeof(open_msg) - 1);
        
        // Test writing to character device
        char test_data[] = "Hello from character device!";
        int write_result = syscall(SYS_DEVICE_WRITE, (uint32_t)"char0", (uint32_t)test_data, strlen(test_data));
        if (write_result > 0) {
            char write_msg[] = "DEV Test: Character device write successful!\n";
            syscall(SYS_WRITE, 1, (uint32_t)write_msg, sizeof(write_msg) - 1);
        }
        
        // Test reading from character device
        char read_buffer[64];
        int read_result = syscall(SYS_DEVICE_READ, (uint32_t)"char0", (uint32_t)read_buffer, sizeof(read_buffer));
        if (read_result > 0) {
            char read_msg[] = "DEV Test: Character device read successful!\n";
            syscall(SYS_WRITE, 1, (uint32_t)read_msg, sizeof(read_msg) - 1);
        }
        
        // Close character device
        syscall(SYS_DEVICE_CLOSE, (uint32_t)"char0", 0, 0);
    }
    
    while (1) {
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 50000; i++);
    }
}
