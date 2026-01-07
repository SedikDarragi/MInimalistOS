#include "../include/pci.h"
#include "io.h"
#include "log.h"

static uint32_t pci_config_address(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (uint32_t)(
        (1U << 31) |                   // Enable bit
        ((uint32_t)bus << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8) |
        (offset & 0xFC)
    );
}

uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr = pci_config_address(bus, slot, func, offset);
    outl(PCI_CONFIG_ADDRESS, addr);
    uint32_t data = inl(PCI_CONFIG_DATA);
    if (offset & 2) {
        return (uint16_t)(data >> 16);
    } else {
        return (uint16_t)(data & 0xFFFF);
    }
}

uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr = pci_config_address(bus, slot, func, offset);
    outl(PCI_CONFIG_ADDRESS, addr);
    return inl(PCI_CONFIG_DATA);
}

int pci_find_device(uint16_t vendor, uint16_t device, pci_device_t* out) {
    if (!out) return -1;

    // For simplicity, just scan bus 0, slots 0-31, function 0
    for (uint8_t slot = 0; slot < 32; ++slot) {
        uint16_t v = pci_read_config_word(0, slot, 0, 0x00); // Vendor ID
        if (v == 0xFFFF) {
            continue; // No device present
        }
        uint16_t d = pci_read_config_word(0, slot, 0, 0x02); // Device ID
        if (v == vendor && d == device) {
            out->bus       = 0;
            out->slot      = slot;
            out->function  = 0;
            out->vendor_id = v;
            out->device_id = d;

            uint32_t bar0 = pci_read_config_dword(0, slot, 0, 0x10);
            out->bar0 = bar0;

            uint32_t irq_data = pci_read_config_dword(0, slot, 0, 0x3C);
            out->irq_line = (uint8_t)(irq_data & 0xFF);

            log_info("PCI device found: bus=0 slot=%u vendor=0x%04x device=0x%04x", slot, v, d);
            return 0;
        }
    }

    log_warn("PCI device not found: vendor=0x%04x device=0x%04x", vendor, device);
    return -1;
}

void pci_dump_bus0(void) {
    for (uint8_t slot = 0; slot < 32; ++slot) {
        uint16_t v = pci_read_config_word(0, slot, 0, 0x00);
        if (v == 0xFFFF) {
            continue;
        }
        uint16_t d = pci_read_config_word(0, slot, 0, 0x02);
        log_info("PCI bus0 slot=%u vendor=0x%04x device=0x%04x", slot, v, d);
    }
}
