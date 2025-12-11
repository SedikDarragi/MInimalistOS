#include "../include/security.h"
#include "../include/syscall.h"
#include "../include/vga.h"
#include "string.h"

// Current process security context
static security_context_t current_context;

// User database (simple implementation)
typedef struct {
    char username[32];
    uint32_t uid;
    uint32_t gid;
    char password[32];
    int active;
} user_entry_t;

static user_entry_t user_db[16];
static int user_count = 0;

// Access control list (simple implementation)
typedef struct {
    char resource[64];
    uint32_t uid;
    uint32_t gid;
    uint32_t permissions;
} acl_entry_extended_t;

static acl_entry_extended_t acl_table[64];
static int acl_count = 0;

// Initialize security system
void security_init(void) {
    // Initialize current context as root
    current_context.uid = ROOT_UID;
    current_context.gid = ROOT_UID;
    current_context.euid = ROOT_UID;
    current_context.egid = ROOT_UID;
    current_context.permissions = PERM_READ | PERM_WRITE | PERM_EXECUTE | PERM_DELETE | PERM_ADMIN;
    
    // Clear user database
    memset(user_db, 0, sizeof(user_db));
    user_count = 0;
    
    // Clear ACL table
    memset(acl_table, 0, sizeof(acl_table));
    acl_count = 0;
    
    // Create root user
    security_create_user("root", ROOT_UID, ROOT_UID);
    
    // Create default permissions for system resources
    security_set_permission("/dev", ROOT_UID, ROOT_UID, PERM_READ | PERM_WRITE | PERM_ADMIN);
    security_set_permission("/proc", ROOT_UID, ROOT_UID, PERM_READ | PERM_ADMIN);
    security_set_permission("/sys", ROOT_UID, ROOT_UID, PERM_READ | PERM_ADMIN);
}

// Set security context
int security_set_context(uint32_t uid, uint32_t gid) {
    // Only root can change security context
    if (current_context.uid != ROOT_UID) {
        return -1;
    }
    
    current_context.uid = uid;
    current_context.gid = gid;
    current_context.euid = uid;
    current_context.egid = gid;
    
    // Reset permissions based on new context
    if (uid == ROOT_UID) {
        current_context.permissions = PERM_READ | PERM_WRITE | PERM_EXECUTE | PERM_DELETE | PERM_ADMIN;
    } else {
        current_context.permissions = PERM_READ | PERM_EXECUTE;
    }
    
    return 0;
}

// Get security context
int security_get_context(security_context_t* ctx) {
    if (!ctx) return -1;
    
    memcpy(ctx, &current_context, sizeof(security_context_t));
    return 0;
}

// Check permission
int security_check_permission(uint32_t required_perm) {
    return (current_context.permissions & required_perm) == required_perm ? 0 : -1;
}

// Check file access permission
int security_check_file_access(const char* filename, uint32_t required_perm) {
    if (!filename) return -1;
    
    // Check ACL for this resource
    for (int i = 0; i < acl_count; i++) {
        if (strcmp(acl_table[i].resource, filename) == 0) {
            if (acl_table[i].uid == current_context.uid || 
                acl_table[i].gid == current_context.gid ||
                acl_table[i].uid == ROOT_UID) {
                return (acl_table[i].permissions & required_perm) == required_perm ? 0 : -1;
            }
        }
    }
    
    // Default: check current permissions
    return security_check_permission(required_perm);
}

// Check device access permission
int security_check_device_access(const char* devname, uint32_t required_perm) {
    if (!devname) return -1;
    
    // Only root can access devices directly
    if (current_context.uid != ROOT_UID) {
        return -1;
    }
    
    return security_check_permission(required_perm);
}

// Set permission for resource
int security_set_permission(const char* resource, uint32_t uid, uint32_t gid, uint32_t perm) {
    if (!resource || acl_count >= 64) return -1;
    
    // Only root can set permissions
    if (current_context.uid != ROOT_UID) {
        return -1;
    }
    
    // Check if entry already exists
    for (int i = 0; i < acl_count; i++) {
        if (strcmp(acl_table[i].resource, resource) == 0 &&
            acl_table[i].uid == uid && acl_table[i].gid == gid) {
            acl_table[i].permissions = perm;
            return 0;
        }
    }
    
    // Add new entry
    strncpy(acl_table[acl_count].resource, resource, 63);
    acl_table[acl_count].resource[63] = '\0';
    acl_table[acl_count].uid = uid;
    acl_table[acl_count].gid = gid;
    acl_table[acl_count].permissions = perm;
    acl_count++;
    
    return 0;
}

// Get permission for resource
int security_get_permission(const char* resource, uint32_t uid, uint32_t gid, uint32_t* perm) {
    if (!resource || !perm) return -1;
    
    for (int i = 0; i < acl_count; i++) {
        if (strcmp(acl_table[i].resource, resource) == 0 &&
            acl_table[i].uid == uid && acl_table[i].gid == gid) {
            *perm = acl_table[i].permissions;
            return 0;
        }
    }
    
    return -1;
}

// Create user
int security_create_user(const char* username, uint32_t uid, uint32_t gid) {
    if (!username || user_count >= 16) return -1;
    
    // Only root can create users
    if (current_context.uid != ROOT_UID) {
        return -1;
    }
    
    // Check if user already exists
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_db[i].username, username) == 0) {
            return -1;
        }
    }
    
    // Add new user
    strncpy(user_db[user_count].username, username, 31);
    user_db[user_count].username[31] = '\0';
    user_db[user_count].uid = uid;
    user_db[user_count].gid = gid;
    user_db[user_count].password[0] = '\0';  // No password initially
    user_db[user_count].active = 1;
    user_count++;
    
    return 0;
}

// Delete user
int security_delete_user(uint32_t uid) {
    // Only root can delete users
    if (current_context.uid != ROOT_UID) {
        return -1;
    }
    
    // Cannot delete root
    if (uid == ROOT_UID) {
        return -1;
    }
    
    // Find and deactivate user
    for (int i = 0; i < user_count; i++) {
        if (user_db[i].uid == uid) {
            user_db[i].active = 0;
            return 0;
        }
    }
    
    return -1;
}

// Set user password
int security_set_user_password(uint32_t uid, const char* password) {
    if (!password) return -1;
    
    // Only root can set passwords or user can set own password
    if (current_context.uid != ROOT_UID && current_context.uid != uid) {
        return -1;
    }
    
    // Find user and set password
    for (int i = 0; i < user_count; i++) {
        if (user_db[i].uid == uid && user_db[i].active) {
            strncpy(user_db[i].password, password, 31);
            user_db[i].password[31] = '\0';
            return 0;
        }
    }
    
    return -1;
}

// Authenticate user
int security_authenticate_user(const char* username, const char* password) {
    if (!username || !password) return -1;
    
    for (int i = 0; i < user_count; i++) {
        if (user_db[i].active && strcmp(user_db[i].username, username) == 0) {
            if (strcmp(user_db[i].password, password) == 0) {
                // Set security context to authenticated user
                security_set_context(user_db[i].uid, user_db[i].gid);
                return 0;
            }
        }
    }
    
    return -1;
}

// System call wrappers
uint32_t sys_setuid(uint32_t uid) {
    return security_set_context(uid, current_context.gid);
}

uint32_t sys_setgid(uint32_t gid) {
    return security_set_context(current_context.uid, gid);
}

uint32_t sys_getuid(void) {
    return current_context.uid;
}

uint32_t sys_getgid(void) {
    return current_context.gid;
}

uint32_t sys_chmod(const char* path, uint32_t mode) {
    return security_set_permission(path, current_context.uid, current_context.gid, mode);
}

uint32_t sys_chown(const char* path, uint32_t uid, uint32_t gid) {
    return security_set_permission(path, uid, gid, PERM_READ | PERM_WRITE | PERM_EXECUTE);
}
