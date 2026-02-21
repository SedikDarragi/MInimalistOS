[BITS 32]

global context_switch
global context_init

section .text

; void context_switch(cpu_context_t* old_context, cpu_context_t* new_context);
context_switch:
    ; Stack layout:
    ; [esp]      = return address (EIP)
    ; [esp + 4]  = old_context pointer
    ; [esp + 8]  = new_context pointer

    mov eax, [esp + 4]      ; Get old_context pointer

    ; Save registers to old_context
    mov [eax + 0], eax      ; Save EAX (contains old_context ptr, but restored later)
    mov [eax + 4], ebx
    mov [eax + 8], ecx
    mov [eax + 12], edx
    mov [eax + 16], esi
    mov [eax + 20], edi
    mov [eax + 24], ebp

    ; Save ESP (current stack pointer, which points to return address)
    mov [eax + 28], esp

    ; Save EIP (return address on stack)
    mov edx, [esp]
    mov [eax + 32], edx

    ; Save EFLAGS
    pushfd
    pop edx
    mov [eax + 36], edx

    ; Save CR3
    mov edx, cr3
    mov [eax + 40], edx

    ; --- Load new context ---
    mov eax, [esp + 8]      ; Get new_context pointer

    ; Load CR3
    mov edx, [eax + 40]
    mov ecx, cr3
    cmp edx, ecx
    je .same_cr3
    mov cr3, edx
.same_cr3:

    ; Load EFLAGS
    mov edx, [eax + 36]
    push edx
    popfd

    ; Load ESP
    mov esp, [eax + 28]

    ; Load EIP (overwrite return address on stack)
    mov edx, [eax + 32]
    mov [esp], edx

    ; Load registers
    mov ebp, [eax + 24]
    mov edi, [eax + 20]
    mov esi, [eax + 16]
    mov edx, [eax + 12]
    mov ecx, [eax + 8]
    mov ebx, [eax + 4]
    mov eax, [eax + 0]

    ret

; void context_init(cpu_context_t* context, void (*entry_point)(), uint32_t stack_top);
context_init:
    ; Stack layout:
    ; [esp]      = return address
    ; [esp + 4]  = context pointer
    ; [esp + 8]  = entry_point
    ; [esp + 12] = stack_top

    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]      ; context pointer
    mov ecx, [ebp + 12]     ; entry_point
    mov edx, [ebp + 16]     ; stack_top

    ; Initialize EIP
    mov [eax + 32], ecx

    ; Initialize ESP
    ; We set ESP to stack_top - 4.
    ; This simulates that a return address was pushed onto the stack.
    ; When context_switch performs 'ret', it will pop this 'address' (which we set to EIP).
    sub edx, 4
    mov [eax + 28], edx

    ; Initialize EFLAGS (Interrupts enabled: 0x202)
    mov dword [eax + 36], 0x202

    ; Initialize CR3 (Use current CR3)
    mov ecx, cr3
    mov [eax + 40], ecx

    ; Zero out general registers
    mov dword [eax + 0], 0
    mov dword [eax + 4], 0
    mov dword [eax + 8], 0
    mov dword [eax + 12], 0
    mov dword [eax + 16], 0
    mov dword [eax + 20], 0
    mov dword [eax + 24], 0

    pop ebp
    ret