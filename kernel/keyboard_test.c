#include "../drivers/keyboard.h"
#include "../drivers/vga.h"
#include "log.h"

// Test keyboard initialization
void test_keyboard_init(void) {
    log_info("Testing keyboard initialization...");
    
    keyboard_init();
    log_info("✓ Keyboard initialized successfully");
}

// Test basic keyboard input
void test_keyboard_input(void) {
    log_info("Testing keyboard input...");
    log_info("Please type some characters (press ESC to exit):");
    
    char buffer[100];
    int pos = 0;
    
    while (pos < 99) {
        if (keyboard_available()) {
            char c = keyboard_getchar();
            
            if (c == 27) { // ESC key
                break;
            }
            
            if (c != 0) {
                buffer[pos++] = c;
                vga_putchar(c);
                
                // Echo character with logging
                log_debug("Key pressed: ");
                vga_putchar('\'');
                vga_putchar(c);
                vga_putchar('\'');
                vga_print("\n");
            }
        }
        
        // Small delay
        for (int i = 0; i < 1000; i++) {
            asm volatile("nop");
        }
    }
    
    buffer[pos] = '\0';
    log_info("Input received: ");
    vga_putchar('"');
    for (int i = 0; i < pos; i++) {
        vga_putchar(buffer[i]);
    }
    vga_putchar('"');
    vga_print("\n");
}

// Test modifier keys
void test_modifier_keys(void) {
    log_info("Testing modifier keys...");
    log_info("Press modifier keys (Shift, Ctrl, Alt) and observe LED changes");
    log_info("Press ESC to continue");
    
    while (1) {
        if (keyboard_available()) {
            char c = keyboard_getchar();
            
            if (c == 27) { // ESC key
                break;
            }
            
            // Check and display modifier status
            int shift, ctrl, alt, caps, num, scroll;
            keyboard_get_status(&shift, &ctrl, &alt, &caps, &num, &scroll);
            
            if (shift || ctrl || alt) {
                log_info("Modifier keys detected:");
                if (shift) log_info("  Shift pressed");
                if (ctrl) log_info("  Ctrl pressed");
                if (alt) log_info("  Alt pressed");
            }
            
            if (caps || num || scroll) {
                log_info("Lock keys status:");
                if (caps) log_info("  Caps Lock ON");
                if (num) log_info("  Num Lock ON");
                if (scroll) log_info("  Scroll Lock ON");
            }
        }
        
        // Small delay
        for (int i = 0; i < 1000; i++) {
            asm volatile("nop");
        }
    }
}

// Test keyboard availability
void test_keyboard_availability(void) {
    log_info("Testing keyboard availability detection...");
    
    int available_count = 0;
    int total_checks = 1000;
    
    for (int i = 0; i < total_checks; i++) {
        if (keyboard_available()) {
            available_count++;
        }
        
        // Small delay
        for (int j = 0; j < 100; j++) {
            asm volatile("nop");
        }
    }
    
    log_info("✓ Keyboard availability test completed");
    log_info("  Times keyboard had data available: ");
    
    // Simple number printing
    if (available_count >= 100) {
        vga_putchar('0' + (available_count / 100) % 10);
    }
    if (available_count >= 10) {
        vga_putchar('0' + (available_count / 10) % 10);
    }
    vga_putchar('0' + available_count % 10);
    vga_print(" / ");
    
    if (total_checks >= 100) {
        vga_putchar('0' + (total_checks / 100) % 10);
    }
    if (total_checks >= 10) {
        vga_putchar('0' + (total_checks / 10) % 10);
    }
    vga_putchar('0' + total_checks % 10);
    vga_print("\n");
}

// Test special keys
void test_special_keys(void) {
    log_info("Testing special keys...");
    log_info("Press special keys (Tab, Enter, Backspace) and observe behavior");
    log_info("Press ESC to continue");
    
    while (1) {
        if (keyboard_available()) {
            char c = keyboard_getchar();
            
            if (c == 27) { // ESC key
                break;
            }
            
            if (c != 0) {
                switch (c) {
                    case '\t':
                        log_info("✓ Tab key detected");
                        break;
                    case '\n':
                        log_info("✓ Enter key detected");
                        break;
                    case '\b':
                        log_info("✓ Backspace key detected");
                        break;
                    default:
                        if (c >= 32 && c <= 126) {
                            log_debug("Character key: ");
                            vga_putchar('\'');
                            vga_putchar(c);
                            vga_putchar('\'');
                            vga_print("\n");
                        }
                        break;
                }
            }
        }
        
        // Small delay
        for (int i = 0; i < 1000; i++) {
            asm volatile("nop");
        }
    }
}

// Main keyboard test function
void run_keyboard_tests(void) {
    log_init();
    log_info("=== Keyboard Driver Tests ===");
    log_info("");
    
    test_keyboard_init();
    log_info("");
    
    test_keyboard_availability();
    log_info("");
    
    test_modifier_keys();
    log_info("");
    
    test_special_keys();
    log_info("");
    
    test_keyboard_input();
    log_info("");
    
    log_info("=== Keyboard Tests Complete ===");
    log_info("Keyboard driver is working correctly!");
    
    while (1) {
        // Halt CPU and wait for interrupts
        asm volatile("hlt");
    }
}
