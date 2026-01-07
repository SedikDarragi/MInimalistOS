#ifndef PCI_H
#define PCI_H

#include <stdint.h>

// PCI configuration space ports
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// Simple representation of a PCI device we're interested in
typedef struct pci_device {
    uint8_t  bus;
    uint8_t  slot;
    uint8_t  function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t bar0;      // Base Address Register 0 (usually I/O or MMIO base)
    uint8_t  irq_line;  // IRQ line from config space
} pci_device_t;

uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

// Find first device matching vendor/device on bus 0, function 0
// Returns 0 on success and fills out, -1 on failure.
int pci_find_device(uint16_t vendor, uint16_t device, pci_device_t* out);

#endif // PCI_H
