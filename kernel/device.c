#include "../include/device.h"
#include "../include/memory.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "string.h"

// Device list
static device_t* device_list = NULL;

// Initialize device system
void device_init(void) {
    device_list = NULL;
}

// Register a device
int device_register(const char* name, uint32_t type, uint32_t major, uint32_t minor, device_ops_t* ops) {
    device_t* dev = (device_t*)alloc_page();
    if (!dev) return -1;
    
    strncpy(dev->name, name, 31);
    dev->name[31] = '\0';
    dev->type = type;
    dev->major = major;
    dev->minor = minor;
    dev->ops = ops;
    dev->private_data = NULL;
    dev->next = device_list;
    device_list = dev;
    
    return 0;
}

// Unregister a device
int device_unregister(const char* name) {
    device_t* current = device_list;
    device_t* prev = NULL;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                device_list = current->next;
            }
            free_page((uint32_t)current);
            return 0;
        }
        prev = current;
        current = current->next;
    }
    
    return -1;
}

// Find device by name
device_t* device_find(const char* name) {
    device_t* current = device_list;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Find device by major/minor numbers
device_t* device_find_by_number(uint32_t major, uint32_t minor) {
    device_t* current = device_list;
    
    while (current) {
        if (current->major == major && current->minor == minor) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Device I/O functions
int device_open(const char* name) {
    device_t* dev = device_find(name);
    if (!dev || !dev->ops || !dev->ops->open) {
        return -1;
    }
    
    return dev->ops->open();
}

int device_close(const char* name) {
    device_t* dev = device_find(name);
    if (!dev || !dev->ops || !dev->ops->close) {
        return -1;
    }
    
    return dev->ops->close();
}

int device_read(const char* name, void* buffer, uint32_t size) {
    device_t* dev = device_find(name);
    if (!dev || !dev->ops || !dev->ops->read) {
        return -1;
    }
    
    return dev->ops->read(buffer, size);
}

int device_write(const char* name, const void* buffer, uint32_t size) {
    device_t* dev = device_find(name);
    if (!dev || !dev->ops || !dev->ops->write) {
        return -1;
    }
    
    return dev->ops->write(buffer, size);
}

int device_ioctl(const char* name, uint32_t cmd, void* arg) {
    device_t* dev = device_find(name);
    if (!dev || !dev->ops || !dev->ops->ioctl) {
        return -1;
    }
    
    return dev->ops->ioctl(cmd, arg);
}

// System call wrappers
uint32_t sys_device_open(const char* name) {
    return device_open(name);
}

uint32_t sys_device_close(const char* name) {
    return device_close(name);
}

uint32_t sys_device_read(const char* name, void* buffer, uint32_t size) {
    return device_read(name, buffer, size);
}

uint32_t sys_device_write(const char* name, const void* buffer, uint32_t size) {
    return device_write(name, buffer, size);
}

uint32_t sys_device_ioctl(const char* name, uint32_t cmd, void* arg) {
    return device_ioctl(name, cmd, arg);
}
