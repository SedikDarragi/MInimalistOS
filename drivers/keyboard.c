#include "keyboard.h"
#include "../kernel/log.h"
#include "../include/idt.h"

static const char scancode_to_ascii_azerty[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',
    0, 'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'u', '`',
    0, '*', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', 0,
    '*', 0, ' '
};

static const char scancode_to_ascii_azerty_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0, 0, '\n',
    0, 'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', '%', 0,
    0, '>', 'W', 'X', 'C', 'V', 'B', 'N', '?', '.', '/', 0, 0,
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

// Keyboard buffer
#define KB_BUFFER_SIZE 256
static char kb_buffer[KB_BUFFER_SIZE];
static volatile int kb_write_ptr = 0;
static volatile int kb_read_ptr = 0;

// Interrupt wrapper
static void keyboard_interrupt_handler(struct regs* r) {
    (void)r;
    keyboard_handler();
}

void keyboard_init(void) {
    // Explicitly disable the mouse interface on the PS/2 controller
    outb(KEYBOARD_STATUS_PORT, 0xA7);
    
    // Enable keyboard
    outb(KEYBOARD_STATUS_PORT, 0xAE);
    register_interrupt_handler(33, keyboard_interrupt_handler); // Register IRQ1 (INT 33)
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

// Interrupt handler called by ISR
void keyboard_handler(void) {
    // In an interrupt handler, we don't need to check the status port.
    // The IRQ itself tells us data is ready to be read from the data port.
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Handle modifier keys
    switch (scancode) {
        case 0x2A: // Left shift pressed
        case 0x36: // Right shift pressed
            shift_pressed = 1;
            return;
        case 0xAA: // Left shift released
        case 0xB6: // Right shift released
            shift_pressed = 0;
            return;
        case 0x1D: // Left ctrl pressed
            ctrl_pressed = 1;
            return;
        case 0x9D: // Left ctrl released
            ctrl_pressed = 0;
            return;
        case 0x38: // Left alt pressed
            alt_pressed = 1;
            return;
        case 0xB8: // Left alt released
            alt_pressed = 0;
            return;
        case 0x3A: // Caps lock
            caps_lock = !caps_lock;
            keyboard_update_leds();
            return;
        case 0x45: // Num lock
            num_lock = !num_lock;
            keyboard_update_leds();
            return;
        case 0x46: // Scroll lock
            scroll_lock = !scroll_lock;
            keyboard_update_leds();
            return;
    }
    
    // Ignore key releases (high bit set)
    if (scancode & 0x80) {
        return;
    }
    
    // Convert scancode to ASCII
    if (scancode < sizeof(scancode_to_ascii_azerty)) {
        char base_char = shift_pressed ? scancode_to_ascii_azerty_shift[scancode] : scancode_to_ascii_azerty[scancode];
        
        // Apply caps lock to letters
        if (caps_lock && base_char >= 'a' && base_char <= 'z') {
            base_char -= 32; // Convert to uppercase
        } else if (caps_lock && base_char >= 'A' && base_char <= 'Z') {
            base_char += 32; // Convert to lowercase
        }
        
        if (base_char != 0) {
            // Add to buffer
            int next_write = (kb_write_ptr + 1) % KB_BUFFER_SIZE;
            if (next_write != kb_read_ptr) { // Buffer not full
                kb_buffer[kb_write_ptr] = base_char;
                kb_write_ptr = next_write;
            }
        }
    }
}

char keyboard_getchar(void) {
    // Check if buffer is empty
    if (kb_read_ptr == kb_write_ptr) {
        return 0;
    }
    
    char c = kb_buffer[kb_read_ptr];
    kb_read_ptr = (kb_read_ptr + 1) % KB_BUFFER_SIZE;
    return c;
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
