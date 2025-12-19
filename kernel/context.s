/* Context switching assembly functions - GAS syntax */

.global context_switch
.global context_init

.section .text

/* void context_switch(cpu_context_t* old_context, cpu_context_t* new_context) */
/* Switch from old_context to new_context */
context_switch:
    /* Save current context */
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    
    /* Save ESP */
    movl 32(%esp), %eax      /* Get old_context pointer */
    movl %esp, 28(%eax)      /* Save ESP */
    
    /* Save EIP (return address) */
    movl 32(%esp), %eax
    movl (%esp), %ebx         /* Get return address */
    movl %ebx, 32(%eax)       /* Save EIP */
    
    /* Save EFLAGS */
    pushfl
    popl %eax
    movl 36(%esp), %ebx      /* Get old_context pointer again */
    movl %eax, 36(%ebx)       /* Save EFLAGS */
    
    /* Load new context */
    movl 40(%esp), %eax      /* Get new_context pointer */
    
    /* Load ESP */
    movl 28(%eax), %esp
    
    /* Load EIP */
    pushl 32(%eax)
    
    /* Load EFLAGS */
    pushl 36(%eax)
    popfl
    
    /* Restore registers */
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    
    ret

/* void context_init(cpu_context_t* context, void (*entry_point)(), uint32_t stack_top) */
/* Initialize a context for a new process */
context_init:
    pushl %ebp
    movl %esp, %ebp
    
    /* Get parameters */
    movl 8(%ebp), %eax       /* context pointer */
    movl 12(%ebp), %ebx      /* entry_point */
    movl 16(%ebp), %ecx      /* stack_top */
    
    /* Clear context */
    pushl %edi
    pushl %eax
    movl %eax, %edi
    xorl %eax, %eax
    movl $10, %ecx            /* 10 dwords to clear */
    rep stosl
    popl %eax
    popl %edi
    
    /* Set up initial context */
    movl 12(%ebp), %ebx      /* entry_point */
    movl %ebx, 32(%eax)      /* eip = entry_point */
    movl 16(%ebp), %ecx      /* stack_top */
    movl %ecx, 28(%eax)      /* esp = stack_top */
    
    /* Set EFLAGS (interrupts enabled) */
    movl $0x202, 36(%eax)
    
    popl %ebp
    ret
