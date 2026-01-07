#include "net_ne2k.h"
#include "../kernel/log.h"
#include "../kernel/io.h"
#include "../include/pci.h"

// NOTE: This is structured as a NE2000 driver but currently does not
// perform real hardware I/O. It is safe to keep in-tree while we
// incrementally add actual RX/TX support.

static net_device_t ne2k_dev;

static uint32_t ne2k_tx_packets = 0;
static uint32_t ne2k_tx_bytes = 0;
static uint32_t ne2k_poll_calls = 0;

// NE2000 / 8390 register layout (placeholders for future use)
// When we add real I/O, these will be used together with a detected
// base I/O port from PCI/ISA probing.
enum {
    NE2K_CR   = 0x00, // Command register
    NE2K_ISR  = 0x07, // Interrupt status
    NE2K_IMR  = 0x0F, // Interrupt mask
    NE2K_RBCR0 = 0x0A,
    NE2K_RBCR1 = 0x0B,
    NE2K_RSAR0 = 0x08,
    NE2K_RSAR1 = 0x09,
    NE2K_TPSR  = 0x04,
    NE2K_TBCR0 = 0x05,
    NE2K_TBCR1 = 0x06,
};

// Stubbed hardware init: currently just logs and succeeds.
// We now detect the device via PCI and log BAR0 and IRQ, but still
// do not touch the NIC registers.
static int ne2k_hw_init(void) {
    pci_device_t dev;

    // QEMU ne2k_pci emulates an NE2000-compatible card. Its vendor/device
    // IDs are implementation-defined; for many emulations, 0x10EC:0x8029
    // (Realtek RTL-8029) is used. If this does not match your setup, we
    // will simply log a warning and keep the driver as a no-op.
    const uint16_t NE2K_VENDOR_ID = 0x10EC; // Realtek
    const uint16_t NE2K_DEVICE_ID = 0x8029; // RTL-8029 (NE2000 compatible)

    if (pci_find_device(NE2K_VENDOR_ID, NE2K_DEVICE_ID, &dev) != 0) {
        log_warn("NE2000 PCI device not found (vendor=0x%04x device=0x%04x)", NE2K_VENDOR_ID, NE2K_DEVICE_ID);
        return -1;
    }

    log_info("NE2000 PCI: bus=%u slot=%u func=%u bar0=0x%08x irq=%u",
             dev.bus, dev.slot, dev.function, dev.bar0, dev.irq_line);

    // For now we keep I/O base and IRQ only for future use; we still do not
    // interact with the NIC registers so behavior remains safe.
    ne2k_dev.driver_data = 0;

    return 0;
}

static int ne2k_send(struct net_device* dev, const void* buf, uint32_t len) {
    (void)dev;
    (void)buf;
    ne2k_tx_packets++;
    ne2k_tx_bytes += len;
    return (int)len;
}

static void ne2k_poll(struct net_device* dev) {
    (void)dev;
    ne2k_poll_calls++;
}

int net_ne2k_init(void) {
    if (ne2k_hw_init() != 0) {
        log_error("NE2000 hardware init failed");
        return -1;
    }

    for (int i = 0; i < 6; ++i) {
        ne2k_dev.mac[i] = 0;
    }
    ne2k_dev.mtu = 1500;
    ne2k_dev.name = "eth0";
    ne2k_dev.driver_data = 0;
    ne2k_dev.send = ne2k_send;
    ne2k_dev.poll = ne2k_poll;

    if (net_register_device(&ne2k_dev) != 0) {
        log_error("Failed to register NE2000 network device");
        return -1;
    }

    log_info("NE2000 network device skeleton initialized");
    return 0;
}
