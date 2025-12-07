#ifndef IPC_H
#define IPC_H

#include <stdint.h>

// IPC message types
#define IPC_MSG_TEXT      1
#define IPC_MSG_DATA      2
#define IPC_MSG_SIGNAL    3
#define IPC_MSG_REQUEST   4
#define IPC_MSG_RESPONSE  5

// IPC message structure
typedef struct {
    uint32_t sender_pid;
    uint32_t receiver_pid;
    uint8_t type;
    uint8_t priority;
    uint16_t length;
    uint8_t data[256];  // Message payload
} ipc_message_t;

// IPC mailbox structure
typedef struct {
    ipc_message_t messages[16];  // Message queue
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    uint32_t owner_pid;
} ipc_mailbox_t;

// IPC system calls
#define IPC_SEND      1
#define IPC_RECEIVE   .   2++; 2 1   1  1 .  .  . .    .
#define IPC                .   .  .  . . .   .  .X.    .  .   .   .  .   . .  .  .  .   .   .  .  .   .
#define .   .  .  .   .  .   .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   . .  .EMOVE  3
.
#define IPC.   . |
#define IPC .   . . .  .   ..
#define .   .   .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .   .  .   .  .   .  .   .  .   .  .   .  .   .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .  .   .   .  .   .  .   .  .   .  .   .  .   .  .   .   .  .   .   .  .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .   .   .  .;   .Sync  4
#define IPC .
#define .  . . . . . ”; .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .OUS  5

// IPC functions
void ipc_init(void);
int ipc_send(uint32_t receiver_pid, uint8_t type, const void* data, uint16_t length);
int ipc_receive(uint32_t sender_pid, ipc_message_t* msg);
int ipc_mailbox_create(uint32_t pid);
int ipc_mailbox_destroy(uint32_t pid);
ipc_mailbox_t* ipc_get_mailbox(uint32_t pid);

// System call wrappers
uint32_t sys_ipc_send(uint32_t receiver_pid, uint8_t type, const void* data, uint16_t length);
uint32_t sys_ipc_receive(uint32_t sender_pid, ipc_message_t* msg);
uint32_t sys_ipc_mailbox_create(uint32_t pid);
uint32_t sys_ipc_mailbox_destroy(uint32_t pid);

#endif
