#include <console.h>
#include <xen/xen.h>

#include <mini-os/mm.h>
#include <mini-os/gdt.h>
#include <mini-os/wait.h>
#include <mini-os/go_pthread.h>
#include <mini-os/crash.h>

extern void _rt0_amd64_unigornel_lib(void);
extern void Main(long long);

static void *initialize_go_thread(void *ctx) {
    _rt0_amd64_unigornel_lib();
    return NULL;
}

static void *main_thread(void *ctx) {
    Main(0);
    CRASH("main thread must not return");
    return NULL;
}

int app_main(start_info_t *si) {
    pthread_t t;

    printk("go_main.c: app_main(%p)\n", si);
    pthread_create_name(&t, "main", NULL, main_thread, NULL);
    pthread_create_name(&t, "initialize_go", NULL, initialize_go_thread, NULL);
    return 0;
}
