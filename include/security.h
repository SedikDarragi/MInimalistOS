#ifndef SECURITY_H
#define SECURITY_H

#include <stdint.h>

// User and group constants
#define ROOT_UID        0
#define MAX_UID         65535
#define MAX_GID         65535

// Process permissions
#define PERM_READ       0x001
#define PERM_WRITE      0x002
#define PERM_EXECUTE    0x004
#define PERM_DELETE     0x008
#define PERM_ADMIN      0x800

// Security context structure
typedef struct {
    uint32_t uid;        // User ID
    uint32_t gid;        // Group ID
    uint32_t euid;       // Effective user ID
    uint32_t egid;       // Effective group ID
    uint32_t permissions; // Current permissions
} security_context_t;

// Access control entry
typedef struct {
    uint32_t uid;        // User ID
    uint32_t gid;        // Group ID
    uint32_t permissions; // Allowed permissions
} acl_entry_t;

// Security functions
void security_init(void);
int security_set_context(uint32_t uid, uint32_t gid);
int security_get_context(security_context_t* ctx);
int security_check_permission(uint32_t required_perm);
int security_check_file_access(const char* filename, uint32_t required_perm);
int security_check_device_access(const char* devname, uint32_t required_perm);
int security_set_permission(const char* resource, uint32_t uid, uint32_t gid, uint32_t perm);
int security_get_permission(const char* resource, uint32_t uid, uint32_t gid, uint32_t* perm);

// User management
int security_create_user(const char* username, uint32_t uid, uint32_t gid);
int security_delete_user(uint32_t uid);
int security_set_user_password(uint32_t uid, const char* password);
int security_authenticate_user(const char* username, const char* password);

// System call wrappers
uint32_t sys_setuid(uint32_t uid);
uint32_t sys_setgid(uint32_t gid);
uint32_t sys_getuid(void);
uint32_t sys_getgid(void);
uint32_t sys_chmod(const char* path, uint32_t mode);
uint32_t sys_chown(const char* path, uint32_t uid, uint32_t gid);

#endif
