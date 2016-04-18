/**
 * network.c
 *
 * Minimal communication with the hypervisor to enable networking.
 */
#include <mini-os/network.h>

#include <mini-os/console.h>
#include <mini-os/netfront.h>
#include <mini-os/xmalloc.h>

void init_network(void)
{
    unsigned char mac[6];
    char *ip;

    init_netfront(NULL, NULL, mac, &ip);
    printk("init_network: MAC %x:%x:%x:%x:%x:%x\n",
        (int)mac[0], (int)mac[1], (int)mac[2],
        (int)mac[3], (int)mac[4], (int)mac[5]);
    printk("init_network: IP %s\n", ip);
    free(ip);
}
