#include <sched.h>
#include <console.h>
#include <xen/xen.h>

#include <mini-os/mm.h>
#include <mini-os/gdt.h>
#include <mini-os/wait.h>

#include "sum.h" // generated by cgo

extern void _rt0_amd64_netbsd_lib(void);

static void initialize_go_thread(void *ctx) {
    unsigned long tls_page;
    unsigned int v;

    /* Setup TLS */
    tls_page = alloc_page();
    printk("Virtual address of TLS page: 0x%lx\n", tls_page);
    switch_fs(tls_page + PAGE_SIZE);

    *(int *)(tls_page + PAGE_SIZE - 8) = 9876;
    __asm__ __volatile__("mov %%fs:0xfffffffffffffff8, %0" : "=r"(v));
    if(v != 9876) {
        printk("Could not successfully set up TLS\n");
        *(char *)0 = 0;
    }

    /* Initialize runtime */
    printk("initialize_go_thread: initializing go\n");
    _rt0_amd64_netbsd_lib();
    printk("initialize_go_thread: go is initialized\n");

}

static void main_thread(void *ctx) {
    GoInt i;

    i = Sum(3, 4);
    printk("main_thread: result: %lld\n", i);
}

int app_main(start_info_t *si) {
    printk("go_main.c: app_main(%p)\n", si);
    create_thread("main", main_thread, NULL);
    create_thread("initialize_go", initialize_go_thread, NULL);
    return 0;
}
