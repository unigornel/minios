/**
 * syscalls.h
 *
 * Various system calls needed by the Go runtime.
 */
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <mini-os/types.h>

void *sys_alloc(size_t length);
int32_t sys_write(uint64_t fd, void *p, int32_t n);

#endif /* _SYSCALL_H */
