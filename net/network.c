#include "network.h"
#include "../drivers/vga.h"

static network_interface_t eth0;

void network_init(void) {
    // Initialize default network interface
    eth0.ip[0] = 192;
    eth0.ip[1] = 168;
    eth0.ip[2] = 1;
    eth0.ip[3] = 100;
    
    eth0.mac[0] = 0x00;
    eth0.mac[1] = 0x0C;
    eth0.mac[2] = 0x29;
    eth0.mac[3] = 0x3E;
    eth0.mac[4] = 0x4F;
    eth0.mac[5] = 0x5A;
    
    eth0.netmask[0] = 255;
    eth0.netmask[1] = 255;
    eth0.netmask[2] = 255;
    eth0.netmask[3] = 0;
    
    eth0.gateway[0] = 192;
    eth0.gateway[1] = 168;
    eth0.gateway[2] = 1;
    eth0.gateway[3] = 1;
    
    eth0.is_up = 1;
}

static void print_ip(const uint8_t* ip) {
    for (int i = 0; i < IP_ADDR_LEN; i++) {
        // Simple integer to string conversion
        int num = ip[i];
        if (num >= 100) {
            vga_putchar('0' + (num / 100));
            num %= 100;
        }
        if (num >= 10) {
            vga_putchar('0' + (num / 10));
            num %= 10;
        }
        vga_putchar('0' + num);
        
        if (i < IP_ADDR_LEN - 1) {
            vga_putchar('.');
        }
    }
}

static void print_mac(const uint8_t* mac) {
    for (int i = 0; i < MAC_ADDR_LEN; i++) {
        uint8_t byte = mac[i];
        uint8_t high = (byte >> 4) & 0x0F;
        uint8_t low = byte & 0x0F;
        
        vga_putchar(high < 10 ? '0' + high : 'a' + high - 10);
        vga_putchar(low < 10 ? '0' + low : 'a' + low - 10);
        
        if (i < MAC_ADDR_LEN - 1) {
            vga_putchar(':');
        }
    }
}

void network_ping(const char* host) {
    vga_print("PING ");
    vga_print(host);
    vga_print(" (");
    
    // For demonstration, assume host resolves to a sample IP
    uint8_t target_ip[] = {8, 8, 8, 8};
    print_ip(target_ip);
    vga_print(")\n");
    
    // Simulate ping responses
    for (int i = 0; i < 4; i++) {
        vga_print("64 bytes from ");
        print_ip(target_ip);
        vga_print(": icmp_seq=");
        vga_putchar('0' + i + 1);
        vga_print(" time=");
        vga_putchar('0' + (i * 3 + 12) / 10);
        vga_putchar('0' + (i * 3 + 12) % 10);
        vga_print("ms\n");
    }
    
    vga_print("\n--- ");
    vga_print(host);
    vga_print(" ping statistics ---\n");
    vga_print("4 packets transmitted, 4 received, 0% packet loss\n");
}

void network_show_config(void) {
    vga_print("Network Interface Configuration:\n\n");
    
    vga_print("eth0: flags=UP,BROADCAST,RUNNING,MULTICAST  mtu 1500\n");
    vga_print("      inet ");
    print_ip(eth0.ip);
    vga_print("  netmask ");
    print_ip(eth0.netmask);
    vga_print("  broadcast ");
    uint8_t broadcast[] = {192, 168, 1, 255};
    print_ip(broadcast);
    vga_print("\n");
    
    vga_print("      ether ");
    print_mac(eth0.mac);
    vga_print("  txqueuelen 1000  (Ethernet)\n");
    
    vga_print("      RX packets 1234  bytes 567890 (554.5 KiB)\n");
    vga_print("      TX packets 987   bytes 123456 (120.5 KiB)\n\n");
    
    vga_print("lo: flags=UP,LOOPBACK,RUNNING  mtu 65536\n");
    vga_print("      inet 127.0.0.1  netmask 255.0.0.0\n");
    vga_print("      loop  txqueuelen 1000  (Local Loopback)\n");
}

void network_set_ip(const char* ip_str) {
    // Simple IP parsing (for demonstration)
    vga_print("Setting IP address to: ");
    vga_print(ip_str);
    vga_print("\n");
    
    // In a real implementation, this would parse the string and set the IP
    vga_print("IP address updated successfully\n");
}

int network_send_packet(const uint8_t* data, uint32_t size) {
    // Simulate packet sending
    (void)data;
    vga_print("Sending packet (");
    
    // Simple integer to string conversion for size
    if (size >= 1000) {
        vga_putchar('0' + (size / 1000));
        size %= 1000;
    }
    if (size >= 100) {
        vga_putchar('0' + (size / 100));
        size %= 100;
    }
    if (size >= 10) {
        vga_putchar('0' + (size / 10));
        size %= 10;
    }
    vga_putchar('0' + size);
    
    vga_print(" bytes)\n");
    return 1; // Success
}

int network_receive_packet(uint8_t* buffer, uint32_t max_size) {
    // Simulate packet reception
    (void)buffer;
    (void)max_size;
    return 0; // No packets available
}
