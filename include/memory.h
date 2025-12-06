#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

// Memory page size (4KB)
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

// Page directory and table entries
#define PAGE_PRESENT    0x001
#define PAGE_WRITE      0x002
#define PAGE_USER       0x004
#define PAGE_WRITETHROUGH 0x008
#define PAGE_NOCACHE    0x010
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_4MB        0x080
#define PAGE_GLOBAL     0x100

// Memory regions
#define KERNEL_BASE     0xC0000000
#define KERNEL_END      0xC0100000
#define USER_BASE       0x00000000
#define USER_END        0x80000000

// Page directory structure
typedef struct {
    uint32_t entries[1024];
} page_directory_t;

// Page table structure
typedef struct {
    uint32_t entries[1024];
} page_table_t;

// Memory management functions
void memory_init(void);
void paging_init(void);
void enable_paging(void);

// Page allocation
uint32_t alloc_page(void);
void free_page(uint32_t page_addr);
void map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void unmap_page(uint32_t virt_addr);

// Memory utilities
uint32_t get_phys_addr(uint32_t virt_addr);
void flush_tlb(void);

// Heap management
void heap_init(void);
void* kmalloc(uint32_t size);
void kfree(void* ptr);

#endif
