#include "../include/filesystem.h"
#include "../include/vga.h"
#include "../include/string.h"

// Global file system instance
static filesystem_t fs;

// Initialize file system
void fs_init(void) {
    // Clear file system
    memset(&fs, 0, sizeof(filesystem_t));
    
    // Mark first few blocks as used for file system metadata
    fs.bitmap[0] = 0x0000000F;  // First 4 blocks used
    
    // Create root directory
    fs_create("/", FILE_TYPE_DIR);
    
    // Create some test files
    int fd = fs_create("test.txt", FILE_TYPE_REGULAR);
    if (fd >= 0) {
        const char* test_data = "Hello, File System!\nThis is a test file.\n";
        fs_write(fd, test_data, strlen(test_data));
        fs_close(fd);
    }
    
    fd = fs_create("welcome.txt", FILE_TYPE_REGULAR);
    if (fd >= 0) {
        const char* welcome = "Welcome to the OS File System!\n";
        fs_write(fd, welcome, strlen(welcome));
        fs_close(fd);
    }
}

// Allocate a free block
uint32_t alloc_block(void) {
    for (uint32_t i = 0; i < FS_SIZE / BLOCK_SIZE / 32; i++) {
        if (fs.bitmap[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(fs.bitmap[i] & (1 << j))) {
                    fs.bitmap[i] |= (1 << j);
                    return i * 32 + j;
                }
            }
        }
    }
    return 0;  // No free blocks
}

// Free a block
void free_block(uint32_t block) {
    uint32_t bitmap_index = block / 32;
    uint32_t bit_index = block % 32;
    
    if (bitmap_index < FS_SIZE / BLOCK_SIZE / 32) {
        fs.bitmap[bitmap_index] &= ~(1 << bit_index);
    }
}

// Find file by name
file_entry_t* find_file(const char* filename) {
    for (uint32_t i = 0; i < fs.file_count; i++) {
        if (strcmp(fs.files[i].filename, filename) == 0) {
            return &fs.files[i];
        }
    }
    return NULL;
}

// Create a new file
int fs_create(const char* filename, uint8_t type) {
    if (fs.file_count >= MAX_FILES) {
        return -1;  // Too many files
    }
    
    if (find_file(filename) != NULL) {
        return -1;  // File already exists
    }
    
    // Find free file entry
    file_entry_t* file = &fs.files[fs.file_count];
    
    // Initialize file entry
    strncpy(file->filename, filename, MAX_FILENAME_LEN - 1);
    file->filename[MAX_FILENAME_LEN - 1] = '\0';
    file->size = 0;
    file->type = type;
    file->flags = 0;
    file->creation_time = 0;  // Simple - no real time
    file->modify_time = 0;
    
    // Allocate first block
    file->start_block = alloc_block();
    if (file->start_block == 0) {
        return -1;  // Out of space
    }
    
    fs.file_count++;
    return fs.file_count - 1;  // Return file index
}

// Delete a file
int fs_delete(const char* filename) {
    file_entry_t* file = find_file(filename);
    if (!file) {
        return -1;  // File not found
    }
    
    // Free all blocks used by file
    uint32_t blocks = (file->size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    for (uint32_t i = 0; i < blocks; i++) {
        free_block(file->start_block + i);
    }
    
    // Remove file entry (shift remaining files)
    uint32_t index = file - fs.files;
    for (uint32_t i = index; i < fs.file_count - 1; i++) {
        fs.files[i] = fs.files[i + 1];
    }
    
    fs.file_count--;
    return 0;
}

// Open a file
int fs_open(const char* filename, uint32_t mode) {
    file_entry_t* file = find_file(filename);
    if (!file) {
        if (mode & FILE_CREATE) {
            int fd = fs_create(filename, FILE_TYPE_REGULAR);
            if (fd < 0) return -1;
            file = &fs.files[fd];
        } else {
            return -1;  // File not found
        }
    }
    
    // For simplicity, return file index as fd
    // In a real OS, we'd have a separate open file table
    return (file - fs.files);
}

// Close a file
int fs_close(int fd) {
    (void)fd; // Suppress unused parameter warning
    // For simplicity, do nothing
    // In a real OS, we'd clean up open file state
    return 0;
}

// Read from a file
int fs_read(int fd, void* buffer, uint32_t count) {
    if (fd >= (int)fs.file_count) {
        return -1;  // Invalid fd
    }
    
    file_entry_t* file = &fs.files[fd];
    uint32_t bytes_to_read = count;
    
    if (file->size < bytes_to_read) {
        bytes_to_read = file->size;
    }
    
    // Copy data from file system
    uint8_t* file_data = &fs.data[file->start_block * BLOCK_SIZE];
    memcpy(buffer, file_data, bytes_to_read);
    
    return bytes_to_read;
}

// Write to a file
int fs_write(int fd, const void* buffer, uint32_t count) {
    if (fd >= (int)fs.file_count) {
        return -1;  // Invalid fd
    }
    
    file_entry_t* file = &fs.files[fd];
    
    // Check if we need more blocks
    uint32_t needed_blocks = (file->size + count + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint32_t current_blocks = (file->size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    if (needed_blocks > current_blocks) {
        // Allocate more blocks (simplified - just allocate sequentially)
        for (uint32_t i = current_blocks; i < needed_blocks; i++) {
            uint32_t new_block = alloc_block();
            if (new_block == 0) {
                return -1;  // Out of space
            }
        }
    }
    
    // Copy data to file system
    uint8_t* file_data = &fs.data[file->start_block * BLOCK_SIZE];
    memcpy(file_data + file->size, buffer, count);
    
    file->size += count;
    file->modify_time = 0;
    
    return count;
}

// Seek to position in file
int fs_seek(int fd, uint32_t position) {
    if (fd >= (int)fs.file_count) {
        return -1;  // Invalid fd
    }
    
    file_entry_t* file = &fs.files[fd];
    if (position > file->size) {
        return -1;  // Invalid position
    }
    
    // For simplicity, we don't track position per open file
    // In a real OS, we'd have a separate open file table
    return position;
}

// List directory contents
int fs_list(const char* dirname) {
    (void)dirname; // Suppress unused parameter warning
    
    // Simple directory listing
    for (uint32_t i = 0; i < fs.file_count; i++) {
        file_entry_t* file = &fs.files[i];
        if (file->type == FILE_TYPE_REGULAR) {
            // Output file info would go here
            // In a real implementation, this would use sys_write
        }
    }
    
    return 0;
}

// Get file size
uint32_t get_file_size(const char* filename) {
    file_entry_t* file = find_file(filename);
    return file ? file->size : 0;
}
