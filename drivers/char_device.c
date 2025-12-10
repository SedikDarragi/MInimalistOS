#include "../include/device.h"
#include "../include/memory.h"
#include "../include/vga.h"
#include "string.h"

// Simple character device (console-like)
#define CHAR_BUFFER_SIZE 1024

// Character device private data
typedef struct {
    char buffer[CHAR_BUFFER_SIZE];
    uint32_t read_pos;
    uint32_t write_pos;
    uint32_t count;
} char_device_t;

static char_device_t char_dev;

// Character device operations
static int char_open(void) {
    char_dev.read_pos = 0;
    char_dev.write_pos = 0;
    char_dev.count = 0;
    return 0;
}

static int char_close(void) {
    return 0;
}

static int char_read(void* buffer, uint32_t size) {
    if (!buffer) return -1;
    
    char* buf = (char*)buffer;
    uint32_t bytes_read = 0;
    
    while (bytes_read < size && char_dev.count > 0) {
        buf[bytes_read] = char_dev.buffer[char_dev.read_pos];
        char_dev.read_pos = (char_dev.read_pos + 1) % CHAR_BUFFER_SIZE;
        char_dev.count--;
        bytes_read++;
    }
    
    return bytes_read;
}

static int char_write(const void* buffer, uint32_t size) {
    if (!buffer) return -1;
    
    const char* buf = (const char*)buffer;
    uint32_t bytes_written = 0;
    
    while (bytes_written < size && char_dev.count < CHAR_BUFFER_SIZE) {
        char_dev.buffer[char_dev.write_pos] = buf[bytes_written];
        char_dev.write_pos = (char_dev.write_pos + 1) % CHAR_BUFFER_SIZE;
        char_dev.count++;
        bytes_written++;
    }
    
    return bytes_written;
}

static int char_ioctl(uint32_t cmd, void* arg) {
    switch (cmd) {
        case 0:  // Clear buffer
            char_dev.read_pos = 0;
            char_dev.write_pos = 0;
            char_dev.count = 0;
            return 0;
            
        case 1:  // Get buffer count
            if (arg) {
                *(uint32_t*)arg = char_dev.count;
                return 0;
            }
            return -1;
            
        case 2:  // Get buffer size
            if (arg) {
                *(uint32_t*)arg = CHAR_BUFFER_SIZE;
                return 0;
            }
            return -1;
            
        default:
            return -1;
    }
}

// Device operations structure
static device_ops_t char_ops = {
    .open = char_open,
    .close = char_close,
    .read = char_read,
    .write = char_write,
    .ioctl = char_ioctl
};

// Initialize character device
void char_device_init(void) {
    char_dev.read_pos = 0;
    char_dev.write_pos = 0;
    char_dev.count = 0;
    
    // Register the device
    device_register("char0", DEV_TYPE_CHAR, 2, 0, &char_ops);
}
