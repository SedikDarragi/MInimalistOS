#include "../include/network.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/string.h"

// Network test process - sender
void network_test_sender(void) {
    char msg[] = "NET Test: Network sender started!\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    int count = 0;
    while (1) {
        // Create test message
        char test_msg[] = "Network packet #0 from sender";
        test_msg[16] = '0' + (count % 10);
        
        // Send packet to destination IP
        uint32_t dst_ip = 0xC0A80101;  // 192.168.1.1
        int result = syscall(SYS_NETWORK_SEND, dst_ip, PACKET_TYPE_UDP, (uint32_t)test_msg);
        
        if (result == 0) {
            char success_msg[] = "NET Test: Packet sent successfully!\n";
            syscall(SYS_WRITE, 1, (uint32_t)success_msg, sizeof(success_msg) - 1);
        }
        
        count++;
        
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 60000; i++);
    }
}

// Network test process - receiver
void network_test_receiver(void) {
    char msg[] = "NET Test: Network receiver started!\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    int packets_received = 0;
    while (1) {
        network_packet_t packet;
        
        // Try to receive a packet
        int result = syscall(SYS_NETWORK_RECEIVE, (uint32_t)&packet, 0, 0);
        if (result == 0) {
            packets_received++;
            
            char recv_msg[] = "NET Test: Received packet! Count: 0\n";
            recv_msg[36] = '0' + (packets_received % 10);
            syscall(SYS_WRITE, 1, (uint32_t)recv_msg, sizeof(recv_msg) - 1);
            
            // Display packet content
            syscall(SYS_WRITE, 1, (uint32_t)packet.data, packet.length);
            
            char newline[] = "\n";
            syscall(SYS_WRITE, 1, (uint32_t)newline, 1);
        }
        
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 40000; i++);
    }
}
