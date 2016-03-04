/*
 * crash.h
 *
 * Helper functions to crash the guest.
 */
#ifndef _CRASH_H_
#define _CRASH_H_

#include <mini-os/console.h>

#define CRASH(fmt, args...) { \
    struct sched_shutdown sched_shutdown = { .reason = SHUTDOWN_crash }; \
    printk("crash: %s, line %d: " fmt "\n", __FILE__, __LINE__, ##args); \
    HYPERVISOR_sched_op(SCHEDOP_shutdown, &sched_shutdown); \
}

#define ASSERT(condition, fmt, args...) { \
    if(!(condition)) { \
        CRASH(fmt, ##args); \
    } \
}

#endif /* _CRASH_H_ */
