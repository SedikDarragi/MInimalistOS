#ifndef POWER_H
#define POWER_H

#include <stdint.h>

// Power states
#define POWER_STATE_ON      0
#define POWER_STATE_SLEEP   1
#define POWER_STATE_HIBERNATE 2
#define POWER_STATE_OFF     3

// Sleep states (ACPI)
#define S0_STATE    0   // Working
#define S1_STATE    1   // Sleep 1
#define S3_STATE    3   // Sleep 3 (Suspend to RAM)
#define S4_STATE    4   // Sleep 4 (Suspend to Disk)
#define S5_STATE    5   // Soft Off

// Power management events
#define POWER_EVENT_NONE        0
#define POWER_EVENT_SLEEP       1
#define POWER_EVENT_WAKE        2
#define POWER_EVENT_BATTERY_LOW 3
#define POWER_EVENT_CRITICAL    4

// Battery status
typedef struct {
    uint32_t present;        // Battery present flag
    uint32_t charging;       // Charging flag
    uint32_t percentage;     // Charge percentage (0-100)
    uint32_t remaining_time; // Estimated remaining time (minutes)
    uint32_t voltage;        // Current voltage (mV)
    uint32_t current;        // Current draw (mA)
} battery_status_t;

// Power statistics
typedef struct {
    uint32_t uptime_seconds;
    uint32_t sleep_time_seconds;
    uint32_t power_consumption;  // Power consumption in mW
    uint32_t thermal_state;      // Thermal state (0-100°C)
    uint32_t cpu_frequency;      // Current CPU frequency in MHz
    uint32_t fan_speed;          // Fan speed in RPM
} power_stats_t;

// Power management functions
void power_init(void);
int power_set_state(uint32_t state);
uint32_t power_get_state(void);
int power_request_sleep(uint32_t sleep_state);
int power_request_wake(void);
int power_handle_event(uint32_t event);

// Battery management
int power_get_battery_status(battery_status_t* status);
int power_get_battery_percentage(void);
int power_is_battery_charging(void);
int power_estimate_remaining_time(void);

// Power statistics
int power_get_statistics(power_stats_t* stats);
void power_update_statistics(void);
void power_reset_statistics(void);

// CPU power management
int power_set_cpu_frequency(uint32_t frequency_mhz);
uint32_t power_get_cpu_frequency(void);
int power_throttle_cpu(uint32_t percentage);

// Thermal management
int power_get_thermal_state(void);
int power_set_fan_speed(uint32_t speed_rpm);
uint32_t power_get_fan_speed(void);

// System call wrappers (declared in syscall.h)

#endif
