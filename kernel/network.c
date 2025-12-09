#include "../include/network.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "string.h"

// Global network interface
static network_interface_t netif;

// Packet buffer for received packets
static network_packet_t packet_buffer[MAX_PACKETS];
static int packet_head = 0;
static int packet_tail = 0;
static int packet_count = 0;

// Initialize network stack
void network_init(void) {
    // Clear network interface
    memset(&netif, 0, sizeof(netif));
    
    // Set default MAC address
    netif.mac[0] = 0x52;
    netif.mac[1] = 0x54;
    netif.mac[2] = 0x00;
    netif.mac[3] = 0x12;
    netif.mac[4] = 0x34;
    netif.mac[5] = 0x56;
    
    // Set default IP configuration
    netif.ip = 0xC0A80164;    // 192.168.1.100
    netif.netmask = 0xFFFFFF00;  // 255.255.255.0
    netif.gateway = 0xC0A80101;  // 192.168.1.1
    
    netif.active = 1;
    
    // Clear packet buffer
    memset(packet_buffer, 0, sizeof(packet_buffer));
    packet_head = 0;
    packet_tail = 0;
    packet_count = 0;
}

// Send a network packet
int network_send_packet(uint32_t dst_ip, uint8_t type, const void* data, uint16_t length) {
    if (!netif.active || length > NET_BUFFER_SIZE) {
        return -1;
    }
    
    // In a real implementation, this would:
    // 1. Create Ethernet frame
    // 2. Add IP header
    // 3. Add transport header (TCP/UDP)
    // 4. Send to network hardware
    
    // For simulation, we'll just create a packet entry
    if (packet_count < MAX_PACKETS) {
        network_packet_t* packet = &packet_buffer[packet_tail];
        
        packet->dst_ip = dst_ip;
        packet->src_ip = netif.ip;
        packet->type = type;
        packet->length = length;
        
        if (data && length > 0) {
            memcpy(packet->data, data, length);
        }
        
        packet_tail = (packet_tail + 1) % MAX_PACKETS;
        packet_count++;
        
        return 0;
    }
    
    return -1;  // Buffer full
}

// Receive a network packet
int network_receive_packet(network_packet_t* packet) {
    if (packet_count == 0) {
        return -1;  // No packets available
    }
    
    // Copy packet from buffer
    network_packet_t* src_packet = &packet_buffer[packet_head];
    memcpy(packet, src_packet, sizeof(network_packet_t));
    
    // Remove packet from buffer
    packet_head = (packet_head + 1) % MAX_PACKETS;
    packet_count--;
    
    return 0;
}

// Set network interface IP
int network_set_ip(uint32_t ip) {
    netif.ip = ip;
    return 0;
}

// Set network interface MAC
int network_set_mac(const uint8_t* mac) {
    if (mac) {
        memcpy(netif.mac, mac, 6);
        return 0;
    }
    return -1;
}

// System call wrappers
uint32_t sys_network_send(uint32_t dst_ip, uint8_t type, const void* data, uint16_t length) {
    int result = network_send_packet(dst_ip, type, data, length);
    return (result >= 0) ? 0 : -1;
}

uint32_t sys_network_receive(void* packet) {
    int result = network_receive_packet((network_packet_t*)packet);
    return (result >= 0) ? 0 : -1;
}
