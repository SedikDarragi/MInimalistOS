#include "../include/memory.h"
#include "../drivers/vga.h"
#include "log.h"

// Test memory allocation
void test_memory_allocation(void) {
    log_info("Testing memory allocation...");
    
    // Test basic allocation
    void* ptr1 = kmalloc(100);
    if (ptr1) {
        log_info("✓ Basic allocation successful");
    } else {
        log_error("✗ Basic allocation failed");
    }
    
    // Test aligned allocation
    void* ptr2 = kmalloc_aligned(256, 16);
    if (ptr2 && ((uint32_t)ptr2 % 16 == 0)) {
        log_info("✓ Aligned allocation successful");
    } else {
        log_error("✗ Aligned allocation failed");
    }
    
    // Test zero-initialized allocation
    void* ptr3 = kcalloc(10, 20);
    if (ptr3) {
        log_info("✓ Zero-initialized allocation successful");
        // Verify it's actually zeroed
        uint8_t* bytes = (uint8_t*)ptr3;
        int all_zero = 1;
        for (int i = 0; i < 200; i++) {
            if (bytes[i] != 0) {
                all_zero = 0;
                break;
            }
        }
        if (all_zero) {
            log_info("✓ Memory properly zeroed");
        } else {
            log_error("✗ Memory not properly zeroed");
        }
    } else {
        log_error("✗ Zero-initialized allocation failed");
    }
    
    // Test multiple allocations
    void* ptrs[10];
    int success = 1;
    for (int i = 0; i < 10; i++) {
        ptrs[i] = kmalloc(100 * (i + 1));
        if (!ptrs[i]) {
            success = 0;
            break;
        }
    }
    
    if (success) {
        log_info("✓ Multiple allocations successful");
    } else {
        log_error("✗ Multiple allocations failed");
    }
}

// Test memory statistics
void test_memory_statistics(void) {
    log_info("Testing memory statistics...");
    
    uint32_t total_pages, used_pages, free_pages;
    memory_stats(&total_pages, &used_pages, &free_pages);
    
    if (total_pages > 0 && used_pages > 0) {
        log_info("✓ Page statistics available");
        log_info("  Total pages: ");
        // Simple number printing
        if (total_pages >= 1000) {
            vga_putchar('0' + (total_pages / 1000) % 10);
        }
        if (total_pages >= 100) {
            vga_putchar('0' + (total_pages / 100) % 10);
        }
        if (total_pages >= 10) {
            vga_putchar('0' + (total_pages / 10) % 10);
        }
        vga_putchar('0' + total_pages % 10);
        vga_print("\n");
        
        log_info("  Used pages: ");
        if (used_pages >= 1000) {
            vga_putchar('0' + (used_pages / 1000) % 10);
        }
        if (used_pages >= 100) {
            vga_putchar('0' + (used_pages / 100) % 10);
        }
        if (used_pages >= 10) {
            vga_putchar('0' + (used_pages / 10) % 10);
        }
        vga_putchar('0' + used_pages % 10);
        vga_print("\n");
        
        log_info("  Free pages: ");
        if (free_pages >= 1000) {
            vga_putchar('0' + (free_pages / 1000) % 10);
        }
        if (free_pages >= 100) {
            vga_putchar('0' + (free_pages / 100) % 10);
        }
        if (free_pages >= 10) {
            vga_putchar('0' + (free_pages / 10) % 10);
        }
        vga_putchar('0' + free_pages % 10);
        vga_print("\n");
    } else {
        log_error("✗ Invalid page statistics");
    }
    
    uint32_t total_heap, used_heap;
    heap_stats(&total_heap, &used_heap);
    
    if (total_heap > 0) {
        log_info("✓ Heap statistics available");
        log_info("  Total heap: ");
        if (total_heap >= 1000000) {
            vga_putchar('0' + (total_heap / 1000000) % 10);
        }
        if (total_heap >= 100000) {
            vga_putchar('0' + (total_heap / 100000) % 10);
        }
        if (total_heap >= 10000) {
            vga_putchar('0' + (total_heap / 10000) % 10);
        }
        if (total_heap >= 1000) {
            vga_putchar('0' + (total_heap / 1000) % 10);
        }
        if (total_heap >= 100) {
            vga_putchar('0' + (total_heap / 100) % 10);
        }
        if (total_heap >= 10) {
            vga_putchar('0' + (total_heap / 10) % 10);
        }
        vga_putchar('0' + total_heap % 10);
        vga_print("\n");
        
        log_info("  Used heap: ");
        if (used_heap >= 1000000) {
            vga_putchar('0' + (used_heap / 1000000) % 10);
        }
        if (used_heap >= 100000) {
            vga_putchar('0' + (used_heap / 100000) % 10);
        }
        if (used_heap >= 10000) {
            vga_putchar('0' + (used_heap / 10000) % 10);
        }
        if (used_heap >= 1000) {
            vga_putchar('0' + (used_heap / 1000) % 10);
        }
        if (used_heap >= 100) {
            vga_putchar('0' + (used_heap / 100) % 10);
        }
        if (used_heap >= 10) {
            vga_putchar('0' + (used_heap / 10) % 10);
        }
        vga_putchar('0' + used_heap % 10);
        vga_print("\n");
    } else {
        log_error("✗ Invalid heap statistics");
    }
}

// Test page allocation
void test_page_allocation(void) {
    log_info("Testing page allocation...");
    
    // Allocate a few pages
    uint32_t page1 = alloc_page();
    uint32_t page2 = alloc_page();
    uint32_t page3 = alloc_page();
    
    if (page1 && page2 && page3) {
        log_info("✓ Page allocation successful");
        log_info("  Allocated pages at: ");
        // Print page addresses (simplified)
        vga_print("0x");
        for (int shift = 28; shift >= 0; shift -= 4) {
            uint8_t nibble = (page1 >> shift) & 0xF;
            if (nibble < 10) {
                vga_putchar('0' + nibble);
            } else {
                vga_putchar('A' + nibble - 10);
            }
        }
        vga_print("\n");
        
        // Test page mapping
        map_page(0x10000000, page1, PAGE_PRESENT | PAGE_WRITE);
        volatile uint32_t* mapped = (volatile uint32_t*)0x10000000;
        *mapped = 0xDEADBEEF;
        
        if (*mapped == 0xDEADBEEF) {
            log_info("✓ Page mapping verification successful");
        } else {
            log_error("✗ Page mapping verification failed");
        }
        
        // Clean up
        unmap_page(0x10000000);
        free_page(page1);
        free_page(page2);
        free_page(page3);
        log_info("✓ Page deallocation successful");
    } else {
        log_error("✗ Page allocation failed");
    }
}

// Memory test process
void memory_test_process(void) {
    log_init();
    log_info("=== Memory Management Tests ===");
    log_info("");
    
    test_memory_allocation();
    log_info("");
    
    test_memory_statistics();
    log_info("");
    
    test_page_allocation();
    log_info("");
    
    log_info("=== Memory Tests Complete ===");
    
    while (1) {
        // Halt the CPU
        asm volatile("hlt");
    }
}
