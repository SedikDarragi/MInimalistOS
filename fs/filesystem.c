#include "filesystem.h"
#include "../include/vga.h"

// Local VGA functions for filesystem
static void fs_vga_print(const char* str) {
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    static int pos = 80 * 5; // Start at line 5
    
    while (*str) {
        if (*str == '\n') {
            pos = ((pos / 80) + 1) * 80;
            if (pos >= 80 * 25) pos = 0;
        } else {
            vga[pos++] = (0x0F << 8) | *str;
            if (pos >= 80 * 25) pos = 0;
        }
        str++;
    }
}

static void fs_vga_putchar(char c) {
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    static int pos = 80 * 5;
    
    if (c == '\n') {
        pos = ((pos / 80) + 1) * 80;
        if (pos >= 80 * 25) pos = 0;
    } else {
        vga[pos++] = (0x0F << 8) | c;
        if (pos >= 80 * 25) pos = 0;
    }
}

static directory_t root_directory;
static directory_t* current_directory;
static char current_path[256] = "/";

void fs_init(void) {
    // Initialize root directory
    strcpy(root_directory.name, "/");
    root_directory.file_count = 0;
    current_directory = &root_directory;
    
    // Create some sample files
    fs_write_file("readme.txt", "Welcome to MinimalOS!\nThis is a simple operating system.", 50);
    fs_write_file("hello.txt", "Hello, World!\nThis is a test file.", 35);
    
    // Create sample directory structure
    memset(&root_directory.files[root_directory.file_count], 0, sizeof(file_t));
    strcpy(root_directory.files[root_directory.file_count].name, "bin");
    root_directory.files[root_directory.file_count].is_directory = 1;
    root_directory.file_count++;
    
    memset(&root_directory.files[root_directory.file_count], 0, sizeof(file_t));
    strcpy(root_directory.files[root_directory.file_count].name, "etc");
    root_directory.files[root_directory.file_count].is_directory = 1;
    root_directory.file_count++;
}

void fs_list_directory(const char* path) {
    (void)path; // For now, just list current directory
    
    fs_vga_print("Directory listing for ");
    fs_vga_print(current_path);
    fs_vga_print(":\n");
    
    for (int i = 0; i < current_directory->file_count; i++) {
        file_t* file = &current_directory->files[i];
        if (file->is_directory) {
            fs_vga_print("[DIR]  ");
        } else {
            fs_vga_print("[FILE] ");
        }
        fs_vga_print(file->name);
        if (!file->is_directory) {
            fs_vga_print(" (");
            // Simple integer to string conversion
            char size_str[16];
            int size = file->size;
            int pos = 0;
            if (size == 0) {
                size_str[pos++] = '0';
            } else {
                while (size > 0) {
                    size_str[pos++] = '0' + (size % 10);
                    size /= 10;
                }
                // Reverse the string
                for (int j = 0; j < pos / 2; j++) {
                    char temp = size_str[j];
                    size_str[j] = size_str[pos - 1 - j];
                    size_str[pos - 1 - j] = temp;
                }
            }
            size_str[pos] = '\0';
            fs_vga_print(size_str);
            fs_vga_print(" bytes)");
        }
        fs_vga_print("\n");
    }
}

void fs_read_file(const char* filename) {
    for (int i = 0; i < current_directory->file_count; i++) {
        file_t* file = &current_directory->files[i];
        if (!file->is_directory && strcmp(file->name, filename) == 0) {
            fs_vga_print("Contents of ");
            fs_vga_print(filename);
            fs_vga_print(":\n");
            for (uint32_t j = 0; j < file->size; j++) {
                fs_vga_putchar(file->data[j]);
            }
            fs_vga_print("\n");
            return;
        }
    }
    fs_vga_print("File not found: ");
    fs_vga_print(filename);
    fs_vga_print("\n");
}

void fs_write_file(const char* filename, const char* data, uint32_t size) {
    // Check if file already exists
    for (int i = 0; i < current_directory->file_count; i++) {
        file_t* file = &current_directory->files[i];
        if (!file->is_directory && strcmp(file->name, filename) == 0) {
            // Update existing file
            file->size = size > FILE_DATA_SIZE ? FILE_DATA_SIZE : size;
            memcpy(file->data, data, file->size);
            return;
        }
    }
    
    // Create new file
    if (current_directory->file_count < MAX_FILES) {
        file_t* file = &current_directory->files[current_directory->file_count];
        strcpy(file->name, filename);
        file->size = size > FILE_DATA_SIZE ? FILE_DATA_SIZE : size;
        file->is_directory = 0;
        memcpy(file->data, data, file->size);
        current_directory->file_count++;
    } else {
        fs_vga_print("Directory full\n");
    }
}

void fs_create_directory(const char* dirname) {
    if (current_directory->file_count < MAX_FILES) {
        file_t* dir = &current_directory->files[current_directory->file_count];
        strcpy(dir->name, dirname);
        dir->size = 0;
        dir->is_directory = 1;
        current_directory->file_count++;
        fs_vga_print("Directory created: ");
        fs_vga_print(dirname);
        fs_vga_print("\n");
    } else {
        fs_vga_print("Directory full\n");
    }
}

void fs_delete_file(const char* filename) {
    for (int i = 0; i < current_directory->file_count; i++) {
        file_t* file = &current_directory->files[i];
        if (strcmp(file->name, filename) == 0) {
            // Shift remaining files
            for (int j = i; j < current_directory->file_count - 1; j++) {
                current_directory->files[j] = current_directory->files[j + 1];
            }
            current_directory->file_count--;
            fs_vga_print("Deleted: ");
            fs_vga_print(filename);
            fs_vga_print("\n");
            return;
        }
    }
    fs_vga_print("File not found: ");
    fs_vga_print(filename);
    fs_vga_print("\n");
}

void fs_change_directory(const char* dirname) {
    if (strcmp(dirname, "..") == 0) {
        // Go to parent directory (simplified - just go to root)
        current_directory = &root_directory;
        strcpy(current_path, "/");
        fs_vga_print("Changed to root directory\n");
        return;
    }
    
    for (int i = 0; i < current_directory->file_count; i++) {
        file_t* file = &current_directory->files[i];
        if (file->is_directory && strcmp(file->name, dirname) == 0) {
            // For simplicity, we don't actually change directory structure
            // Just update the path display
            if (strcmp(current_path, "/") != 0) {
                strcat(current_path, "/");
            }
            strcat(current_path, dirname);
            fs_vga_print("Changed directory to: ");
            fs_vga_print(current_path);
            fs_vga_print("\n");
            return;
        }
    }
    fs_vga_print("Directory not found: ");
    fs_vga_print(dirname);
    fs_vga_print("\n");
}

void fs_print_working_directory(void) {
    fs_vga_print(current_path);
    fs_vga_print("\n");
}
