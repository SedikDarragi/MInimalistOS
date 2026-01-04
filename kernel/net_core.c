#include "../include/net.h"
#include "log.h"

// Simple static registry of network devices
static net_device_t* net_devices[NET_MAX_DEVICES];
static uint32_t net_device_count = 0;

// Aggregate stats across all devices
static uint32_t net_rx_frames = 0;
static uint32_t net_rx_bytes  = 0;
static uint32_t net_tx_frames = 0;
static uint32_t net_tx_bytes  = 0;

void net_init(void) {
    for (uint32_t i = 0; i < NET_MAX_DEVICES; ++i) {
        net_devices[i] = 0;
    }
    net_device_count = 0;
    net_rx_frames = net_rx_bytes = 0;
    net_tx_frames = net_tx_bytes = 0;
    log_info("Network core initialized");
}

int net_register_device(net_device_t* dev) {
    if (!dev) {
        return -1;
    }
    if (net_device_count >= NET_MAX_DEVICES) {
        log_error("Cannot register network device: limit reached");
        return -1;
    }

    net_devices[net_device_count] = dev;
    net_device_count++;

    log_info("Network device registered");
    return 0;
}

void net_poll_all(void) {
    for (uint32_t i = 0; i < net_device_count; ++i) {
        net_device_t* dev = net_devices[i];
        if (dev && dev->poll) {
            dev->poll(dev);
        }
    }
}

uint32_t net_get_device_count(void) {
    return net_device_count;
}

net_device_t* net_get_device(uint32_t index) {
    if (index >= net_device_count) {
        return 0;
    }
    return net_devices[index];
}

// Entry point for NIC RX paths: record stats and (later) dispatch to protocols
void net_receive_frame(net_device_t* dev, const uint8_t* buf, uint32_t len) {
    (void)dev;
    (void)buf;
    if (len == 0) {
        return;
    }
    net_rx_frames++;
    net_rx_bytes += len;
    // TODO: once Ethernet/ARP/IPv4 layers exist, dispatch frame here
}

void net_get_stats(net_stats_t* out) {
    if (!out) return;
    out->rx_frames = net_rx_frames;
    out->rx_bytes  = net_rx_bytes;
    out->tx_frames = net_tx_frames;
    out->tx_bytes  = net_tx_bytes;
}
