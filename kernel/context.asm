; Context switching assembly functions

global context_switch
global context_init

section .text

; void context_switch(cpu_context_t* old_context, cpu_context_t* new_context)
; Switch from old_context to new_context
context_switch:
    ; Save current context
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    push    ebp
    
    ; Save ESP
    mov     eax, [esp+32]      ; Get old_context pointer
    mov     [eax+28], esp       ; Save ESP
    
    ; Save EIP (return address)
    mov     eax, [esp+32]
    mov     ebx, [esp]          ; Get return address
    mov     [eax+32], ebx       ; Save EIP
    
    ; Save EFLAGS
    pushfd
    pop     eax
    mov     ebx, [esp+36]      ; Get old_context pointer again
    mov     [ebx+36], eax       ; Save EFLAGS
    
    ; Load new context
    mov     eax, [esp+40]      ; Get new_context pointer
    
    ; Load ESP
    mov     esp, [eax+28]
    
    ; Load EIP
    push    dword [eax+32]
    
    ; Load EFLAGS
    push    dword [eax+36]
    popfd
    
    ; Restore registers
    pop     ebp
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    
    ret

; void context_init(cpu_context_t* context, void (*entry_point)(), uint32_t stack_top)
; Initialize a context for a new process
context_init:
    push    ebp
    mov     ebp, esp
    
    ; Get parameters
    mov     eax, [ebp+8]       ; context pointer
    mov     ebx, [ebp+12]      ; entry_point
    mov     ecx, [ebp+16]      ; stack_top
    
    ; Clear context
    push    edi
    push    eax
    mov     edi, eax
    xor     eax, eax
    mov     ecx, 10            ; 10 dwords to clear
    rep     stosd
    pop     eax
    pop     edi
    
    ; Set up initial context
    mov     ebx, [ebp+12]      ; entry_point
    mov     [eax+32], ebx      ; eip = entry_point
    mov     ecx, [ebp+16]      ; stack_top
    mov     [eax+28], ecx      ; esp = stack_top
    
    ; Set EFLAGS (interrupts enabled)
    mov     dword [eax+36], 0x202
    
    pop     ebp
    ret
