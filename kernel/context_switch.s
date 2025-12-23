/* Context switching assembly functions */

.global context_switch
.global context_init

.section .text

/* context_switch: Switch from old_context to new_context */
/* void context_switch(context_t* old_context, context_t* new_context); */
context_switch:
    pushl %ebp
    movl %esp, %ebp
    
    /* Save current context to old_context */
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    
    /* Save ESP and EIP */
    movl 8(%ebp), %eax      /* Get old_context pointer */
    movl %esp, 28(%eax)      /* Save ESP */
    movl 4(%ebp), %ebx       /* Get return address */
    movl %ebx, 32(%eax)      /* Save EIP */
    
    /* Save EFLAGS */
    pushfl
    popl %ebx
    movl %ebx, 36(%eax)      /* Save EFLAGS */
    
    /* Load new context */
    movl 12(%ebp), %eax      /* Get new_context pointer */
    movl 28(%eax), %esp      /* Load ESP */
    
    /* Load EIP and EFLAGS */
    pushl 36(%eax)            /* Push EFLAGS */
    popfl                     /* Load EFLAGS */
    pushl 32(%eax)            /* Push EIP */
    
    /* Restore registers */
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    
    popl %ebp
    ret

/* context_init: Initialize a new context */
/* void context_init(context_t* context, void* entry_point, void* stack_top); */
context_init:
    pushl %ebp
    movl %esp, %ebp
    
    /* Get parameters */
    movl 8(%ebp), %eax       /* context pointer */
    movl 12(%ebp), %ebx      /* entry_point */
    movl 16(%ebp), %ecx      /* stack_top */
    
    /* Clear context structure */
    pushl %edi
    pushl %eax
    movl %eax, %edi
    xorl %eax, %eax
    movl $10, %ecx           /* 10 dwords to clear */
    rep stosl
    popl %eax
    popl %edi
    
    /* Set up initial context */
    movl 12(%ebp), %ebx      /* entry_point */
    movl %ebx, 32(%eax)      /* eip = entry_point */
    movl 16(%ebp), %ecx      /* stack_top */
    movl %ecx, 28(%eax)      /* esp = stack_top */
    
    /* Set EFLAGS (interrupts enabled) */
    movl $0x202, 36(%eax)    /* EFLAGS */
    
    popl %ebp
    ret
