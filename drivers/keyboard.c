#include "keyboard.h"
#include "../kernel/log.h"

static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_to_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;
static int caps_lock = 0;
static int num_lock = 0;
static int scroll_lock = 0;

// LED status bits
#define LED_SCROLL_LOCK 0x01
#define LED_NUM_LOCK    0x02
#define LED_CAPS_LOCK   0x04

void keyboard_init(void) {
    // Enable keyboard
    outb(KEYBOARD_STATUS_PORT, 0xAE);
}

int keyboard_available(void) {
    return inb(KEYBOARD_STATUS_PORT) & 0x01;
}

// Update keyboard LEDs
static void keyboard_update_leds(void) {
    uint8_t leds = 0;
    if (scroll_lock) leds |= LED_SCROLL_LOCK;
    if (num_lock) leds |= LED_NUM_LOCK;
    if (caps_lock) leds |= LED_CAPS_LOCK;
    
    // Send LED command
    outb(KEYBOARD_DATA_PORT, 0xED);
    while (inb(KEYBOARD_STATUS_PORT) & 0x02); // Wait for input buffer empty
    outb(KEYBOARD_DATA_PORT, leds);
}

char keyboard_getchar(void) {
    uint8_t scancode;
    
    while (!keyboard_available());
    
    scancode = inb(KEYBOARD_DATA_PORT);
    
    // Handle modifier keys
    switch (scancode) {
        case 0x2A: // Left shift pressed
        case 0x36: // Right shift pressed
            shift_pressed = 1;
            return 0;
        case 0xAA: // Left shift released
        case 0xB6: // Right shift released
            shift_pressed = 0;
            return 0;
        case 0x1D: // Left ctrl pressed
            ctrl_pressed = 1;
            return 0;
        case 0x9D: // Left ctrl released
            ctrl_pressed = 0;
            return 0;
        case 0x38: // Left alt pressed
            alt_pressed = 1;
            return 0;
        case 0xB8: // Left alt released
            alt_pressed = 0;
            return 0;
        case 0x3A: // Caps lock
            caps_lock = !caps_lock;
            keyboard_update_leds();
            return 0;
        case 0x45: // Num lock
            num_lock = !num_lock;
            keyboard_update_leds();
            return 0;
        case 0x46: // Scroll lock
            scroll_lock = !scroll_lock;
            keyboard_update_leds();
            return 0;
    }
    
    // Ignore key releases (high bit set)
    if (scancode & 0x80) {
        return 0;
    }
    
    // Convert scancode to ASCII
    if (scancode < sizeof(scancode_to_ascii)) {
        char base_char = shift_pressed ? scancode_to_ascii_shift[scancode] : scancode_to_ascii[scancode];
        
        // Apply caps lock to letters
        if (caps_lock && base_char >= 'a' && base_char <= 'z') {
            base_char -= 32; // Convert to uppercase
        } else if (caps_lock && base_char >= 'A' && base_char <= 'Z') {
            base_char += 32; // Convert to lowercase
        }
        
        return base_char;
    }
    
    return 0;
}

// Check if a key is currently pressed
int keyboard_is_pressed(int scancode) {
    return inb(KEYBOARD_DATA_PORT) == scancode;
}

// Get keyboard status
void keyboard_get_status(int* shift, int* ctrl, int* alt, int* caps, int* num, int* scroll) {
    if (shift) *shift = shift_pressed;
    if (ctrl) *ctrl = ctrl_pressed;
    if (alt) *alt = alt_pressed;
    if (caps) *caps = caps_lock;
    if (num) *num = num_lock;
    if (scroll) *scroll = scroll_lock;
}
