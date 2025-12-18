#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "../kernel/io.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

void keyboard_init(void);
char keyboard_getchar(void);
int keyboard_available(void);
int keyboard_is_pressed(int scancode);
void keyboard_get_status(int* shift, int* ctrl, int* alt, int* caps, int* num, int* scroll);

#endif
