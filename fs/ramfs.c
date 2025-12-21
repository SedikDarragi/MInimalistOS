#include "../include/filesystem.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../drivers/vga.h"
#include "../kernel/log.h"

// Simple RAM-based filesystem

#define MAX_FILES 64
#define MAX_FILE_SIZE 4096
#define RAMFS_MAGIC 0x52414D46  // "RAMF"

// Ramfs file structure
typedef struct {
    char name[MAX_FILENAME_LEN];
    uint32_t size;
    uint32_t data;
    uint8_t in_use;
    uint32_t mode;  // File permissions
} ramfs_file_t;

// Ramfs filesystem data
typedef struct {
    uint32_t magic;
    ramfs_file_t files[MAX_FILES];
    uint8_t* file_data;
} ramfs_t;

static ramfs_t* ramfs = NULL;

// Initialize RAM filesystem
int ramfs_init(void) {
    // Allocate memory for filesystem structure
    ramfs = kmalloc(sizeof(ramfs_t));
    if (!ramfs) {
        log_error("Failed to allocate RAMFS structure");
        return -1;
    }
    
    // Allocate memory for file data
    ramfs->file_data = kmalloc(MAX_FILES * MAX_FILE_SIZE);
    if (!ramfs->file_data) {
        kfree(ramfs);
        log_error("Failed to allocate RAMFS data");
        return -1;
    }
    
    // Initialize filesystem
    ramfs->magic = RAMFS_MAGIC;
    
    for (int i = 0; i < MAX_FILES; i++) {
        ramfs->files[i].in_use = 0;
        ramfs->files[i].size = 0;
        ramfs->files[i].data = 0;
        ramfs->files[i].name[0] = '\0';
        ramfs->files[i].mode = 0644;  // Default permissions
    }
    
    // Create some default files
    ramfs_create_file("hello.txt", "Hello, World!\n", 14);
    ramfs_create_file("kernel.txt", "Minimalist OS Kernel v1.0\n", 26);
    ramfs_create_file("readme.txt", "This is a RAM-based filesystem.\n", 33);
    
    log_info("RAM filesystem initialized");
    return 0;
}

// Find file by name
static ramfs_file_t* ramfs_find_file(const char* name) {
    if (!ramfs || !name) return NULL;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (ramfs->files[i].in_use && 
            strcmp(ramfs->files[i].name, name) == 0) {
            return &ramfs->files[i];
        }
    }
    
    return NULL;
}

// Find free file slot
static ramfs_file_t* ramfs_find_free_slot(void) {
    if (!ramfs) return NULL;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (!ramfs->files[i].in_use) {
            return &ramfs->files[i];
        }
    }
    
    return NULL;
}

// Create a file
int ramfs_create_file(const char* name, const void* data, uint32_t size) {
    if (!ramfs || !name) return -1;
    
    // Check if file already exists
    if (ramfs_find_file(name)) {
        log_error("File already exists: %s", name);
        return -1;
    }
    
    // Find free slot
    ramfs_file_t* file = ramfs_find_free_slot();
    if (!file) {
        log_error("No free file slots");
        return -1;
    }
    
    // Find free data space
    uint32_t data_offset = 0;
    uint8_t found_space = 0;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (!ramfs->files[i].in_use) {
            data_offset = i * MAX_FILE_SIZE;
            found_space = 1;
            break;
        }
    }
    
    if (!found_space) {
        log_error("No free data space");
        return -1;
    }
    
    // Initialize file
    strncpy(file->name, name, MAX_FILENAME_LEN - 1);
    file->name[MAX_FILENAME_LEN - 1] = '\0';
    file->size = size;
    file->data = data_offset;
    file->in_use = 1;
    file->mode = 0644;
    
    // Copy data if provided
    if (data && size > 0) {
        memcpy(ramfs->file_data + data_offset, data, size);
    }
    
    log_info("File created: %s (%d bytes)", name, size);
    return 0;
}

// Open file (returns inode number)
int ramfs_open(const char* path, int flags) {
    if (!ramfs || !path) return -1;
    
    // Find file
    ramfs_file_t* file = ramfs_find_file(path);
    if (!file) {
        // Try to create file if write flag is set
        if (flags & O_CREAT) {
            if (ramfs_create_file(path, NULL, 0) == 0) {
                file = ramfs_find_file(path);
            }
        }
        
        if (!file) {
            log_error("File not found: %s", path);
            return -1;
        }
    }
    
    // Return inode number (file index + 1)
    return (file - ramfs->files) + 1;
}

// Close file
int ramfs_close(uint32_t inode) {
    if (!ramfs || inode == 0 || inode > MAX_FILES) return -1;
    
    // Nothing to do for RAMFS - files are always in memory
    log_debug("File closed (inode=%d)", inode);
    return 0;
}

// Read from file
int ramfs_read(uint32_t inode, void* buffer, uint32_t count, uint32_t offset) {
    if (!ramfs || !buffer || inode == 0 || inode > MAX_FILES) return -1;
    
    ramfs_file_t* file = &ramfs->files[inode - 1];
    if (!file->in_use) {
        log_error("Invalid inode: %d", inode);
        return -1;
    }
    
    // Check bounds
    if (offset >= file->size) {
        return 0;  // EOF
    }
    
    uint32_t bytes_to_read = count;
    if (offset + bytes_to_read > file->size) {
        bytes_to_read = file->size - offset;
    }
    
    // Copy data
    memcpy(buffer, ramfs->file_data + file->data + offset, bytes_to_read);
    
    log_debug("Read %d bytes from inode %d", bytes_to_read, inode);
    return bytes_to_read;
}

// Write to file
int ramfs_write(uint32_t inode, const void* buffer, uint32_t count, uint32_t offset) {
    if (!ramfs || !buffer || inode == 0 || inode > MAX_FILES) return -1;
    
    ramfs_file_t* file = &ramfs->files[inode - 1];
    if (!file->in_use) {
        log_error("Invalid inode: %d", inode);
        return -1;
    }
    
    // Check bounds
    if (offset >= MAX_FILE_SIZE) {
        log_error("File offset too large");
        return -1;
    }
    
    uint32_t bytes_to_write = count;
    if (offset + bytes_to_write > MAX_FILE_SIZE) {
        bytes_to_write = MAX_FILE_SIZE - offset;
    }
    
    // Copy data
    memcpy(ramfs->file_data + file->data + offset, buffer, bytes_to_write);
    
    // Update file size if necessary
    if (offset + bytes_to_write > file->size) {
        file->size = offset + bytes_to_write;
    }
    
    log_debug("Wrote %d bytes to inode %d", bytes_to_write, inode);
    return bytes_to_write;
}

// Get file status
int ramfs_stat(const char* path, vfs_stat_t* stat) {
    if (!ramfs || !path || !stat) return -1;
    
    ramfs_file_t* file = ramfs_find_file(path);
    if (!file) {
        log_error("File not found: %s", path);
        return -1;
    }
    
    // Fill stat structure
    stat->inode = (file - ramfs->files) + 1;
    stat->size = file->size;
    stat->mode = file->mode;
    stat->type = VFS_TYPE_FILE;
    
    return 0;
}

// Mount RAM filesystem
int ramfs_mount(void) {
    if (ramfs_init() != 0) {
        return -1;
    }
    
    // Mount at /ram
    return vfs_mount("/ram", ramfs, 
                   ramfs_read, ramfs_write, 
                   ramfs_open, ramfs_close, 
                   ramfs_stat);
}
