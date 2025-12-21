/* System call assembly interface */

.global syscall_handler
.global syscall_entry

.section .text

/* System call handler - called from interrupt */
syscall_handler:
    /* Save registers */
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    
    /* Call C system call dispatcher */
    /* Arguments are already in registers: eax=syscall, ebx=arg1, ecx=arg2, edx=arg3, esi=arg4 */
    pushl %esi      /* arg4 */
    pushl %edx      /* arg3 */
    pushl %ecx      /* arg2 */
    pushl %ebx      /* arg1 */
    pushl %eax      /* syscall number */
    call syscall_dispatch
    addl $20, %esp  /* Clean up stack */
    
    /* Return value is in eax */
    
    /* Restore registers */
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    
    /* Return from interrupt */
    iret

/* System call entry point for user programs */
syscall_entry:
    int $0x80  /* Trigger software interrupt 0x80 */
    ret
