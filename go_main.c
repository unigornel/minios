#include <sched.h>
#include <console.h>
#include <xen/xen.h>

#include "sum.h" // generated by cgo

int app_main(start_info_t *si) {
    GoInt i;

    printk("go_main.c: app_main(%p)\n", si);

    i = Sum(3, 4);
    printk("go_main.c: result: %lld\n", i);

    return 0;
}
