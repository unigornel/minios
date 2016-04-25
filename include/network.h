/**
 * network.h
 *
 * Minimal communication with the hypervisor to enable networking.
 */
#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <mini-os/types.h>

enum {
    kEtherTypeIPv4 = 0x0800,
    kEtherTypeARP = 0x0806,
};

struct eth_packet {
    char destination[6];
    uint16_t ether_type;
    unsigned char *payload;
    unsigned int payload_length;
};

void init_network(void);
void send_packet(struct eth_packet *p);
void get_mac_address(char mac[6]);

#endif /* _NETWORK_H_ */
