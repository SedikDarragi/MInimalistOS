#include "../include/power.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/string.h"

// Power management test process
void power_test_process(void) {
    char msg[] = "PWR Test: Power management test started!\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    // Test power state management
    uint32_t current_state = power_get_state();
    if (current_state == POWER_STATE_ON) {
        char state_msg[] = "PWR Test: Power state is ON\n";
        syscall(SYS_WRITE, 1, (uint32_t)state_msg, sizeof(state_msg) - 1);
    }
    
    // Test battery status
    battery_status_t battery;
    if (power_get_battery_status(&battery) == 0) {
        char battery_msg[] = "PWR Test: Battery status retrieved!\n";
        syscall(SYS_WRITE, 1, (uint32_t)battery_msg, sizeof(battery_msg) - 1);
        
        if (battery.present) {
            char present_msg[] = "PWR Test: Battery is present\n";
            syscall(SYS_WRITE, 1, (uint32_t)present_msg, sizeof(present_msg) - 1);
            
            // Display battery percentage
            char perc_msg[] = "PWR Test: Battery: 000%\n";
            int percentage = battery.percentage;
            perc_msg[17] = '0' + (percentage / 100);
            perc_msg[18] = '0' + ((percentage % 100) / 10);
            perc_msg[19] = '0' + (percentage % 10);
            syscall(SYS_WRITE, 1, (uint32_t)perc_msg, sizeof(perc_msg) - 1);
        }
    }
    
    // Test power statistics
    power_stats_t stats;
    if (power_get_statistics(&stats) == 0) {
        char stats_msg[] = "PWR Test: Power statistics retrieved!\n";
        syscall(SYS_WRITE, 1, (uint32_t)stats_msg, sizeof(stats_msg) - 1);
        
        // Display CPU frequency
        char freq_msg[] = "PWR Test: CPU: 0000MHz\n";
        uint32_t freq = stats.cpu_frequency;
        freq_msg[13] = '0' + (freq / 1000);
        freq_msg[14] = '0' + ((freq % 1000) / 100);
        freq_msg[15] = '0' + ((freq % 100) / 10);
        freq_msg[16] = '0' + (freq % 10);
        syscall(SYS_WRITE, 1, (uint32_t)freq_msg, sizeof(freq_msg) - 1);
        
        // Display thermal state
        char thermal_msg[] = "PWR Test: Temp: 000°C\n";
        uint32_t temp = stats.thermal_state;
        thermal_msg[13] = '0' + (temp / 100);
        thermal_msg[14] = '0' + ((temp % 100) / 10);
        thermal_msg[15] = '0' + (temp % 10);
        syscall(SYS_WRITE, 1, (uint32_t)thermal_msg, sizeof(thermal_msg) - 1);
    }
    
    // Test CPU frequency scaling
    if (power_set_cpu_frequency(500) == 0) {
        char scale_msg[] = "PWR Test: CPU frequency scaled to 500MHz\n";
        syscall(SYS_WRITE, 1, (uint32_t)scale_msg, sizeof(scale_msg) - 1);
    }
    
    // Restore original frequency
    if (power_set_cpu_frequency(1000) == 0) {
        char restore_msg[] = "PWR Test: CPU frequency restored to 1000MHz\n";
        syscall(SYS_WRITE, 1, (uint32_t)restore_msg, sizeof(restore_msg) - 1);
    }
    
    // Test CPU throttling
    if (power_throttle_cpu(50) == 0) {
        char throttle_msg[] = "PWR Test: CPU throttled to 50%\n";
        syscall(SYS_WRITE, 1, (uint32_t)throttle_msg, sizeof(throttle_msg) - 1);
    }
    
    // Restore full speed
    if (power_throttle_cpu(100) == 0) {
        char full_msg[] = "PWR Test: CPU restored to 100%\n";
        syscall(SYS_WRITE, 1, (uint32_t)full_msg, sizeof(full_msg) - 1);
    }
    
    // Test fan control
    if (power_set_fan_speed(1500) == 0) {
        char fan_msg[] = "PWR Test: Fan speed set to 1500 RPM\n";
        syscall(SYS_WRITE, 1, (uint32_t)fan_msg, sizeof(fan_msg) - 1);
    }
    
    // Test sleep state (briefly)
    if (power_request_sleep(S1_STATE) == 0) {
        char sleep_msg[] = "PWR Test: Entered sleep state\n";
        syscall(SYS_WRITE, 1, (uint32_t)sleep_msg, sizeof(sleep_msg) - 1);
        
        // Brief delay
        for (volatile int i = 0; i < 10000; i++);
        
        // Wake up
        if (power_request_wake() == 0) {
            char wake_msg[] = "PWR Test: Woke from sleep state\n";
            syscall(SYS_WRITE, 1, (uint32_t)wake_msg, sizeof(wake_msg) - 1);
        }
    }
    
    // Test power events
    if (power_handle_event(POWER_EVENT_BATTERY_LOW) == 0) {
        char event_msg[] = "PWR Test: Handled battery low event\n";
        syscall(SYS_WRITE, 1, (uint32_t)event_msg, sizeof(event_msg) - 1);
    }
    
    // Test system calls
    battery_status_t syscall_battery;
    if (syscall(SYS_GET_BATTERY_INFO, (uint32_t)&syscall_battery, 0, 0) == 0) {
        char syscall_msg[] = "PWR Test: Syscall battery info works!\n";
        syscall(SYS_WRITE, 1, (uint32_t)syscall_msg, sizeof(syscall_msg) - 1);
    }
    
    power_stats_t syscall_stats;
    if (syscall(SYS_GET_POWER_STATS, (uint32_t)&syscall_stats, 0, 0) == 0) {
        char syscall_stats_msg[] = "PWR Test: Syscall power stats work!\n";
        syscall(SYS_WRITE, 1, (uint32_t)syscall_stats_msg, sizeof(syscall_stats_msg) - 1);
    }
    
    // Periodic power monitoring
    int counter = 0;
    while (1) {
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        counter++;
        
        // Every 20 iterations, check battery status
        if (counter % 20 == 0) {
            int battery_level = power_get_battery_percentage();
            if (battery_level < 20) {
                char low_msg[] = "PWR Test: Low battery warning!\n";
                syscall(SYS_WRITE, 1, (uint32_t)low_msg, sizeof(low_msg) - 1);
            }
        }
        
        // Every 50 iterations, update and display stats
        if (counter % 50 == 0) {
            power_update_statistics();
            char monitor_msg[] = "PWR Test: Power monitoring active...\n";
            syscall(SYS_WRITE, 1, (uint32_t)monitor_msg, sizeof(monitor_msg) - 1);
        }
        
        // Simple delay
        for (volatile int i = 0; i < 30000; i++);
    }
}
