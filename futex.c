/**
 * futex.c
 *
 * Minimal Linux-like futex implementation.
 */

#include <mini-os/futex.h>

#include <mini-os/list.h>
#include <mini-os/xmalloc.h>
#include <mini-os/wait.h>
#include <mini-os/time.h>
#include <mini-os/crash.h>


#define NUM_FUTEXES 10

typedef struct {
    uint32_t *address;
    struct wait_queue_head wq;
    volatile int woken;
} futex_t;

static futex_t futexes[NUM_FUTEXES];

int32_t sys_futex_wait(uint32_t *addr, uint32_t val, int64_t ns)
{
    if(*addr == val) {
        s_time_t deadline;
        futex_t *f = NULL;
        int i;

        for(i = 0; i < NUM_FUTEXES; i++) {
            if(futexes[i].address == NULL) {
                f = &futexes[i];
                break;
            }
        }

        ASSERT(f != NULL, "error: all futexes have been used");

        f->address = addr;
        f->woken = 0;
        init_waitqueue_head(&f->wq);

        deadline = (ns ? NOW() + ns : 0);
        wait_event_deadline(f->wq, f->woken != 0, deadline);
        f->address = NULL;
    }

    return 0;
}

int32_t sys_futex_wake(uint32_t *addr, uint32_t max)
{
    futex_t *f;
    int32_t count = 0;
    int i;

    for(i = 0; i < NUM_FUTEXES && count < max; i++) {
        f = &futexes[i];
        if(f->address == addr) {
            count++;
            f->woken = 1;
            wake_up(&f->wq);
        }
    }

    return count;
}

void init_futex(void)
{
}
