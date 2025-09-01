#include "keyboard.h"

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

void keyboard_init(void) {
    // Enable keyboard
    outb(KEYBOARD_STATUS_PORT, 0xAE);
}

int keyboard_available(void) {
    return inb(KEYBOARD_STATUS_PORT) & 0x01;
}

char keyboard_getchar(void) {
    uint8_t scancode;
    
    while (!keyboard_available());
    
    scancode = inb(KEYBOARD_DATA_PORT);
    
    // Handle shift keys
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return 0;
    } else if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return 0;
    }
    
    // Ignore key releases (high bit set)
    if (scancode & 0x80) {
        return 0;
    }
    
    // Convert scancode to ASCII
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            return scancode_to_ascii_shift[scancode];
        } else {
            return scancode_to_ascii[scancode];
        }
    }
    
    return 0;
}
