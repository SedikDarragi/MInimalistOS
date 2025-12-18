#include "memory.h"
#include "../include/vga.h"
#include "string.h"

// Page directory and initial page table
static page_directory_t kernel_page_directory __attribute__((aligned(PAGE_SIZE)));
static page_table_t kernel_page_table __attribute__((aligned(PAGE_SIZE)));

// Bitmap for physical memory allocation
#define MAX_PAGES 1024  // 4MB of physical memory for simplicity
static uint32_t page_bitmap[MAX_PAGES / 32];
static uint32_t total_pages = MAX_PAGES;
static uint32_t used_pages = 0;

// Simple heap implementation
#define HEAP_SIZE 0x100000  // 1MB heap
static uint8_t heap[HEAP_SIZE];
static uint32_t heap_pos = 0;

// Initialize memory management
void memory_init(void) {
    // Clear page bitmap
    memset(page_bitmap, 0, sizeof(page_bitmap));
    
    // Mark first 4MB as used (kernel space)
    for (uint32_t i = 0; i < 1024; i++) {
        page_bitmap[i / 32] |= (1 << (i % 32));
    }
    used_pages = 1024;
    
    // Initialize heap
    heap_init();
}

// Initialize paging
void paging_init(void) {
    // Clear page directory and page table
    memset(&kernel_page_directory, 0, sizeof(kernel_page_directory));
    memset(&kernel_page_table, 0, sizeof(kernel_page_table));
    
    // Map first 4MB of physical memory to virtual address 0x00000000
    for (uint32_t i = 0; i < 1024; i++) {
        kernel_page_table.entries[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITE;
    }
    
    // Set page directory entry 0 to point to kernel page table
    kernel_page_directory.entries[0] = (uint32_t)&kernel_page_table | PAGE_PRESENT | PAGE_WRITE;
    
    // Map kernel space (0xC0000000) to same physical memory
    kernel_page_directory.entries[768] = (uint32_t)&kernel_page_table | PAGE_PRESENT | PAGE_WRITE;
    
    // Enable paging
    enable_paging();
}

// Enable paging
void enable_paging(void) {
    uint32_t cr0;
    
    // Load page directory address into CR3
    asm volatile ("mov %0, %%cr3" : : "r" ((uint32_t)&kernel_page_directory));
    
    // Read CR0, set paging bit (bit 31), and write back
    asm volatile ("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;  // Enable paging
    asm volatile ("mov %0, %%cr0" : : "r" (cr0));
}

// Allocate a physical page
uint32_t alloc_page(void) {
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!(page_bitmap[i / 32] & (1 << (i % 32)))) {
            page_bitmap[i / 32] |= (1 << (i % 32));
            used_pages++;
            return i * PAGE_SIZE;
        }
    }
    return 0;  // Out of memory
}

// Free a physical page
void free_page(uint32_t page_addr) {
    uint32_t page_num = page_addr / PAGE_SIZE;
    if (page_num < total_pages) {
        page_bitmap[page_num / 32] &= ~(1 << (page_num % 32));
        used_pages--;
    }
}

// Map a virtual page to a physical page
void map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    uint32_t page_dir_index = virt_addr >> 22;
    uint32_t page_table_index = (virt_addr >> 12) & 0x3FF;
    
    // Get page table (for simplicity, we assume it exists)
    page_table_t* page_table = (page_table_t*)(kernel_page_directory.entries[page_dir_index] & ~0xFFF);
    
    // Set page table entry
    page_table->entries[page_table_index] = phys_addr | flags;
    
    // Flush TLB
    flush_tlb();
}

// Unmap a virtual page
void unmap_page(uint32_t virt_addr) {
    uint32_t page_dir_index = virt_addr >> 22;
    uint32_t page_table_index = (virt_addr >> 12) & 0x3FF;
    
    // Get page table
    page_table_t* page_table = (page_table_t*)(kernel_page_directory.entries[page_dir_index] & ~0xFFF);
    
    // Clear page table entry
    page_table->entries[page_table_index] = 0;
    
    // Flush TLB
    flush_tlb();
}

// Get physical address from virtual address
uint32_t get_phys_addr(uint32_t virt_addr) {
    uint32_t page_dir_index = virt_addr >> 22;
    uint32_t page_table_index = (virt_addr >> 12) & 0x3FF;
    uint32_t page_offset = virt_addr & 0xFFF;
    
    // Get page table
    page_table_t* page_table = (page_table_t*)(kernel_page_directory.entries[page_dir_index] & ~0xFFF);
    
    // Get physical address
    uint32_t phys_addr = page_table->entries[page_table_index] & ~0xFFF;
    return phys_addr + page_offset;
}

// Flush TLB
void flush_tlb(void) {
    asm volatile ("mov %%cr3, %%eax; mov %%eax, %%cr3" : : : "eax");
}

// Initialize heap
void heap_init(void) {
    heap_pos = 0;
}

// Allocate memory from heap
void* kmalloc(uint32_t size) {
    if (heap_pos + size > HEAP_SIZE) {
        return NULL;  // Out of heap memory
    }
    
    void* ptr = &heap[heap_pos];
    heap_pos += (size + 3) & ~3;  // Align to 4 bytes
    return ptr;
}

// Free memory (simplified - does nothing for now)
void kfree(void* ptr) {
    // For simplicity, we don't implement freeing in this simple heap
    (void)ptr;
}

// Memory statistics
void memory_stats(uint32_t* total, uint32_t* used, uint32_t* free) {
    if (total) *total = total_pages;
    if (used) *used = used_pages;
    if (free) *free = total_pages - used_pages;
}

// Get heap statistics
void heap_stats(uint32_t* total_heap, uint32_t* used_heap) {
    if (total_heap) *total_heap = HEAP_SIZE;
    if (used_heap) *used_heap = heap_pos;
}

// Enhanced heap allocator with better alignment
void* kmalloc_aligned(uint32_t size, uint32_t alignment) {
    // Align the heap position
    uint32_t aligned_pos = (heap_pos + alignment - 1) & ~(alignment - 1);
    
    if (aligned_pos + size > HEAP_SIZE) {
        return NULL;  // Out of heap memory
    }
    
    void* ptr = &heap[aligned_pos];
    heap_pos = aligned_pos + size;
    return ptr;
}

// Zero-initialized allocation
void* kcalloc(uint32_t num, uint32_t size) {
    uint32_t total_size = num * size;
    void* ptr = kmalloc(total_size);
    
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}
