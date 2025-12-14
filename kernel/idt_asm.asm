; IDT loading function
; void idt_load(uint32_t idt_ptr);

global idt_load
extern idtp

section .text
idt_load:
    mov eax, [esp+4]    ; Get the pointer to the IDT
    lidt [eax]          ; Load the IDT
    ret                 ; Return to the caller
