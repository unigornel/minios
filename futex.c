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

typedef struct _futex futex_t;
typedef MINIOS_TAILQ_HEAD(, futex_t) futex_queue_t;

struct _futex {
    uint32_t *address;
    struct wait_queue_head wq;
    int woken;

    MINIOS_TAILQ_ENTRY(futex_t) entries;
};

static struct {
    futex_queue_t queue;
} futexes;

int32_t sys_futex_wait(uint32_t *addr, uint32_t val, int64_t ns)
{
    if(*addr == val) {
        futex_t *f;
        s_time_t deadline;

        f = malloc(sizeof(*f));
        f->address = addr;
        f->woken = 0;
        init_waitqueue_head(&f->wq);

        MINIOS_TAILQ_INSERT_TAIL(&futexes.queue, f, entries);

        deadline = (ns ? NOW() + ns : 0);
        wait_event_deadline(f->wq, f->woken, deadline);

        free(f);
    }

    return 0;
}

int32_t sys_futex_wake(uint32_t *addr, uint32_t max)
{
    futex_t *f;
    int32_t count = 0;

    for(f = futexes.queue.tqh_first; f != NULL && count < max; f = f->entries.tqe_next) {
        if(f->address == addr) {
            futex_t *g;

            count++;
            f->woken = 1;
            wake_up(&f->wq);

            g = *f->entries.tqe_prev;
            MINIOS_TAILQ_REMOVE(&futexes.queue, f, entries);
            f = g;
        }
    }

    return count;
}

void init_futex(void)
{
    MINIOS_TAILQ_INIT(&futexes.queue);
}
