/**
 * syscalls.c
 *
 * Implementation of various system calls needed by the Go runtime.
 */
#include <mini-os/syscalls.h>

#include <mini-os/console.h>
#include <mini-os/xmalloc.h>

void *sys_alloc(size_t length)
{
    return malloc(length);
}
