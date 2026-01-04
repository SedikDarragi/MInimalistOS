#include "net_ne2k.h"
#include "../kernel/log.h"
#include "../kernel/io.h"

static net_device_t ne2k_dev;

static uint32_t ne2k_tx_packets = 0;
static uint32_t ne2k_tx_bytes = 0;
static uint32_t ne2k_poll_calls = 0;

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
