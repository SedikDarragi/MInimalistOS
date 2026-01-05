#include "net_ne2k.h"
#include "../kernel/log.h"
#include "../kernel/io.h"

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
// Later we will detect the device via PCI/ISA, reset it, and read MAC.
static int ne2k_hw_init(void) {
    log_info("NE2000 hw init stub (no real I/O yet)");
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
