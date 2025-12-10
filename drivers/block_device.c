#include "../include/device.h"
#include "../include/memory.h"
#include "../include/vga.h"
#include "string.h"

// Simple block device with 1024 blocks of 512 bytes each
#define BLOCK_SIZE      512
#define NUM_BLOCKS      1024
#define DEVICE_SIZE     (BLOCK_SIZE * NUM_BLOCKS)

// Block device private data
typedef struct {
    uint8_t* data;
    uint32_t block_count;
    uint32_t current_block;
} block_device_t;

static block_device_t block_dev;

// Block device operations
static int block_open(void) {
    if (!block_dev.data) {
        block_dev.data = (uint8_t*)alloc_pages(DEVICE_SIZE / PAGE_SIZE);
        if (!block_dev.data) return -1;
        
        block_dev.block_count = NUM_BLOCKS;
        block_dev.current_block = 0;
        
        // Initialize with zeros
        memset(block_dev.data, 0, DEVICE_SIZE);
    }
    
    return 0;
}

static int block_close(void) {
    // For this simple implementation, we keep the data allocated
    return 0;
}

static int block_read(void* buffer, uint32_t size) {
    if (!block_dev.data || !buffer) return -1;
    
    uint32_t bytes_to_read = size;
    uint32_t offset = block_dev.current_block * BLOCK_SIZE;
    
    if (offset + bytes_to_read > DEVICE_SIZE) {
        bytes_to_read = DEVICE_SIZE - offset;
    }
    
    memcpy(buffer, block_dev.data + offset, bytes_to_read);
    return bytes_to_read;
}

static int block_write(const void* buffer, uint32_t size) {
    if (!block_dev.data || !buffer) return -1;
    
    uint32_t bytes_to_write = size;
    uint32_t offset = block_dev.current_block * BLOCK_SIZE;
    
    if (offset + bytes_to_write > DEVICE_SIZE) {
        bytes_to_write = DEVICE_SIZE - offset;
    }
    
    memcpy(block_dev.data + offset, buffer, bytes_to_write);
    return bytes_to_write;
}

static int block_ioctl(uint32_t cmd, void* arg) {
    switch (cmd) {
        case 0:  // Set block
            if (arg) {
                uint32_t block = *(uint32_t*)arg;
                if (block < block_dev.block_count) {
                    block_dev.current_block = block;
                    return 0;
                }
            }
            return -1;
            
        case 1:  // Get block count
            if (arg) {
                *(uint32_t*)arg = block_dev.block_count;
                return 0;
            }
            return -1;
            
        case 2:  // Get current block
            if (arg) {
                *(uint32_t*)arg = block_dev.current_block;
                return 0;
            }
            return -1;
            
        default:
            return -1;
    }
}

// Device operations structure
static device_ops_t block_ops = {
    .open = block_open,
    .close = block_close,
    .read = block_read,
    .write = block_write,
    .ioctl = block_ioctl
};

// Initialize block device
void block_device_init(void) {
    block_dev.data = NULL;
    block_dev.block_count = 0;
    block_dev.current_block = 0;
    
    // Register the device
    device_register("block0", DEV_TYPE_BLOCK, 1, 0, &block_ops);
}
