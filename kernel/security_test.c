#include "../include/security.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "../include/string.h"

// Security test process
void security_test_process(void) {
    char msg[] = "SEC Test: Security system test started!\n";
    syscall(SYS_WRITE, 1, (uint32_t)msg, sizeof(msg) - 1);
    
    // Test user ID operations
    uint32_t uid = syscall(SYS_GETUID, 0, 0, 0);
    
    char uid_msg[] = "SEC Test: Current UID: 0000\n";
    // Simple hex conversion
    for (int i = 3; i >= 0; i--) {
        char nibble = (uid >> (i * 4)) & 0xF;
        uid_msg[21 + (3-i)] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
    }
    syscall(SYS_WRITE, 1, (uint32_t)uid_msg, sizeof(uid_msg) - 1);
    
    // Test permission checking
    int result = security_check_permission(PERM_READ);
    if (result == 0) {
        char perm_msg[] = "SEC Test: Read permission granted!\n";
        syscall(SYS_WRITE, 1, (uint32_t)perm_msg, sizeof(perm_msg) - 1);
    }
    
    // Test file access permission
    result = security_check_file_access("/dev/block0", PERM_READ);
    if (result == 0) {
        char file_msg[] = "SEC Test: File access granted!\n";
        syscall(SYS_WRITE, 1, (uint32_t)file_msg, sizeof(file_msg) - 1);
    }
    
    // Test device access permission
    result = security_check_device_access("block0", PERM_READ);
    if (result == 0) {
        char dev_msg[] = "SEC Test: Device access granted!\n";
        syscall(SYS_WRITE, 1, (uint32_t)dev_msg, sizeof(dev_msg) - 1);
    }
    
    // Test user creation (only root can do this)
    result = security_create_user("testuser", 1000, 1000);
    if (result == 0) {
        char user_msg[] = "SEC Test: User created successfully!\n";
        syscall(SYS_WRITE, 1, (uint32_t)user_msg, sizeof(user_msg) - 1);
        
        // Test user authentication
        result = security_authenticate_user("testuser", "");
        if (result == 0) {
            char auth_msg[] = "SEC Test: User authentication successful!\n";
            syscall(SYS_WRITE, 1, (uint32_t)auth_msg, sizeof(auth_msg) - 1);
        }
    }
    
    // Test permission setting
    result = security_set_permission("/test/file", 1000, 1000, PERM_READ | PERM_WRITE);
    if (result == 0) {
        char set_msg[] = "SEC Test: Permission set successfully!\n";
        syscall(SYS_WRITE, 1, (uint32_t)set_msg, sizeof(set_msg) - 1);
    }
    
    // Test context switching (only root can do this)
    if (uid == ROOT_UID) {
        result = security_set_context(1000, 1000);
        if (result == 0) {
            char ctx_msg[] = "SEC Test: Context switched to user 1000!\n";
            syscall(SYS_WRITE, 1, (uint32_t)ctx_msg, sizeof(ctx_msg) - 1);
            
            // Switch back to root
            security_set_context(ROOT_UID, ROOT_UID);
            char back_msg[] = "SEC Test: Context switched back to root!\n";
            syscall(SYS_WRITE, 1, (uint32_t)back_msg, sizeof(back_msg) - 1);
        }
    }
    
    while (1) {
        // Yield to other processes
        syscall(SYS_YIELD, 0, 0, 0);
        
        // Simple delay
        for (volatile int i = 0; i < 50000; i++);
    }
}
