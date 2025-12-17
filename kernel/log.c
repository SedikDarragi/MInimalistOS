#include "log.h"
#include "../include/string.h"

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

// Simple printf-like function (very basic implementation)
void log_printf(log_level_t level, const char* format, ...) {
    if (level < current_log_level) {
        return;
    }
    
    print_log_prefix(level);
    
    // For now, just print the format string directly
    // In a full implementation, we'd parse format specifiers
    vga_print(format);
    vga_print("\n");
}

// Simple log functions
void log_debug(const char* message) {
    log_printf(LOG_DEBUG, message);
}

void log_info(const char* message) {
    log_printf(LOG_INFO, message);
}

void log_warn(const char* message) {
    log_printf(LOG_WARN, message);
}

void log_error(const char* message) {
    log_printf(LOG_ERROR, message);
}

void log_critical(const char* message) {
    log_printf(LOG_CRITICAL, message);
}
