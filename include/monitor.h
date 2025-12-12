#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>

// Log levels
#define LOG_LEVEL_DEBUG   0
#define LOG_LEVEL_INFO    1
#define LOG_LEVEL_WARN    2
#define LOG_LEVEL_ERROR   3
#define LOG_LEVEL_CRITICAL 4

// Log sources
#define LOG_SOURCE_KERNEL  0
#define LOG_SOURCE_PROCESS 1
#define LOG_SOURCE_MEMORY  2
#define LOG_SOURCE_DEVICE  3
#define LOG_SOURCE_NETWORK 4
#define LOG_SOURCE_SECURITY 5
#define LOG_SOURCE_FILESYSTEM 6

// Log entry structure
typedef struct {
    uint32_t timestamp;
    uint8_t level;
    uint8_t source;
    uint16_t length;
    char message[256];
} log_entry_t;

// System statistics
typedef struct {
    uint32_t total_processes;
    uint32_t running_processes;
    uint32_t sleeping_processes;
    uint32_t total_memory;
    uint32_t used_memory;
    uint32_t free_memory;
    uint32_t total_cpu_time;
    uint32_t idle_cpu_time;
    uint32_t system_calls_count;
    uint32_t interrupts_count;
} system_stats_t;

// Performance metrics
typedef struct {
    uint32_t cpu_usage_percent;
    uint32_t memory_usage_percent;
    uint32_t process_count;
    uint32_t uptime_seconds;
    uint32_t context_switches;
    uint32_t page_faults;
} performance_metrics_t;

// Monitoring functions
void monitor_init(void);
void monitor_log(uint8_t level, uint8_t source, const char* message);
void monitor_log_printf(uint8_t level, uint8_t source, const char* format, ...);
int monitor_get_log_entry(uint32_t index, log_entry_t* entry);
int monitor_get_system_stats(system_stats_t* stats);
int monitor_get_performance_metrics(performance_metrics_t* metrics);
void monitor_update_stats(void);
void monitor_dump_logs(void);
void monitor_dump_stats(void);

// Performance monitoring
void monitor_start_timer(const char* name);
void monitor_end_timer(const char* name);
uint32_t monitor_get_timer_value(const char* name);

// System call wrappers
uint32_t sys_log(uint8_t level, const char* message);
uint32_t sys_get_stats(uint32_t stats_type, void* buffer);
uint32_t sys_dump_logs(void);

#endif
