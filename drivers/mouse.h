#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

// Mouse command bytes
#define MOUSE_CMD_SET_DEFAULTS 0xF6
#define MOUSE_CMD_ENABLE_PACKET_STREAMING 0xF4
#define MOUSE_CMD_SET_SAMPLE_RATE 0xF3
#define MOUSE_CMD_GET_DEVICE_ID 0xF2
#define MOUSE_CMD_SET_REMOTE_MODE 0xF0
#define MOUSE_CMD_READ_DATA 0xEB
#define MOUSE_CMD_RESET 0xFF

// Mouse packet bits
#define MOUSE_LEFT_BUTTON 0x01
#define MOUSE_RIGHT_BUTTON 0x02
#define MOUSE_MIDDLE_BUTTON 0x04
#define MOUSE_X_SIGN 0x10
#define MOUSE_Y_SIGN 0x20
#define MOUSE_X_OVERFLOW 0x40
#define MOUSE_Y_OVERFLOW 0x80

// Mouse data port
#define MOUSE_DATA 0x60
#define MOUSE_STATUS 0x64
#define MOUSE_CMD 0x64

// Mouse command responses
#define MOUSE_ACK 0xFA
#define MOUSE_TEST_PASSED 0xAA

// Function declarations
void mouse_init(void);
void mouse_install_handler(void (*handler)(uint8_t, int8_t, int8_t, int8_t));
void mouse_wait(uint8_t type);
void mouse_write(uint8_t data);
uint8_t mouse_read(void);
void mouse_handle_interrupt(void);

#endif // MOUSE_H
