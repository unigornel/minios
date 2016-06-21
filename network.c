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
#include <mini-os/wait.h>
#include <mini-os/semaphore.h>

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
        unsigned char dmac[6];
        unsigned char smac[6];
        uint16_t ether_type;
        unsigned char payload[MAX_ETH_PAYLOAD_SIZE];
    } __attribute__((packed)) raw;

    ASSERT(p->payload_length <= MAX_ETH_PAYLOAD_SIZE, "ethernet payload too large");

    memcpy(raw.smac, network_mac, sizeof(raw.smac));
    memcpy(raw.dmac, p->destination, sizeof(raw.dmac));
    memcpy(raw.payload, p->payload, p->payload_length);
#ifdef __x86_64__
    raw.ether_type = ((p->ether_type >> 8) & 0x00FF) | ((p->ether_type << 8) & 0xFF00);
#else
#error "not implemented"
#endif

    raw_length = sizeof(raw) - sizeof(raw.payload) + p->payload_length;
    netfront_xmit(network_device, (unsigned char *)&raw, raw_length);
}

struct incoming_packet {
    unsigned char *data;
    int64_t length;
    MINIOS_STAILQ_ENTRY(struct incoming_packet) entries;
};
static MINIOS_STAILQ_HEAD(, struct incoming_packet) incoming_packets = MINIOS_STAILQ_HEAD_INITIALIZER(incoming_packets);
static DECLARE_WAIT_QUEUE_HEAD(incoming_wq);
static int incoming_wq_has_data;
static DECLARE_MUTEX(incoming_packets_mutex);
#define LOCK_INCOMING_PACKETS() down(&incoming_packets_mutex)
#define UNLOCK_INCOMING_PACKETS() up(&incoming_packets_mutex)

void netif_rx(unsigned char *data, int len)
{
    struct incoming_packet *packet;

    ASSERT(len >= 6 + 6 + 2, "ethernet packet too small");

    packet = malloc(sizeof(*packet));
    packet->data = malloc(len);
    packet->length = (int64_t)len;
    memcpy(packet->data, data, len);

    LOCK_INCOMING_PACKETS();
    MINIOS_STAILQ_INSERT_TAIL(&incoming_packets, packet, entries);
    UNLOCK_INCOMING_PACKETS();

    incoming_wq_has_data = 1;
    wake_up(&incoming_wq);
}

int64_t receive_packet(void *vdata, int64_t len)
{
    int64_t length;
    struct incoming_packet *packet;

    do {
        LOCK_INCOMING_PACKETS();
        packet = MINIOS_STAILQ_FIRST(&incoming_packets);
        if(packet == NULL) {
            incoming_wq_has_data = 0;
            UNLOCK_INCOMING_PACKETS();
            wait_event(incoming_wq, incoming_wq_has_data);
            continue;
        }
        MINIOS_STAILQ_REMOVE_HEAD(&incoming_packets, entries);
        UNLOCK_INCOMING_PACKETS();
    } while(packet == NULL);

    if(len < packet->length) {
        return -1;
    }

    length = packet->length;
    memcpy(vdata, packet->data, length);

    free(packet->data);
    free(packet);

    return length;
}

void get_mac_address(unsigned char mac[6])
{
    memcpy(mac, network_mac, 6);
}
