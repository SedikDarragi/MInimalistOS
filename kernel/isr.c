#include "../drivers/vga.h"
#include "io.h"
#include "log.h"
#include "../drivers/keyboard.h"

// ISR (Interrupt Service Routine) stubs
// This file is now deprecated in favor of assembly stubs in kernel/interrupts.s
// which call the generic handlers in kernel/idt.c.
// The previous C functions here caused system crashes because they used 'ret'
// instead of 'iret'.
