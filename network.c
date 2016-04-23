/**
 * network.c
 *
 * Minimal communication with the hypervisor to enable networking.
 */
#include <mini-os/network.h>

#include <mini-os/console.h>
#include <mini-os/netfront.h>
#include <mini-os/xmalloc.h>
#include <mini-os/crash.h>
#include <mini-os/lib.h>

#define MAX_ETH_PAYLOAD_SIZE 1500

static struct netfront_dev *network_device;
static unsigned char network_mac[6];

void init_network(void)
{
    unsigned char mac[6];
    char *ip;

    network_device = init_netfront(NULL, NULL, mac, &ip);
    printk("init_network: MAC %x:%x:%x:%x:%x:%x\n",
        (int)mac[0], (int)mac[1], (int)mac[2],
        (int)mac[3], (int)mac[4], (int)mac[5]);
    printk("init_network: IP %s\n", ip);
    free(ip);
    memcpy(network_mac, mac, sizeof(network_mac));
}

void send_packet(struct eth_packet *p)
{
    int raw_length;
    struct {
        char dmac[6];
        char smac[6];
        uint16_t ether_type;
        unsigned char payload[MAX_ETH_PAYLOAD_SIZE];
    } __attribute__((packed)) raw;

    ASSERT(p->payload_length <= MAX_ETH_PAYLOAD_SIZE, "ethernet payload too large");

    memcpy(raw.smac, network_mac, sizeof(raw.smac));
    memcpy(raw.dmac, p->destination, sizeof(raw.dmac));
    memcpy(raw.payload, p->payload, p->payload_length);
    raw.ether_type = p->ether_type;

    raw_length = sizeof(raw) - sizeof(raw.payload) + p->payload_length;
    netfront_xmit(network_device, (unsigned char *)&raw, raw_length);
}
