#ifndef VM_H
#define VM_H

#include <stdint.h>

// Virtual memory constants
#define PAGE_SIZE        4096
#define PAGE_ENTRIES     1024
#define KERNEL_BASE      0xC0000000
#define USER_BASE        0x00000000
#define PAGE_TABLE_SIZE  (PAGE_ENTRIES * sizeof(uint32_t))

// Page flags
#define PAGE_PRESENT     0x001
#define PAGE_WRITE       0x002
#define PAGE_USER        0x004
#define PAGE_WRITETHROUGH 0x008
#define PAGE_NOCACHE     0x010
#define PAGE_ACCESSED    0x020
#define PAGE_DIRTY       0x040
#define PAGE_4MB        0x080
#define PAGE_GLOBAL     0x100

// Memory region structure
typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t flags;
    char name[32];
} memory_region_t;

// Page table entry
typedef struct {
    uint32_t present    : 1;
    uint32_t write      : 1;
    uint32_t user       : 1;
    uint32_t writethrough : 1;
    uint32_t nocache    : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t pat        : 1;
    uint32_t global     : 1;
    uint32_t avail      : 3;
    uint32_t frame      : 20;
} page_entry_t;

// Virtual memory functions
void vm_init(void);
int vm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
int vm_unmap_page(uint32_t virt_addr);
uint32_t vm_get_phys_addr(uint32_t virt_addr);
int vm_alloc_page(uint32_t virt_addr, uint32_t flags);
int vm_free_page(uint32_t virt_addr);

// Memory region management
int vm_add_region(uint32_t start, uint32_t end, uint32_t flags, const char* name);
memory_region_t* vm_find_region(uint32_t addr);
void vm_dump_regions(void);

// System call wrappers
uint32_t sys_vm_alloc(uint32_t size, uint32_t flags);
uint32_t sys_vm_free(uint32_t addr);
uint32_t sys_vm_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);

#endif
