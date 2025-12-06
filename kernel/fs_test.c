#include "../include/filesystem.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/string.h"

// File system test process
void fs_test_process(void) {
    char msg[] = "FS Test: Starting file system tests...\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    // Test file creation
    int fd = fs_create("fs_test.txt", FILE_TYPE_REGULAR);
    if (fd >= 0) {
        char create_msg[] = "FS Test: File created successfully!\n";
        syscall(SYS_WRITE, 1, (uint32_t)create_msg, sizeof(create_msg) - 1);
        
        // Test file writing
        const char* test_data = "File system test data\nLine 2\nLine 3\n";
        int bytes_written = fs_write(fd, test_data, strlen(test_data));
        
        if (bytes_written > 0) {
            char write_msg[] = "FS Test: Data written successfully!\n";
            syscall(SYS_WRITE, 1, (uint32_t)write_msg, sizeof(write_msg) - 1);
            
            // Close and reopen file
            fs_close(fd);
            
            // Test file reading
            fd = fs_open("fs_test.txt", FILE_READ);
            if (fd >= 0) {
                char read_buffer[100];
                int bytes_read = fs_read(fd, read_buffer, sizeof(read_buffer) - 1);
                
                if (bytes_read > 0) {
                    read_buffer[bytes_read] = '\0';
                    
                    char read_msg[] = "FS Test: Data read successfully!\n";
                    syscall(SYS_WRITE, 1, (uint32_t)read_msg, sizeof(read_msg) - 1);
                    
                    // Display some of the read data
                    syscall(SYS_WRITE, 1, (uint32_t)read_buffer, bytes_read);
                    
                    char newline[] = "\n";
                    syscall(SYS_WRITE, 1, (uint32_t)newline, 1);
                }
                
                fs_close(fd);
            }
            
            // Test file size
            uint32_t size = get_file_size("fs_test.txt");
            char size_msg[] = "FS Test: File size: 0 bytes\n";
            size_msg[17] = '0' + (size % 10);
            if (size >= 10) size_msg[16] = '0' + ((size / 10) % 10);
            syscall(SYS_WRITE, 1, (uint32_t)size_msg, sizeof(size_msg) - 1);
            
            // Test file deletion
            if (fs_delete("fs_test.txt") == 0) {
                char delete_msg[] = "FS Test: File deleted successfully!\n";
                syscall(SYS_WRITE, 1, (uint32_t)delete_msg, sizeof(delete_msg) - 1);
            }
        }
    }
    
    char done_msg[] = "FS Test: All file system tests completed!\n";
    syscall(SYS_WRITE, 1, (uint32_t)done_msg, sizeof(done_msg) - 1);
    
    while (1) {
        syscall(SYS_YIELD, 0, 0, 0);
    }
}
