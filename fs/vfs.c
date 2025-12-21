#include "../include/filesystem.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../drivers/vga.h"
#include "../kernel/log.h"

// Simple Virtual File System (VFS) implementation

// Maximum number of open files
#define MAX_OPEN_FILES 32
// Maximum number of mounted filesystems
#define MAX_FILESYSTEMS 8
// Maximum path length
#define MAX_PATH_LEN 256
// Maximum filename length
#define MAX_FILENAME_LEN 64

// File descriptor structure
typedef struct {
    uint32_t fs_id;        // Filesystem ID
    uint32_t inode;        // Inode number
    uint32_t offset;       // Current offset
    uint32_t flags;        // File flags (read/write)
    uint8_t in_use;       // Is this descriptor in use?
} file_descriptor_t;

// Filesystem structure
typedef struct {
    uint8_t in_use;               // Is this filesystem mounted?
    char mount_point[MAX_PATH_LEN]; // Mount point path
    void* fs_data;               // Filesystem-specific data
    // Filesystem operations
    int (*read)(uint32_t inode, void* buffer, uint32_t count, uint32_t offset);
    int (*write)(uint32_t inode, const void* buffer, uint32_t count, uint32_t offset);
    int (*open)(const char* path, int flags);
    int (*close)(uint32_t inode);
    int (*stat)(const char* path, vfs_stat_t* stat);
} filesystem_t;

// Global VFS state
static file_descriptor_t file_descriptors[MAX_OPEN_FILES];
static filesystem_t filesystems[MAX_FILESYSTEMS];
static uint32_t next_fs_id = 1;

// Initialize VFS
void vfs_init(void) {
    // Initialize file descriptors
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        file_descriptors[i].in_use = 0;
        file_descriptors[i].fs_id = 0;
        file_descriptors[i].inode = 0;
        file_descriptors[i].offset = 0;
        file_descriptors[i].flags = 0;
    }
    
    // Initialize filesystems
    for (int i = 0; i < MAX_FILESYSTEMS; i++) {
        filesystems[i].in_use = 0;
        filesystems[i].mount_point[0] = '\0';
        filesystems[i].fs_data = NULL;
        filesystems[i].read = NULL;
        filesystems[i].write = NULL;
        filesystems[i].open = NULL;
        filesystems[i].close = NULL;
        filesystems[i].stat = NULL;
    }
    
    log_info("Virtual File System initialized");
    vga_print("VFS: OK\n");
}

// Mount a filesystem
int vfs_mount(const char* mount_point, void* fs_data,
             int (*read_func)(uint32_t, void*, uint32_t, uint32_t),
             int (*write_func)(uint32_t, const void*, uint32_t, uint32_t),
             int (*open_func)(const char*, int),
             int (*close_func)(uint32_t),
             int (*stat_func)(const char*, vfs_stat_t*)) {
    
    // Find free filesystem slot
    int slot = -1;
    for (int i = 0; i < MAX_FILESYSTEMS; i++) {
        if (!filesystems[i].in_use) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        log_error("No free filesystem slots");
        return -1;
    }
    
    // Initialize filesystem
    filesystems[slot].in_use = 1;
    filesystems[slot].fs_id = next_fs_id++;
    strncpy(filesystems[slot].mount_point, mount_point, MAX_PATH_LEN - 1);
    filesystems[slot].mount_point[MAX_PATH_LEN - 1] = '\0';
    filesystems[slot].fs_data = fs_data;
    filesystems[slot].read = read_func;
    filesystems[slot].write = write_func;
    filesystems[slot].open = open_func;
    filesystems[slot].close = close_func;
    filesystems[slot].stat = stat_func;
    
    log_info("Filesystem mounted at %s", mount_point);
    return filesystems[slot].fs_id;
}

// Find filesystem for a given path
static filesystem_t* find_filesystem(const char* path, char* relative_path) {
    filesystem_t* best_fs = NULL;
    size_t best_len = 0;
    
    for (int i = 0; i < MAX_FILESYSTEMS; i++) {
        if (filesystems[i].in_use) {
            size_t mount_len = strlen(filesystems[i].mount_point);
            
            // Check if path starts with mount point
            if (strncmp(path, filesystems[i].mount_point, mount_len) == 0) {
                // Check if this is the longest match
                if (mount_len > best_len) {
                    best_len = mount_len;
                    best_fs = &filesystems[i];
                }
            }
        }
    }
    
    if (best_fs && relative_path) {
        // Calculate relative path (skip mount point)
        if (best_len > 0) {
            strcpy(relative_path, path + best_len);
            // Remove leading slash if present
            if (relative_path[0] == '/') {
                strcpy(relative_path, relative_path + 1);
            }
        } else {
            strcpy(relative_path, path);
        }
    }
    
    return best_fs;
}

// Open a file
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
    
    // Find filesystem for this path
    char relative_path[MAX_PATH_LEN];
    filesystem_t* fs = find_filesystem(path, relative_path);
    
    if (!fs) {
        log_error("No filesystem found for path: %s", path);
        return -1;
    }
    
    // Call filesystem-specific open
    int inode = fs->open(relative_path, flags);
    if (inode < 0) {
        log_error("Failed to open file: %s", path);
        return -1;
    }
    
    // Initialize file descriptor
    file_descriptors[fd].in_use = 1;
    file_descriptors[fd].fs_id = fs->fs_id;
    file_descriptors[fd].inode = inode;
    file_descriptors[fd].offset = 0;
    file_descriptors[fd].flags = flags;
    
    log_info("File opened: %s (fd=%d)", path, fd);
    return fd;
}

// Close a file
int vfs_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_descriptors[fd].in_use) {
        log_error("Invalid file descriptor: %d", fd);
        return -1;
    }
    
    // Find filesystem
    filesystem_t* fs = NULL;
    for (int i = 0; i < MAX_FILESYSTEMS; i++) {
        if (filesystems[i].in_use && filesystems[i].fs_id == file_descriptors[fd].fs_id) {
            fs = &filesystems[i];
            break;
        }
    }
    
    if (!fs) {
        log_error("Filesystem not found for fd=%d", fd);
        return -1;
    }
    
    // Call filesystem-specific close
    fs->close(file_descriptors[fd].inode);
    
    // Mark descriptor as free
    file_descriptors[fd].in_use = 0;
    
    log_info("File closed (fd=%d)", fd);
    return 0;
}

// Read from a file
int vfs_read(int fd, void* buffer, uint32_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_descriptors[fd].in_use) {
        log_error("Invalid file descriptor: %d", fd);
        return -1;
    }
    
    // Find filesystem
    filesystem_t* fs = NULL;
    for (int i = 0; i < MAX_FILESYSTEMS; i++) {
        if (filesystems[i].in_use && filesystems[i].fs_id == file_descriptors[fd].fs_id) {
            fs = &filesystems[i];
            break;
        }
    }
    
    if (!fs || !fs->read) {
        log_error("Filesystem or read function not found for fd=%d", fd);
        return -1;
    }
    
    // Call filesystem-specific read
    int bytes_read = fs->read(file_descriptors[fd].inode, buffer, count, file_descriptors[fd].offset);
    
    if (bytes_read > 0) {
        file_descriptors[fd].offset += bytes_read;
    }
    
    return bytes_read;
}

// Write to a file
int vfs_write(int fd, const void* buffer, uint32_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_descriptors[fd].in_use) {
        log_error("Invalid file descriptor: %d", fd);
        return -1;
    }
    
    // Find filesystem
    filesystem_t* fs = NULL;
    for (int i = 0; i < MAX_FILESYSTEMS; i++) {
        if (filesystems[i].in_use && filesystems[i].fs_id == file_descriptors[fd].fs_id) {
            fs = &filesystems[i];
            break;
        }
    }
    
    if (!fs || !fs->write) {
        log_error("Filesystem or write function not found for fd=%d", fd);
        return -1;
    }
    
    // Call filesystem-specific write
    int bytes_written = fs->write(file_descriptors[fd].inode, buffer, count, file_descriptors[fd].offset);
    
    if (bytes_written > 0) {
        file_descriptors[fd].offset += bytes_written;
    }
    
    return bytes_written;
}

// Get file status
int vfs_stat(const char* path, vfs_stat_t* stat) {
    // Find filesystem for this path
    char relative_path[MAX_PATH_LEN];
    filesystem_t* fs = find_filesystem(path, relative_path);
    
    if (!fs || !fs->stat) {
        log_error("Filesystem or stat function not found for path: %s", path);
        return -1;
    }
    
    // Call filesystem-specific stat
    return fs->stat(relative_path, stat);
}

// List directory contents (simplified)
int vfs_list_dir(const char* path, char* buffer, uint32_t buffer_size) {
    // For now, just return a simple directory listing
    // In a real implementation, this would call the filesystem-specific list function
    strcpy(buffer, "root\n");
    strcat(buffer, "dev\n");
    strcat(buffer, "tmp\n");
    
    log_info("Directory listed: %s", path);
    return strlen(buffer);
}
