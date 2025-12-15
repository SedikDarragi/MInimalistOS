#include "../include/ipc.h"
#include "process.h"
#include "string.h"

static ipc_msg_t message_queue[MAX_MESSAGES];
static int queue_head = 0;
static int queue_tail = 0;
static int queue_count = 0;

void ipc_init(void) {
    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;
}

int ipc_send(uint32_t receiver, uint8_t type, void* data, uint16_t len) {
    if (queue_count >= MAX_MESSAGES) {
        return -1;
    }
    
    if (len > MAX_MSG_SIZE) {
        return -2;
    }
    
    ipc_msg_t* msg = &message_queue[queue_tail];
    msg->sender = process_get_current()->pid;
    msg->receiver = receiver;
    msg->type = type;
    msg->length = len;
    
    if (data && len > 0) {
        memcpy(msg->data, data, len);
    }
    
    queue_tail = (queue_tail + 1) % MAX_MESSAGES;
    queue_count++;
    
    return 0;
}

int ipc_receive(uint32_t sender, ipc_msg_t* msg) {
    if (queue_count == 0) {
        return -1;
    }
    
    uint32_t current_pid = process_get_current()->pid;
    
    for (int i = 0; i < queue_count; i++) {
        int index = (queue_head + i) % MAX_MESSAGES;
        ipc_msg_t* candidate = &message_queue[index];
        
        if (candidate->receiver == current_pid && 
            (sender == 0 || candidate->sender == sender)) {
            
            memcpy(msg, candidate, sizeof(ipc_msg_t));
            
            // Remove message from queue
            for (int j = i; j < queue_count - 1; j++) {
                int src = (queue_head + j + 1) % MAX_MESSAGES;
                int dst = (queue_head + j) % MAX_MESSAGES;
                message_queue[dst] = message_queue[src];
            }
            
            queue_count--;
            return 0;
        }
    }
    
    return -1;
}
