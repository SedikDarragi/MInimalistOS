#include "../include/monitor.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/string.h"

// Monitoring test process
void monitor_test_process(void) {
    char msg[] = "MON Test: Monitoring system test started!\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    // Test logging
    monitor_log(LOG_LEVEL_INFO, LOG_SOURCE_PROCESS, "Monitor test: Starting logging tests");
    monitor_log(LOG_LEVEL_DEBUG, LOG_SOURCE_PROCESS, "Monitor test: Debug message");
    monitor_log(LOG_LEVEL_WARN, LOG_SOURCE_PROCESS, "Monitor test: Warning message");
    monitor_log(LOG_LEVEL_ERROR, LOG_SOURCE_PROCESS, "Monitor test: Error message");
    
    char log_msg[] = "MON Test: Logging tests completed!\n";
    syscall(SYS_WRITE, 1, (uint32_t)log_msg, sizeof(log_msg) - 1);
    
    // Test performance timers
    monitor_start_timer("test_operation");
    
    // Simulate some work
    for (volatile int i = 0; i < 100000; i++);
    
    monitor_end_timer("test_operation");
    
    uint32_t timer_value = monitor_get_timer_value("test_operation");
    if (timer_value > 0) {
        char timer_msg[] = "MON Test: Performance timer working!\n";
        syscall(SYS_WRITE, 1, (uint32_t)timer_msg, sizeof(timer_msg) - 1);
    }
    
    // Test system statistics
    system_stats_t stats;
    if (monitor_get_system_stats(&stats) == 0) {
        char stats_msg[] = "MON Test: System statistics retrieved!\n";
        syscall(SYS_WRITE, 1, (uint32_t)stats_msg, sizeof(stats_msg) - 1);
    }
    
    // Test performance metrics
    performance_metrics_t metrics;
    if (monitor_get_performance_metrics(&metrics) == 0) {
        char metrics_msg[] = "MON Test: Performance metrics retrieved!\n";
        syscall(SYS_WRITE, 1, (uint32_t)metrics_msg, sizeof(metrics_msg) - 1);
    }
    
    // Test log retrieval
    log_entry_t entry;
    if (monitor_get_log_entry(0, &entry) == 0) {
        char retrieve_msg[] = "MON Test: Log entry retrieved successfully!\n";
        syscall(SYS_WRITE, 1, (uint32_t)retrieve_msg, sizeof(retrieve_msg) - 1);
    }
    
    // Test system call logging
    syscall(SYS_LOG, LOG_LEVEL_INFO, (uint32_t)"MON Test: System call logging works!", 0);
    
    // Test statistics system call
    system_stats_t syscall_stats;
    if (syscall(SYS_GET_STATS, 0, (uint32_t)&syscall_stats, 0) == 0) {
        char syscall_stats_msg[] = "MON Test: Syscall statistics retrieved!\n";
        syscall(SYS_WRITE, 1, (uint32_t)syscall_stats_msg, sizeof(syscall_stats_msg) - 1);
    }
    
    // Periodic monitoring
    int counter = 0;
    while (1) {
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        counter++;
        
        // Every 10 iterations, log a status message
        if (counter % 10 == 0) {
            char status_msg[] = "MON Test: Monitoring process still running...\n";
            syscall(SYS_WRITE, 1, (uint32_t)status_msg, sizeof(status_msg) - 1);
            
            // Update monitoring stats
            monitor_update_stats();
        }
        
        // Every 50 iterations, dump some stats
        if (counter % 50 == 0) {
            monitor_dump_stats();
        }
        
        // Simple delay
        for (volatile int i = 0; i < 50000; i++);
    }
}
