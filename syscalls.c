/**
 * syscalls.c
 *
 * Implementation of various system calls needed by the Go runtime.
 */
#include <mini-os/syscalls.h>

#include <mini-os/console.h>
#include <mini-os/crash.h>
#include <mini-os/time.h>

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
