#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

// File system constants
#define MAX_FILENAME_LEN 256
#define MAX_FILES 64
#define BLOCK_SIZE 512
#define FS_SIZE (1024 * 1024)  // 1MB file system

// File modes
#define FILE_READ    0x01
#define FILE_WRITE   0x02
#define FILE_APPEND  0x04
#define FILE_CREATE  0x08

// File types
#define FILE_TYPE_REGULAR  1
#define FILE_TYPE_DIR      2

// File descriptor structure
typedef struct {
    char filename[MAX_FILENAME_LEN];
    uint32_t size;
    uint32_t start_block;
    uint8_t type;
    uint8_t flags;
    uint32_t creation_time;
    uint32_t modify_time;
} file_entry_t;

// Open file structure
typedef struct {
    file_entry_t* file;
    uint32_t position;
    uint32_t mode;
} open_file_t;

// File system structure
typedef struct {
    uint8_t data[FS_SIZE];
    uint32_t bitmap[FS_SIZE / BLOCK_SIZE / 32];  // Block allocation bitmap
    file_entry_t files[MAX_FILES];
    uint32_t file_count;
    uint32_t next_block;
} filesystem_t;

// File system functions
void fs_init(void);
int fs_create(const char* filename, uint8_t type);
int fs_delete(const char* filename);
int fs_open(const char* filename, uint32_t mode);
int fs_close(int fd);
int fs_read(int fd, void* buffer, uint32_t count);
int fs_write(int fd, const void* buffer, uint32_t count);
int fs_seek(int fd, uint32_t position);
int fs_list(const char* dirname);

// Block management
uint32_t alloc_block(void);
void free_block(uint32_t block);

// Utility functions
file_entry_t* find_file(const char* filename);
uint32_t get_file_size(const char* filename);

#endif
