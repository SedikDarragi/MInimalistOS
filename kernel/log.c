#include "log.h"
#include "../include/string.h"
#include <stdarg.h>

// External declaration for itoa (defined in kernel/string.c)
extern char* itoa(int value, char* str, int base);

// Current log level
static log_level_t current_log_level = LOG_DEBUG;

// Initialize the logging system
void log_init(void) {
    vga_init();
    vga_clear();
    log_info("Kernel logging system initialized");
}

// Set the minimum log level to display
void log_set_level(log_level_t level) {
    current_log_level = level;
}

// Internal helper to print log level prefix
static void print_log_prefix(log_level_t level) {
    switch (level) {
        case LOG_DEBUG:
            vga_set_color(LOG_COLORS[level], VGA_COLOR_BLACK);
            vga_print("[DEBUG] ");
            break;
        case LOG_INFO:
            vga_set_color(LOG_COLORS[level], VGA_COLOR_BLACK);
            vga_print("[INFO]  ");
            break;
        case LOG_WARN:
            vga_set_color(LOG_COLORS[level], VGA_COLOR_BLACK);
            vga_print("[WARN]  ");
            break;
        case LOG_ERROR:
            vga_set_color(LOG_COLORS[level], VGA_COLOR_BLACK);
            vga_print("[ERROR] ");
            break;
        case LOG_CRITICAL:
            vga_set_color(LOG_COLORS[level], VGA_COLOR_RED);
            vga_print("[CRIT]  ");
            break;
        default:
            vga_print("[LOG]   ");
            break;
    }
}

// Minimal vsnprintf implementation for kernel logging
static int vsnprintf(char* buf, size_t size, const char* format, va_list args) {
    if (!buf || size == 0) return 0;
    
    size_t i = 0;
    const char* p = format;
    
    while (*p && i < size - 1) {
        if (*p != '%') {
            buf[i++] = *p++;
            continue;
        }
        
        // Handle format specifier
        p++; // skip '%'
        switch (*p) {
            case 'd': {
                int num = va_arg(args, int);
                char num_str[12];
                itoa(num, num_str, 10);
                for (char* s = num_str; *s && i < size - 1; s++) {
                    buf[i++] = *s;
                }
                p++;
                break;
            }
            case 'u': {
                unsigned int num = va_arg(args, unsigned int);
                char num_str[12];
                itoa(num, num_str, 10);
                for (char* s = num_str; *s && i < size - 1; s++) {
                    buf[i++] = *s;
                }
                p++;
                break;
            }
            case 'x': {
                unsigned int num = va_arg(args, unsigned int);
                char num_str[9];
                itoa(num, num_str, 16);
                for (char* s = num_str; *s && i < size - 1; s++) {
                    buf[i++] = *s;
                }
                p++;
                break;
            }
            case 'X': {
                unsigned int num = va_arg(args, unsigned int);
                char num_str[9];
                itoa(num, num_str, 16);
                // Convert to uppercase
                for (char* s = num_str; *s && i < size - 1; s++) {
                    buf[i++] = (*s >= 'a' && *s <= 'f') ? (*s - 'a' + 'A') : *s;
                }
                p++;
                break;
            }
            case 's': {
                const char* str = va_arg(args, const char*);
                if (!str) str = "(null)";
                while (*str && i < size - 1) {
                    buf[i++] = *str++;
                }
                p++;
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                buf[i++] = c;
                p++;
                break;
            }
            case '%': {
                buf[i++] = '%';
                p++;
                break;
            }
            case '\0':
                // End of format string
                break;
            default:
                // Unknown specifier, just copy as-is
                buf[i++] = '%';
                if (i < size - 1) buf[i++] = *p;
                p++;
                break;
        }
    }
    
    buf[i] = '\0';
    return i;
}

// Internal function to handle the actual logging with a va_list
static void log_vprintf(log_level_t level, const char* format, va_list args) {
    if (level < current_log_level) {
        return;
    }

    print_log_prefix(level);

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);

    vga_print(buffer);
    vga_print("\n");
}

// Public printf-style logging function
void log_printf(log_level_t level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_vprintf(level, format, args);
    va_end(args);
}

// Simple log functions (variadic wrappers)
void log_debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_vprintf(LOG_DEBUG, format, args);
    va_end(args);
}

void log_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_vprintf(LOG_INFO, format, args);
    va_end(args);
}

void log_warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_vprintf(LOG_WARN, format, args);
    va_end(args);
}

void log_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_vprintf(LOG_ERROR, format, args);
    va_end(args);
}

void log_critical(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_vprintf(LOG_CRITICAL, format, args);
    va_end(args);
}
