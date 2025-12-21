#include "../include/filesystem.h"
#include "../include/memory.h"
#include "../include/string.h"
#include "../drivers/vga.h"
#include "../kernel/log.h"

// Simple RAM filesystem

int ramfs_mount(void) {
    log_info("RAM filesystem mounted");
    vga_print("RAMFS: OK\n");
    return 1; // Success
}

int ramfs_init(void) {
    log_info("RAM filesystem initialized");
    return 0;
}
