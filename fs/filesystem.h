#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../kernel/kernel.h"

#define MAX_FILENAME 32
#define MAX_FILES 64
#define MAX_DIRECTORIES 16
#define FILE_DATA_SIZE 1024

typedef struct {
    char name[MAX_FILENAME];
    uint32_t size;
    uint8_t data[FILE_DATA_SIZE];
    int is_directory;
} file_t;

typedef struct {
    char name[MAX_FILENAME];
    file_t files[MAX_FILES];
    int file_count;
} directory_t;

void fs_init(void);
void fs_list_directory(const char* path);
void fs_read_file(const char* filename);
void fs_write_file(const char* filename, const char* data, uint32_t size);
void fs_create_directory(const char* dirname);
void fs_delete_file(const char* filename);
void fs_change_directory(const char* dirname);
void fs_print_working_directory(void);

#endif
