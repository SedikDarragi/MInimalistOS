#ifndef NET_H
#define NET_H

#include <stdint.h>

// Maximum number of network devices supported
#define NET_MAX_DEVICES 8

// Forward declaration
struct net_device;

// Network device operations
typedef int  (*net_send_fn)(struct net_device* dev, const void* buf, uint32_t len);
typedef void (*net_poll_fn)(struct net_device* dev);

// Generic network device descriptor
typedef struct net_device {
    uint8_t  mac[6];        // MAC address (if applicable)
    uint32_t mtu;           // Maximum transmission unit
    const char* name;       // Device name, e.g., "eth0"

    // Driver-specific context pointer
    void* driver_data;

    // Driver operations
    net_send_fn send;
    net_poll_fn poll;
} net_device_t;

// Core networking API
void net_init(void);
int  net_register_device(net_device_t* dev);
void net_poll_all(void);
uint32_t net_get_device_count(void);
net_device_t* net_get_device(uint32_t index);

// Frame receive entry point for NIC drivers (to be called from their RX path)
void net_receive_frame(net_device_t* dev, const uint8_t* buf, uint32_t len);

// Optional simple statistics
typedef struct net_stats {
    uint32_t rx_frames;
    uint32_t rx_bytes;
    uint32_t tx_frames;
    uint32_t tx_bytes;
} net_stats_t;

void net_get_stats(net_stats_t* out);

#endif // NET_H
