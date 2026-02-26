#include "../include/monitor.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/memory.h"
#include "string.h"

// Log buffer
#define LOG_BUFFER_SIZE 256
static log_entry_t log_buffer[LOG_BUFFER_SIZE];
static uint32_t log_index = 0;
static uint32_t log_count = 0;

// System statistics
static system_stats_t system_stats;
static performance_metrics_t performance_metrics;

// Performance timers
#define MAX_TIMERS 16
typedef struct {
    char name[32];
    uint32_t start_time;
    uint32_t total_time;
    int active;
} timer_entry_t;

static timer_entry_t timers[MAX_TIMERS];
static uint32_t uptime_seconds = 0;

// Simple timestamp counter
static uint32_t timestamp_counter = 0;

// Initialize monitoring system
void monitor_init(void) {
    // Clear log buffer
    memset(log_buffer, 0, sizeof(log_buffer));
    log_index = 0;
    log_count = 0;
    
    // Initialize system statistics
    memset(&system_stats, 0, sizeof(system_stats));
    system_stats.total_memory = 1024 * 1024; // 1MB total memory
    system_stats.free_memory = system_stats.total_memory;
    
    // Initialize performance metrics
    memset(&performance_metrics, 0, sizeof(performance_metrics));
    
    // Clear timers
    memset(timers, 0, sizeof(timers));
    
    // Log initialization
    monitor_log(LOG_LEVEL_INFO, LOG_SOURCE_KERNEL, "Monitoring system initialized");
}

// Log a message
void monitor_log(uint8_t level, uint8_t source, const char* message) {
    if (!message) return;
    
    // Get current timestamp
    timestamp_counter++;
    
    // Create log entry
    log_entry_t* entry = &log_buffer[log_index];
    entry->timestamp = timestamp_counter;
    entry->level = level;
    entry->source = source;
    
    // Copy message (truncate if necessary)
    uint32_t msg_len = strlen(message);
    if (msg_len > 255) msg_len = 255;
    entry->length = msg_len;
    strncpy(entry->message, message, msg_len);
    entry->message[msg_len] = '\0';
    
    // Update log index
    log_index = (log_index + 1) % LOG_BUFFER_SIZE;
    if (log_count < LOG_BUFFER_SIZE) {
        log_count++;
    }
    
    // Update statistics
    system_stats.system_calls_count++;
}

// Printf-style logging (simplified)
void monitor_log_printf(uint8_t level, uint8_t source, const char* format, ...) {
    // For simplicity, just log the format string
    monitor_log(level, source, format);
}

// Get log entry
int monitor_get_log_entry(uint32_t index, log_entry_t* entry) {
    if (!entry || index >= log_count) return -1;
    
    uint32_t actual_index = (log_index - log_count + index) % LOG_BUFFER_SIZE;
    memcpy(entry, &log_buffer[actual_index], sizeof(log_entry_t));
    return 0;
}

// Get system statistics
int monitor_get_system_stats(system_stats_t* stats) {
    if (!stats) return -1;
    
    monitor_update_stats();
    memcpy(stats, &system_stats, sizeof(system_stats_t));
    return 0;
}

// Get performance metrics
int monitor_get_performance_metrics(performance_metrics_t* metrics) {
    if (!metrics) return -1;
    
    monitor_update_stats();
    memcpy(metrics, &performance_metrics, sizeof(performance_metrics_t));
    return 0;
}

// Update system statistics
void monitor_update_stats(void) {
    // Update uptime
    uptime_seconds++;
    
    // Update process counts (simplified)
    system_stats.total_processes = 8; // Approximate
    system_stats.running_processes = 3;
    system_stats.sleeping_processes = 5;
    
    // Update memory usage (simplified)
    system_stats.used_memory = system_stats.total_memory - system_stats.free_memory;
    
    // Update CPU time
    system_stats.total_cpu_time = uptime_seconds * 100;
    system_stats.idle_cpu_time = system_stats.total_cpu_time / 4; // 25% idle
    
    // Calculate performance metrics
    performance_metrics.cpu_usage_percent = 
        ((system_stats.total_cpu_time - system_stats.idle_cpu_time) * 100) / 
        (system_stats.total_cpu_time + 1);
    
    performance_metrics.memory_usage_percent = 
        (system_stats.used_memory * 100) / system_stats.total_memory;
    
    performance_metrics.process_count = system_stats.total_processes;
    performance_metrics.uptime_seconds = uptime_seconds;
    performance_metrics.context_switches = system_stats.system_calls_count / 10;
    performance_metrics.page_faults = system_stats.interrupts_count / 20;
}

// Simple snprintf implementation
static int snprintf(char* str, size_t size, const char* format, ...) {
    // Simplified implementation - just copy the format string
    if (!str || !format || size == 0) return 0;
    
    size_t i = 0;
    for (i = 0; i < size - 1 && format[i] != '\0'; i++) {
        str[i] = format[i];
    }
    str[i] = '\0';
    
    return i;
}

// Dump all logs to console
void monitor_dump_logs(void) {
    char header[] = "=== SYSTEM LOG DUMP ===\n";
    syscall(SYS_WRITE, 1, (uint32_t)header, sizeof(header) - 1);
    
    for (uint32_t i = 0; i < log_count; i++) {
        log_entry_t entry;
        if (monitor_get_log_entry(i, &entry) == 0) {
            char log_line[300];
            int len = snprintf(log_line, sizeof(log_line), 
                "[%u] Level:%d Source:%d - %s\n", 
                entry.timestamp, entry.level, entry.source, entry.message);
            syscall(SYS_WRITE, 1, (uint32_t)log_line, len);
        }
    }
    
    char footer[] = "=== END LOG DUMP ===\n";
    syscall(SYS_WRITE, 1, (uint32_t)footer, sizeof(footer) - 1);
}

// Dump system statistics
void monitor_dump_stats(void) {
    monitor_update_stats();
    
    char header[] = "=== SYSTEM STATISTICS ===\n";
    syscall(SYS_WRITE, 1, (uint32_t)header, sizeof(header) - 1);
    
    char stats_line[200];
    int len;
    
    len = snprintf(stats_line, sizeof(stats_line), 
        "Processes: %d total, %d running, %d sleeping\n",
        system_stats.total_processes, system_stats.running_processes, 
        system_stats.sleeping_processes);
    syscall(SYS_WRITE, 1, (uint32_t)stats_line, len);
    
    len = snprintf(stats_line, sizeof(stats_line), 
        "Memory: %d KB total, %d KB used, %d KB free\n",
        system_stats.total_memory / 1024, system_stats.used_memory / 1024,
        system_stats.free_memory / 1024);
    syscall(SYS_WRITE, 1, (uint32_t)stats_line, len);
    
    len = snprintf(stats_line, sizeof(stats_line), 
        "CPU: %d%% usage, %d syscalls, %d interrupts\n",
        performance_metrics.cpu_usage_percent,
        system_stats.system_calls_count, system_stats.interrupts_count);
    syscall(SYS_WRITE, 1, (uint32_t)stats_line, len);
    
    len = snprintf(stats_line, sizeof(stats_line), 
        "Uptime: %d seconds, %d context switches\n",
        uptime_seconds, performance_metrics.context_switches);
    syscall(SYS_WRITE, 1, (uint32_t)stats_line, len);
    
    char footer[] = "=== END STATISTICS ===\n";
    syscall(SYS_WRITE, 1, (uint32_t)footer, sizeof(footer) - 1);
}

// Start performance timer
void monitor_start_timer(const char* name) {
    if (!name) return;
    
    // Find existing timer or create new one
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].name[0] == '\0' || strcmp(timers[i].name, name) == 0) {
            strncpy(timers[i].name, name, 31);
            timers[i].name[31] = '\0';
            timers[i].start_time = timestamp_counter;
            timers[i].active = 1;
            break;
        }
    }
}

// End performance timer
void monitor_end_timer(const char* name) {
    if (!name) return;
    
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].active && strcmp(timers[i].name, name) == 0) {
            uint32_t elapsed = timestamp_counter - timers[i].start_time;
            timers[i].total_time += elapsed;
            timers[i].active = 0;
            break;
        }
    }
}

// Get timer value
uint32_t monitor_get_timer_value(const char* name) {
    if (!name) return 0;
    
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (strcmp(timers[i].name, name) == 0) {
            return timers[i].total_time;
        }
    }
    
    return 0;
}
