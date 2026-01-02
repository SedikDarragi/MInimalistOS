#include "../include/syscall.h"

// Assembly entry point that triggers int 0x80 and returns with eax as result
extern int syscall_entry(void);

int syscall(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    int ret;
    __asm__ volatile (
        "movl %1, %%eax\n"
        "movl %2, %%ebx\n"
        "movl %3, %%ecx\n"
        "movl %4, %%edx\n"
        "call syscall_entry\n"
        "movl %%eax, %0\n"
        : "=r"(ret)
        : "r"(syscall_num), "r"(arg1), "r"(arg2), "r"(arg3)
        : "eax", "ebx", "ecx", "edx"
    );
    return ret;
}
