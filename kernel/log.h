#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include "../drivers/vga.h"

// Log levels
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARN = 2,
    LOG_ERROR = 3,
    LOG_CRITICAL = 4
} log_level_t;

// Log colors for different levels
static const vga_color_t LOG_COLORS[] = {
    VGA_COLOR_LIGHT_GREY,   // DEBUG
    VGA_COLOR_GREEN,        // INFO
    VGA_COLOR_BROWN,        // WARN
    VGA_COLOR_LIGHT_RED,    // ERROR
    VGA_COLOR_WHITE         // CRITICAL
};

// Log functions
void log_init(void);
void log_set_level(log_level_t level);
void log_printf(log_level_t level, const char* format, ...);
void log_debug(const char* message);
void log_info(const char* message);
void log_warn(const char* message);
void log_error(const char* message);
void log_critical(const char* message);

// Panic and assertion macros
#define panic(msg) do { \
    log_critical(msg); \
    log_critical("System halted due to panic!"); \
    asm volatile("cli; hlt"); \
} while(0)

#define assert(condition) do { \
    if (!(condition)) { \
        log_critical("Assertion failed: " #condition); \
        panic("Assertion failure"); \
    } \
} while(0)

#define KERNEL_ERROR(msg) do { \
    log_error(msg); \
    log_error("File: " __FILE__ ", Line: " __LINE__); \
} while(0)

#endif
