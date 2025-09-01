#ifndef NETWORK_H
#define NETWORK_H

#include "../kernel/kernel.h"

#define IP_ADDR_LEN 4
#define MAC_ADDR_LEN 6

typedef struct {
    uint8_t ip[IP_ADDR_LEN];
    uint8_t mac[MAC_ADDR_LEN];
    uint8_t netmask[IP_ADDR_LEN];
    uint8_t gateway[IP_ADDR_LEN];
    int is_up;
} network_interface_t;

void network_init(void);
void network_ping(const char* host);
void network_show_config(void);
void network_set_ip(const char* ip_str);
int network_send_packet(const uint8_t* data, uint32_t size);
int network_receive_packet(uint8_t* buffer, uint32_t max_size);

#endif
