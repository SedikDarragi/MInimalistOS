#include "../include/program_loader.h"
#include "../include/filesystem.h"
#include "../include/memory.h"
#include "../include/process_simple.h"
#include "../kernel/log.h"
#include "../drivers/vga.h"
#include "../include/string.h"

// ELF 32-bit structures
typedef struct {
    uint32_t e_magic;
    uint8_t  e_class;
    uint8_t  e_data;
    uint8_t  e_version;
    uint8_t  e_osabi;
    uint8_t  e_abiversion;
    uint8_t  e_pad[7];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version2;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) elf32_header_t;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed)) elf32_phdr_t;

// Memory allocation for programs
static uint8_t program_memory[MAX_PROGRAM_SIZE];
static uint32_t program_memory_used = 0;

// Initialize program loader
int program_loader_init(void) {
    program_memory_used = 0;
    log_info("Program loader initialized");
    return 0;
}

// Validate ELF header
int program_validate_elf(const void* elf_data) {
    const elf32_header_t* header = (const elf32_header_t*)elf_data;
    
    // Check ELF magic number
    if (header->e_magic != ELF_MAGIC) {
        log_info("Invalid ELF magic number");
        return -1;
    }
    
    // Check 32-bit class
    if (header->e_class != ELF_CLASS_32) {
        log_info("Not 32-bit ELF");
        return -1;
    }
    
    // Check little endian
    if (header->e_data != ELF_DATA_LSB) {
        log_info("Not little endian ELF");
        return -1;
    }
    
    // Check executable type
    if (header->e_type != ELF_TYPE_EXEC) {
        log_info("Not executable ELF");
        return -1;
    }
    
    // Check i386 architecture
    if (header->e_machine != ELF_MACHINE_386) {
        log_info("Not i386 ELF");
        return -1;
    }
    
    return 0;
}

// Allocate memory for program
void* program_alloc_memory(uint32_t size, uint32_t alignment) {
    uint32_t aligned_addr = (program_memory_used + alignment - 1) & ~(alignment - 1);
    
    if (aligned_addr + size > MAX_PROGRAM_SIZE) {
        log_info("Out of program memory");
        return 0;
    }
    
    void* ptr = &program_memory[aligned_addr];
    program_memory_used = aligned_addr + size;
    return ptr;
}

// Free program memory
void program_free_memory(void* ptr, uint32_t size) {
    // Simple implementation - just reset if this was the last allocation
    if ((uint8_t*)ptr + size == &program_memory[program_memory_used]) {
        program_memory_used -= size;
    }
}

// Load program from file
int program_load(const char* filename, program_info_t* prog_info) {
    if (!filename || !prog_info) {
        return -1;
    }
    
    // Open file
    int fd = fs_open(filename, 0);
    if (fd < 0) {
        log_info("Failed to open program file");
        return -1;
    }
    
    // Get file size
    int file_size = fs_seek(fd, 0, SEEK_END);
    if (file_size <= 0 || file_size > MAX_PROGRAM_SIZE) {
        fs_close(fd);
        log_info("Invalid program file size");
        return -1;
    }
    
    // Read ELF header
    elf32_header_t header;
    fs_seek(fd, 0, SEEK_SET);
    if (fs_read(fd, &header, sizeof(header)) != sizeof(header)) {
        fs_close(fd);
        log_info("Failed to read ELF header");
        return -1;
    }
    
    // Validate ELF
    if (program_validate_elf(&header) != 0) {
        fs_close(fd);
        log_info("Invalid ELF file");
        return -1;
    }
    
    // Read program headers
    uint32_t ph_size = header.e_phentsize * header.e_phnum;
    elf32_phdr_t* phdrs = (elf32_phdr_t*)program_alloc_memory(ph_size, 4);
    if (!phdrs) {
        fs_close(fd);
        log_info("Failed to allocate memory for program headers");
        return -1;
    }
    
    fs_seek(fd, header.e_phoff, SEEK_SET);
    if (fs_read(fd, phdrs, ph_size) != ph_size) {
        program_free_memory(phdrs, ph_size);
        fs_close(fd);
        log_info("Failed to read program headers");
        return -1;
    }
    
    // Initialize program info
    prog_info->entry_point = header.e_entry;
    prog_info->base_address = 0xFFFFFFFF;
    prog_info->size = 0;
    prog_info->stack_size = 8192;  // 8KB stack
    prog_info->is_loaded = 0;
    
    // Load program segments
    for (uint16_t i = 0; i < header.e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            uint32_t vaddr = phdrs[i].p_vaddr;
            uint32_t memsz = phdrs[i].p_memsz;
            uint32_t filesz = phdrs[i].p_filesz;
            
            // Update base address and size
            if (vaddr < prog_info->base_address) {
                prog_info->base_address = vaddr;
            }
            if (vaddr + memsz > prog_info->base_address + prog_info->size) {
                prog_info->size = vaddr + memsz - prog_info->base_address;
            }
            
            // Allocate memory for segment
            void* segment_ptr = program_alloc_memory(vaddr + memsz, 4096);
            if (!segment_ptr) {
                program_free_memory(phdrs, ph_size);
                fs_close(fd);
                log_info("Failed to allocate memory for program segment");
                return -1;
            }
            
            // Read segment data
            if (filesz > 0) {
                fs_seek(fd, phdrs[i].p_offset, SEEK_SET);
                if (fs_read(fd, (uint8_t*)segment_ptr + vaddr, filesz) != filesz) {
                    program_free_memory(phdrs, ph_size);
                    fs_close(fd);
                    log_info("Failed to read program segment");
                    return -1;
                }
            }
            
            // Zero BSS
            if (memsz > filesz) {
                memset((uint8_t*)segment_ptr + vaddr + filesz, 0, memsz - filesz);
            }
        }
    }
    
    // Clean up
    program_free_memory(phdrs, ph_size);
    fs_close(fd);
    
    prog_info->is_loaded = 1;
    log_info("Program loaded successfully");
    return 0;
}

// Execute program from file
int program_execute(const char* filename) {
    program_info_t prog_info;
    
    if (program_load(filename, &prog_info) != 0) {
        return -1;
    }
    
    int result = program_execute_from_info(&prog_info);
    program_cleanup(&prog_info);
    
    return result;
}

// Execute program from loaded info
int program_execute_from_info(const program_info_t* prog_info) {
    if (!prog_info || !prog_info->is_loaded) {
        return -1;
    }
    
    // Allocate stack
    void* stack_ptr = program_alloc_memory(prog_info->stack_size, 4);
    if (!stack_ptr) {
        log_info("Failed to allocate stack for program");
        return -1;
    }
    
    // Create process
    int pid = process_create((void*)prog_info->entry_point, "user_program");
    if (pid < 0) {
        program_free_memory(stack_ptr, prog_info->stack_size);
        log_info("Failed to create process for program");
        return -1;
    }
    
    vga_print("Program executed successfully\n");
    log_info("Program execution started");
    
    return pid;
}

// Clean up program resources
void program_cleanup(const program_info_t* prog_info) {
    if (prog_info && prog_info->is_loaded) {
        // Free program memory (simplified - just reset counter)
        program_memory_used = 0;
    }
}
