#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

// Network constants
#define NET_BUFFER_SIZE 1514  // Ethernet MTU + header
#define MAX_PACKETS 32

// Network packet types
#define PACKET_TYPE_IP    0x0800
#define PACKET_TYPE_ARP   0x0806
#define PACKET_TYPE_ICMP  1
#define PACKET_TYPE_TCP   6
#define PACKET_TYPE_UDP   17

// Network interface structure
typedef struct {
    uint8_t mac[6];           // MAC address
    uint32_t ip;              // IP address
    uint32_t netmask;         // Subnet mask
    uint32_t gateway;         // Gateway address
    uint8_t active;           // Interface status
} network_interface_t;

// Network packet structure
typedef struct {
    uint8_t data[NET_BUFFER_SIZE];
    uint16_t length;
    uint8_t type;
    uint32_t src_ip;
    uint32_t dst_ip;
} network_packet_t;

// Network functions
void network_init(void);
int network_send_packet(uint32_t dst_ip, uint8_t type, const void* data, uint16_t length);
int network_receive_packet(network_packet_t* packet);
int network_set_ip(uint32_t ip);
int network_set_mac(const uint8_t* mac);

// System call wrappers (declared in syscall.h)

#endif
