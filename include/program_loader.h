#ifndef PROGRAM_LOADER_H
#define PROGRAM_LOADER_H

#include <stdint.h>

// ELF header constants
#define ELF_MAGIC 0x464C457F
#define ELF_CLASS_32 1
#define ELF_DATA_LSB 1
#define ELF_TYPE_EXEC 2
#define ELF_MACHINE_386 3
#define ELF_VERSION_CURRENT 1

// Program header types
#define PT_LOAD 1
#define PT_NULL 0

// Maximum program size
#define MAX_PROGRAM_SIZE (1024 * 1024)  // 1MB

// Program information structure
typedef struct {
    uint32_t entry_point;
    uint32_t base_address;
    uint32_t size;
    uint32_t stack_size;
    uint8_t is_loaded;
} program_info_t;

// Function declarations
int program_loader_init(void);
int program_load(const char* filename, program_info_t* prog_info);
int program_execute(const char* filename);
int program_execute_from_info(const program_info_t* prog_info);
void program_cleanup(const program_info_t* prog_info);
int program_validate_elf(const void* elf_data);
void* program_alloc_memory(uint32_t size, uint32_t alignment);
void program_free_memory(void* ptr, uint32_t size);

#endif // PROGRAM_LOADER_H
