/**
 * syscalls.c
 *
 * Implementation of various system calls needed by the Go runtime.
 */
#include <mini-os/syscalls.h>

#include <mini-os/console.h>
#include <mini-os/crash.h>
#include <mini-os/time.h>
#include <mini-os/sched.h>

int const sys_argc = 0;
char *const sys_argv[] = {
    NULL, // end of argv
    NULL, // end of envs
};

int32_t sys_write(uint64_t fd, void *p, int32_t n)
{
    if(fd != 1 && fd != 2) {
        CRASH("expected fd to be 1 or 2, got %lu\n", fd);
    }

    console_print(NULL, (char *)p, (int)n);

    return n;
}

uint64_t sys_nanotime(void)
{
    return monotonic_clock();
}

void sys_now(struct sys_now_t *now)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    now->sec = tv.tv_sec;
    now->nsec = (uint32_t)(tv.tv_usec * 1000);
}

uint64_t sys_thread_id(void)
{
    uint64_t current_thread;

    current_thread = (uint64_t)get_current();
    ASSERT(current_thread != 0, "sys_thread_id called on an invalid thread");

    return current_thread;
}

void sys_usleep(uint32_t usec)
{
    uint64_t nsec;
    nsec = (uint64_t)usec * 1000UL;
    sys_nanosleep(nsec);
}

void sys_nanosleep(uint64_t nsec)
{
    s_time_t stop;
    struct thread *thread;

    stop = NOW() + nsec;
    thread = get_current();

    while(stop > NOW()) {
        thread->wakeup_time = stop;
        clear_runnable(thread);
        schedule();
    }
}
