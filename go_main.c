#include <console.h>
#include <xen/xen.h>

#include <mini-os/mm.h>
#include <mini-os/gdt.h>
#include <mini-os/wait.h>
#include <mini-os/go_pthread.h>
#include <mini-os/crash.h>
#include <mini-os/network.h>

extern void _rt0_amd64_unigornel_lib(void);
extern void Main(long long);

DECLARE_WAIT_QUEUE_HEAD(network_wq);
static int network_initialized = 0;

static void *initialize_go_thread(void *ctx) {
    wait_event(network_wq, network_initialized);
    _rt0_amd64_unigornel_lib();
    return NULL;
}

#include <mini-os/lib.h>
#include <mini-os/sched.h>
static void test_network(void) {
    char payload[] = "Hello, World!";
    struct eth_packet p;

    memset(&p.destination, ~0, sizeof(p.destination));
    p.ether_type = (uint16_t)strlen(payload);
    p.payload = (unsigned char *)payload;
    p.payload_length = (unsigned int)p.ether_type;

    while(1) {
        printk("sending packet\n");
        send_packet(&p);
        msleep(100);
    }
}

static void *initialize_network_thread(void *ctx) {
    init_network();
    test_network();
    network_initialized = 1;
    wake_up(&network_wq);
    return NULL;
}

static void *main_thread(void *ctx) {
    wait_event(network_wq, network_initialized);
    Main(0);
    CRASH("main thread must not return");
    return NULL;
}

int app_main(start_info_t *si) {
    pthread_t t;

    printk("go_main.c: app_main(%p)\n", si);
    pthread_create_name(&t, "main", NULL, main_thread, NULL);
    pthread_create_name(&t, "initialize_network", NULL, initialize_network_thread, NULL);
    pthread_create_name(&t, "initialize_go", NULL, initialize_go_thread, NULL);
    return 0;
}
