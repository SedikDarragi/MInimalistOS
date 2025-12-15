#ifndef IPC_H
#define IPC_H

#include <stdint.h>

#define MAX_MSG_SIZE 256
#define MAX_MESSAGES 16

typedef struct {
    uint32_t sender;
    uint32_t receiver;
    uint8_t type;
    uint16_t length;
    uint8_t data[MAX_MSG_SIZE];
} ipc_msg_t;

void ipc_init(void);
int ipc_send(uint32_t receiver, uint8_t type, void* data, uint16_t len);
int ipc_receive(uint32_t sender, ipc_msg_t* msg);

#endif
