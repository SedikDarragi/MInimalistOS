#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

// Device types
#define DEV_TYPE_BLOCK     1
#define DEV_TYPE_CHAR      2
#define DEV_TYPE_NETWORK   3
#define DEV_TYPE_DISPLAY   4
#define DEV_TYPE_INPUT     5

// Device operations
typedef struct device_ops {
    int (*open)(void);
    int (*close)(void);
    int (*read)(void* buffer, uint32_t size);
    int (*write)(const void* buffer, uint32_t size);
    int (*ioctl)(uint32_t cmd, void* arg);
} device_ops_t;

// Device structure
typedef struct device {
    char name[32];
    uint32_t type;
    uint32_t major;
    uint32_t minor;
    device_ops_t* ops;
    void* private_data;
    struct device* next;
} device_t;

// Device management functions
int device_register(const char* name, uint32_t type, uint32_t major, uint32_t minor, device_ops_t* ops);
int device_unregister(const char* name);
device_t* device_find(const char* name);
device_t* device_find_by_number(uint32_t major, uint32_t minor);

// Device I/O functions
int device_open(const char* name);
int device_close(const char* name);
int device_read(const char* name, void* buffer, uint32_t size);
int device_write(const char* name, const void* buffer, uint32_t size);
int device_ioctl(const char* name, uint32_t cmd, void* arg);

// System call wrappers
uint32_t sys_device_open(const char* name);
uint32_t sys_device_close(const char* name);
uint32_t sys_device_read(const char* name, void* buffer, uint32_t size);
uint32_t sys_device_write(const char* name, const void* buffer, uint32_t size);
uint32_t sys_device_ioctl(const char* name, uint32_t cmd, void* arg);

#endif
