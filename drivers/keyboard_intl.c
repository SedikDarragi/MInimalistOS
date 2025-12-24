#include "keyboard_intl.h"
#include "../kernel/io.h"
#include "../include/string.h"
#include "../drivers/vga.h"
#include "../kernel/log.h"

// Keyboard controller ports
#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT  0x64

// Keyboard controller commands
#define KEYBOARD_ENABLE_SCANNING  0xF4
#define KEYBOARD_DISABLE_SCANNING 0xF5
#define KEYBOARD_SET_LEDS        0xED

// International keyboard layouts
typedef struct {
    const char* name;
    const char* scancode_to_ascii;
    const char* scancode_to_ascii_shift;
    const char* scancode_to_ascii_altgr;
} keyboard_layout_t;

// US QWERTY layout
static const char us_qwerty[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char us_qwerty_shift[128] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// German QWERTZ layout (simplified - using ASCII chars)
static const char de_qwertz[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\'', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', '\'', '#', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '^', '`',
    0, '<', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char de_qwertz_shift[128] = {
    0, 0, '!', '"', '$', '%', '&', '/', '(', ')', '=', '?', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '"', '*', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
    0, '>', 'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char de_qwertz_altgr[128] = {
    0, 0, '2', '3', '{', '[', ']', '}', '\\', '\b',
    '\t', '@', 0, 0, 0, 0, 0, 0, 0, 0, 0, '~', '\b',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '`',
    0, '|', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// French AZERTY layout (simplified - using ASCII chars)
static const char fr_azerty[128] = {
    0, 0, '&', '1', '2', '\'', '(', '-', '4', '_', '5', '\b',
    '\t', 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',
    0, 'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', ';', ':',
    0, '*', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char fr_azerty_shift[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\b',
    '\t', 'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '*', '\b',
    0, 'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '%', '\b',
    0, 'W', 'W', 'X', 'C', 'V', 'B', 'N', '?', '.', '/', '\b',
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Available keyboard layouts
static const keyboard_layout_t keyboard_layouts[] = {
    {"US QWERTY", us_qwerty, us_qwerty_shift, 0},
    {"German QWERTZ", de_qwertz, de_qwertz_shift, de_qwertz_altgr},
    {"French AZERTY", fr_azerty, fr_azerty_shift, 0},
};

static const int num_layouts = sizeof(keyboard_layouts) / sizeof(keyboard_layout_t);

// Current keyboard state
static int current_layout = 0;
static uint8_t shift_pressed = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t alt_pressed = 0;
static uint8_t altgr_pressed = 0;
static uint8_t caps_lock = 0;
static uint8_t num_lock = 0;
static uint8_t scroll_lock = 0;

// Function declarations
static char handle_extended_keys(void);

// Initialize international keyboard driver
int keyboard_intl_init(void) {
    // Reset keyboard state
    shift_pressed = 0;
    ctrl_pressed = 0;
    alt_pressed = 0;
    altgr_pressed = 0;
    caps_lock = 0;
    num_lock = 0;
    scroll_lock = 0;
    current_layout = 0; // Default to US QWERTY
    
    // Enable keyboard scanning
    outb(KEYBOARD_DATA_PORT, KEYBOARD_ENABLE_SCANNING);
    
    log_info("International keyboard driver initialized");
    vga_print("Keyboard INTL: OK\n");
    return 0;
}

// Set keyboard layout
int keyboard_intl_set_layout(int layout_index) {
    if (layout_index < 0 || layout_index >= num_layouts) {
        return -1;
    }
    
    current_layout = layout_index;
    log_info("Keyboard layout changed");
    return 0;
}

// Get current keyboard layout name
const char* keyboard_intl_get_layout_name(void) {
    return keyboard_layouts[current_layout].name;
}

// Get number of available layouts
int keyboard_intl_get_layout_count(void) {
    return num_layouts;
}

// Get layout name by index
const char* keyboard_intl_get_layout_name_by_index(int index) {
    if (index < 0 || index >= num_layouts) {
        return NULL;
    }
    return keyboard_layouts[index].name;
}

// Update keyboard LEDs
static void keyboard_update_leds(void) {
    uint8_t leds = 0;
    
    if (caps_lock) leds |= 0x04;
    if (num_lock) leds |= 0x02;
    if (scroll_lock) leds |= 0x01;
    
    // Send LED command to keyboard
    outb(KEYBOARD_DATA_PORT, KEYBOARD_SET_LEDS);
    // Wait for keyboard to accept command
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    outb(KEYBOARD_DATA_PORT, leds);
}

// Handle special keys
static char handle_special_keys(uint8_t scancode) {
    switch (scancode) {
        case 0x2A: case 0x36: // Left/Right Shift
            shift_pressed = 1;
            return 0;
        case 0xAA: case 0xB6: // Shift release
            shift_pressed = 0;
            return 0;
        case 0x1D: // Ctrl
            ctrl_pressed = 1;
            return 0;
        case 0x9D: // Ctrl release
            ctrl_pressed = 0;
            return 0;
        case 0x38: // Alt
            alt_pressed = 1;
            return 0;
        case 0xB8: // Alt release
            alt_pressed = 0;
            return 0;
        case 0xE0: // Extended key prefix
            return handle_extended_keys();
        case 0x3A: // Caps Lock
            caps_lock = !caps_lock;
            keyboard_update_leds();
            return 0;
        case 0x45: // Num Lock
            num_lock = !num_lock;
            keyboard_update_leds();
            return 0;
        case 0x46: // Scroll Lock
            scroll_lock = !scroll_lock;
            keyboard_update_leds();
            return 0;
    }
    return 0;
}

// Handle extended keys (E0 prefix)
static char handle_extended_keys(void) {
    uint8_t extended_scancode = inb(KEYBOARD_DATA_PORT);
    
    switch (extended_scancode) {
        case 0x38: // AltGr
            altgr_pressed = 1;
            return 0;
        case 0xB8: // AltGr release
            altgr_pressed = 0;
            return 0;
    }
    return 0;
}

// Get character from scancode
char keyboard_intl_getchar(void) {
    uint8_t scancode;
    
    // Wait for key press
    while (!keyboard_intl_available());
    
    scancode = inb(KEYBOARD_DATA_PORT);
    
    // Handle special keys first
    char special = handle_special_keys(scancode);
    if (special != 0) {
        return special;
    }
    
    // Ignore key releases (bit 7 set)
    if (scancode & 0x80) {
        return 0;
    }
    
    if (scancode >= 128) {
        return 0;
    }
    
    // Get current layout
    const keyboard_layout_t* layout = &keyboard_layouts[current_layout];
    
    // Determine which character map to use
    char base_char = 0;
    
    if (altgr_pressed && layout->scancode_to_ascii_altgr) {
        base_char = layout->scancode_to_ascii_altgr[scancode];
    } else if (shift_pressed) {
        base_char = layout->scancode_to_ascii_shift[scancode];
    } else {
        base_char = layout->scancode_to_ascii[scancode];
    }
    
    // Apply caps lock for letters
    if (caps_lock && base_char >= 'a' && base_char <= 'z') {
        base_char -= 32;
    } else if (caps_lock && base_char >= 'A' && base_char <= 'Z') {
        base_char += 32;
    }
    
    return base_char;
}

// Check if keyboard input is available
int keyboard_intl_available(void) {
    return inb(KEYBOARD_STATUS_PORT) & 0x01;
}

// Get current keyboard state
void keyboard_intl_get_state(keyboard_state_t* state) {
    if (!state) return;
    
    state->shift_pressed = shift_pressed;
    state->ctrl_pressed = ctrl_pressed;
    state->alt_pressed = alt_pressed;
    state->altgr_pressed = altgr_pressed;
    state->caps_lock = caps_lock;
    state->num_lock = num_lock;
    state->scroll_lock = scroll_lock;
    state->current_layout = current_layout;
}

// Set keyboard LEDs manually
void keyboard_intl_set_leds(uint8_t caps, uint8_t num, uint8_t scroll) {
    caps_lock = caps;
    num_lock = num;
    scroll_lock = scroll;
    keyboard_update_leds();
}
