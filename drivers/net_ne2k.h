#ifndef NET_NE2K_H
#define NET_NE2K_H

#include <stdint.h>
#include "../include/net.h"

// Initialize NE2000-compatible PCI NIC (e.g., QEMU ne2k_pci)
int net_ne2k_init(void);

#endif // NET_NE2K_H
