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
#include <mini-os/list.h>
#include <mini-os/xmalloc.h>
#include <mini-os/lib.h>
#include <mini-os/wait.h>
#include <mini-os/semaphore.h>

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

struct console_data_chunk {
    char *buffer;
    char *offset;
    unsigned remaining;
    MINIOS_STAILQ_ENTRY(struct console_data_chunk) entries;
};
static MINIOS_STAILQ_HEAD(, struct console_data_chunk) console_data = MINIOS_STAILQ_HEAD_INITIALIZER(console_data);
static DECLARE_WAIT_QUEUE_HEAD(console_wq);
static int console_wq_has_data;
static DECLARE_MUTEX(console_data_chunk_mutex);
#define LOCK_CONSOLE_DATA() down(&console_data_chunk_mutex);
#define UNLOCK_CONSOLE_DATA() up(&console_data_chunk_mutex);

void console_input(char *buf, unsigned len)
{
    struct console_data_chunk *chunk;

    LOCK_CONSOLE_DATA();
    chunk = malloc(sizeof(*chunk));
    chunk->buffer = malloc(len);
    chunk->offset = chunk->buffer;
    chunk->remaining = len;
    memcpy(chunk->buffer, buf, len);

    MINIOS_STAILQ_INSERT_TAIL(&console_data, chunk, entries);
    UNLOCK_CONSOLE_DATA();

    console_wq_has_data = 1;
    wake_up(&console_wq);
}

int32_t sys_read(int32_t fd, void *p, int32_t n)
{
    int32_t read = 0;
    char *dest = (char *)p;

    ASSERT(fd == 0, "expected fd to be 0, got %d\n", fd);

    while(read < n) {
        struct console_data_chunk *chunk;
        int32_t will_read;

        LOCK_CONSOLE_DATA();
        chunk = MINIOS_STAILQ_FIRST(&console_data);
        if(chunk == NULL && read == 0) {
            console_wq_has_data = 0;
            UNLOCK_CONSOLE_DATA();
            wait_event(console_wq, console_wq_has_data);
            continue;

        } else if(chunk == NULL) {
            UNLOCK_CONSOLE_DATA();
            break;
        }

        will_read = (chunk->remaining < (unsigned)n) ? (int32_t)chunk->remaining : n;
        memcpy(&dest[read], chunk->offset, will_read);
        chunk->offset += will_read;
        chunk->remaining -= (unsigned)will_read;
        read += will_read;

        if(chunk->remaining == 0) {
            free(chunk->buffer);
            free(chunk);
            MINIOS_STAILQ_REMOVE_HEAD(&console_data, entries);
        }
        UNLOCK_CONSOLE_DATA();
    }

    return read;
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

void _unimplemented_syscall(void) {
    CRASH("_unimplemented_syscall was called")
}
