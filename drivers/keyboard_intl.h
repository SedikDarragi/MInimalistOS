#ifndef KEYBOARD_INTL_H
#define KEYBOARD_INTL_H

#include <stdint.h>

// Keyboard state structure
typedef struct {
    uint8_t shift_pressed;
    uint8_t ctrl_pressed;
    uint8_t alt_pressed;
    uint8_t altgr_pressed;
    uint8_t caps_lock;
    uint8_t num_lock;
    uint8_t scroll_lock;
    int current_layout;
} keyboard_state_t;

// Function declarations
int keyboard_intl_init(void);
int keyboard_intl_set_layout(int layout_index);
const char* keyboard_intl_get_layout_name(void);
int keyboard_intl_get_layout_count(void);
const char* keyboard_intl_get_layout_name_by_index(int index);
char keyboard_intl_getchar(void);
int keyboard_intl_available(void);
void keyboard_intl_get_state(keyboard_state_t* state);
void keyboard_intl_set_leds(uint8_t caps, uint8_t num, uint8_t scroll);

// Layout constants
#define KEYBOARD_LAYOUT_US     0
#define KEYBOARD_LAYOUT_DE     1
#define KEYBOARD_LAYOUT_FR     2

#endif // KEYBOARD_INTL_H
