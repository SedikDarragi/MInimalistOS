#include "../include/ipc.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/string.h"

// IPC test process 1 - sender
void ipc_test_sender(void) {
    char msg[] = "IPC Test: Sender process started!\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    int count = 0;
    while (1) {
        // Send message to receiver
        char test_msg[] = "Hello from sender! Count: 0";
        test_msg[24] = '0' + (count % 10);
        
        int result = ipc_send(3, 1, test_msg, strlen(test_msg));
        if (result == 0) {
            char success_msg[] = "IPC Test: Message sent successfully!\n";
            syscall(SYS_WRITE, 1, (uint32_t)success_msg, sizeof(success_msg) - 1);
        }
        
        count++;
        
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 50000; i++);
    }
}

// IPC test process 2 - receiver
void ipc_test_receiver(void) {
    char msg[] = "IPC Test: Receiver process started!\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    int messages_received = 0;
    while (1) {
        ipc_msg_t received_msg;
        
        // Try to receive message from any sender (sender = 0)
        int result = ipc_receive(0, &received_msg);
        if (result == 0) {
            messages_received++;
            
            char recv_msg[] = "IPC Test: Received message! Count: 0\n";
            recv_msg[39] = '0' + (messages_received % 10);
            syscall(SYS_WRITE, 1, (uint32_t)recv_msg, sizeof(recv_msg) - 1);
            
            // Display the received message content
            syscall(SYS_WRITE, 1, (uint32_t)received_msg.data, received_msg.length);
            
            char newline[] = "\n";
            syscall(SYS_WRITE, 1, (uint32_t)newline, 1);
        }
        
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 30000; i++);
    }
}
