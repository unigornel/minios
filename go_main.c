#include <sched.h>
#include <console.h>
#include <xen/xen.h>


int app_main(start_info_t *si) {
	printk("go_main.c: app_main(%p)\n", si);
	return 0;
}
