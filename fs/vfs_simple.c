#include "../include/filesystem.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../drivers/vga.h"
#include "../kernel/log.h"

// Simple Virtual File System (VFS) implementation

#define MAX_OPEN_FILES 16
#define MAX_PATH_LEN 256

// Simple file descriptor
typedef struct {
    uint32_t inode;
    uint32_t offset;
    uint8_t in_use;
} simple_fd_t;

static simple_fd_t file_descriptors[MAX_OPEN_FILES];

// Initialize VFS
void vfs_init(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        file_descriptors[i].in_use = 0;
        file_descriptors[i].inode = 0;
        file_descriptors[i].offset = 0;
    }
    
    log_info("Virtual File System initialized");
    vga_print("VFS: OK\n");
}

// Open a file (simplified)
int vfs_open(const char* path, int flags) {
    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!file_descriptors[i].in_use) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) {
        log_error("No free file descriptors");
        return -1;
    }
    
    // For now, just assign a simple inode based on path
    uint32_t inode = 1; // Simplified
    
    file_descriptors[fd].in_use = 1;
    file_descriptors[fd].inode = inode;
    file_descriptors[fd].offset = 0;
    
    log_info("File opened");
    return fd;
}

// Close a file
int vfs_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_descriptors[fd].in_use) {
        log_error("Invalid file descriptor");
        return -1;
    }
    
    file_descriptors[fd].in_use = 0;
    log_info("File closed");
    return 0;
}

// Read from a file (simplified)
int vfs_read(int fd, void* buffer, uint32_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_descriptors[fd].in_use) {
        log_error("Invalid file descriptor");
        return -1;
    }
    
    // For now, return some sample data
    const char* sample_data = "Hello from VFS!\n";
    uint32_t data_len = strlen(sample_data);
    
    if (file_descriptors[fd].offset >= data_len) {
        return 0; // EOF
    }
    
    uint32_t to_read = count;
    if (file_descriptors[fd].offset + to_read > data_len) {
        to_read = data_len - file_descriptors[fd].offset;
    }
    
    memcpy(buffer, sample_data + file_descriptors[fd].offset, to_read);
    file_descriptors[fd].offset += to_read;
    
    return to_read;
}

// Write to a file (simplified)
int vfs_write(int fd, const void* buffer, uint32_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_descriptors[fd].in_use) {
        log_error("Invalid file descriptor");
        return -1;
    }
    
    // For now, just pretend to write
    file_descriptors[fd].offset += count;
    log_info("File write completed");
    return count;
}

// Get file status (simplified)
int vfs_stat(const char* path, vfs_stat_t* stat) {
    if (!path || !stat) {
        return -1;
    }
    
    // Return simple stat info
    stat->inode = 1;
    stat->size = 16; // Simplified size
    stat->mode = 0644;
    stat->type = VFS_TYPE_FILE;
    
    return 0;
}

// Mount filesystem (simplified)
int vfs_mount(const char* mount_point, void* fs_data,
             int (*read_func)(uint32_t, void*, uint32_t, uint32_t),
             int (*write_func)(uint32_t, const void*, uint32_t, uint32_t),
             int (*open_func)(const char*, int),
             int (*close_func)(uint32_t),
             int (*stat_func)(const char*, vfs_stat_t*)) {
    
    log_info("Filesystem mounted");
    return 1; // Success
}

// List directory (simplified)
int vfs_list_dir(const char* path, char* buffer, uint32_t buffer_size) {
    if (!path || !buffer || buffer_size == 0) {
        return -1;
    }
    
    // Return simple directory listing
    const char* listing = "root\ntmp\n";
    uint32_t len = strlen(listing);
    
    if (len >= buffer_size) {
        len = buffer_size - 1;
    }
    
    memcpy(buffer, listing, len);
    buffer[len] = '\0';
    
    log_info("Directory listed");
    return len;
}
